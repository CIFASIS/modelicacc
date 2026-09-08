/*****************************************************************************

    This file is part of Modelica C Compiler.

    Modelica C Compiler is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Modelica C Compiler is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Modelica C Compiler.  If not, see <http://www.gnu.org/licenses/>.

******************************************************************************/

#include "causalize/sbg_implementation/vertical_sorting.hpp"
#include "causalize/sbg_implementation/equation_info.hpp"
#include "causalize/sbg_implementation/set_edge.hpp"
#include "util/compact_set.hpp"
#include "util/debug.hpp"

#include <algorithms/sorting/topological/topological_sorting.hpp>
#include <boost/variant/get.hpp>
#include <sbg/directed_sbg.hpp>
#include <sbg/expression.hpp>
#include <sbg/pw_map.hpp>
#include <sbg/set.hpp>
#include <util/time_profiler.hpp>

#include <algorithm>

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// Converter to Modelica code of an equation/variable pairing ------------------
////////////////////////////////////////////////////////////////////////////////

MatchToModelicaFormat::MatchToModelicaFormat(
  const ModelicaSBG& modelica_bsbg, const SBG::LIB::PWMap& sort
  , const VerticalSortingBuilder& builder, const SBG::LIB::Map& jth_scc_smap
) : _modelica_bsbg(modelica_bsbg), _sort(sort), _builder(builder)
    , _jth_scc_smap(jth_scc_smap) {}

detail::SortedMatchs MatchToModelicaFormat::equationToModelicaFormat(
  const SetEdge& se, const CompactSet& se_match
  , const SBG::LIB::Expression& expr
)
{
  bool is_residual = !se_match.set()
    .intersection(_builder.residual_vertices()).isEmpty();
  auto [eq_info, accesses] = getAccess(_modelica_bsbg, se, se_match, expr);

  // Create new equation and save it to current matching.
  detail::SortedMatchs result;
  for (const auto& [_, indexes] : accesses) {
    EquationInfo access_eq_info{indexes, eq_info.equation(), eq_info.scalar()};
    result.push_back(detail::SortedMatch{access_eq_info, se.access()});
    // Add equation res(...) = 0.
    if (is_residual) {
      AST::Equality eq{se.access(), 0};
      EquationInfo residual_eq_info{indexes, eq, eq_info.scalar()};
      _residual_zero.push_back(
        detail::SortedMatch{residual_eq_info, AST::Expression{}}
      );
    }
  }
  return result;
}

detail::SortedMatchs MatchToModelicaFormat::format(
  const SBG::LIB::Map& match, const SBG::LIB::Expression& expr
)
{
  detail::SortedMatchs result;

  // Traverse set-edges to get equation-variable matching.
  SBG::LIB::Set match_domain = match.domain();
  match_domain.compact();
  for (const SetEdge& se : _modelica_bsbg.set_edges()) {
    CompactSet jth_eq_var_match = se.translatedDomain();
    jth_eq_var_match.intersection(match_domain);
    if (jth_eq_var_match.cardinal() > 0) {
      detail::SortedMatchs jth_result = equationToModelicaFormat(
        se, jth_eq_var_match, expr
      );
      result.insert(
        result.end()
        , std::make_move_iterator(jth_result.begin())
        , std::make_move_iterator(jth_result.end())
      );
    }
  }

  // Add res(...) = 0 equations at the end.
  result.insert(
    result.end()
    , std::make_move_iterator(_residual_zero.begin())
    , std::make_move_iterator(_residual_zero.end())
  );

  return result;
}

////////////////////////////////////////////////////////////////////////////////
// Vertical sorting return structure -------------------------------------------
////////////////////////////////////////////////////////////////////////////////

// Constructors/destructors ----------------------------------------------------

VerticalSortingResult::VerticalSortingResult(
  const ModelicaSBG& modelica_bsbg, const SBG::LIB::PWMap& sort
  , const VerticalSortingBuilder& builder
) : _modelica_bsbg(modelica_bsbg), _sort(sort), _builder(builder)
    , _added_variables(builder.added_variables()) {}

// Getters ---------------------------------------------------------------------

const ModelicaSBG& VerticalSortingResult::modelica_bsbg() const
{
  return _modelica_bsbg;
}

const SBG::LIB::PWMap& VerticalSortingResult::sort() const { return _sort; }

const std::vector<std::pair<AST::Name, VarInfo>>
  VerticalSortingResult::added_variables() const
{
  return _added_variables;
}

// Member functions ------------------------------------------------------------

SortedAlgebraicLoop VerticalSortingResult::outerBounds(
  const detail::SortedMatchs& matchs, bool is_scalar
) const
{
  SortedAlgebraicLoop loop;

  if (is_scalar) {
    for (const detail::SortedMatch& match : matchs) {
      EquationInfo eq_info = match.first;
      AST::Expression expr = match.second;
      loop.pushBack(EqVarMatch{
        eq_info.restrictBounds(eq_info.indices()), AST::ExpList{expr}
      });
    }
  } else {
    bool scalar_equations = true;
    AST::Indexes indexes;
    AST::EquationList eq_list;
    AST::ExpList expr_list;
    bool first = true;
    for (const detail::SortedMatch& match : matchs) {
      EquationInfo eq_info = match.first;
      if (first) {
        indexes = eq_info.indices();
        first = false;
      }
      if (!eq_info.scalar()) {
        scalar_equations = false;
      }
      eq_list.push_back(eq_info.adjustSubscripts(eq_info.indices(), indexes));
      expr_list.push_back(match.second);
    }

    if (scalar_equations) {
      for (std::size_t j = 0; j < eq_list.size(); ++j) {
        loop.pushBack(EqVarMatch{eq_list[j], ExpList{expr_list[j]}});
      }
    } else {
      AST::ForEq for_eq{indexes, eq_list};
      loop.pushBack(EqVarMatch{for_eq, expr_list});
    }
  }

  return loop;
}

bool isScalar(
  const SBG::LIB::PWMap& sort, const SBG::LIB::Set& scc, std::size_t card
)
{
  bool result = false;
  SBG::LIB::PWMap scc_sort = sort.restrict(scc);
  for (const SBG::LIB::Map& m : scc_sort) {
    if (m.domain().cardinal() != card) {
      result = true;
      break;
    }
  }
  return result;
}

SortedAlgebraicLoop VerticalSortingResult::sortLoop(
  const SBG::LIB::Map& jth_scc_smap
) const
{
  MatchToModelicaFormat match_formatter{
    _modelica_bsbg, _sort, _builder, jth_scc_smap
  };
  SBG::LIB::Set reps = jth_scc_smap.domain();
  std::size_t card = reps.cardinal();
  const SBG::LIB::PWMap& rmap = _builder.rmap();
  SBG::LIB::Set represented = rmap.preImage(rmap.image(reps));
  bool is_scalar = isScalar(_sort, represented, card);

  detail::SortedMatchs matchs;
  SBG::LIB::PWMap vertices_smap = _sort.restrict(represented);
  SBG::LIB::Set unsorted_vertices = represented;
  SBG::LIB::Set jth_vertices = reps;
  while (!unsorted_vertices.isEmpty()) {
    // Handle current equation/variable matching.
    SBG::LIB::Map jth_vertices_smap = *(
      vertices_smap.restrict(jth_vertices).begin()
    );
    SBG::LIB::Set sorted_vertices = jth_vertices_smap.domain();
    sorted_vertices.compact();

    // Check if we can handle these repetitive structures.
    if (!is_scalar) { // Array of algebraic loops.
      ERROR_UNLESS(card == sorted_vertices.cardinal()
        , "VericalSortingResult::sortLoop: incompatible repetitive structures");
    }

    SBG::LIB::Expression expr = is_scalar ? jth_vertices_smap.law()
      : jth_scc_smap.law();
    detail::SortedMatchs jth_matchs = match_formatter.format(
      jth_vertices_smap, expr
    );
    matchs.insert(
      matchs.end()
      , std::make_move_iterator(jth_matchs.begin())
      , std::make_move_iterator(jth_matchs.end())
    );

    // Get next map in the sort.
    unsorted_vertices = unsorted_vertices.difference(sorted_vertices);
    vertices_smap = vertices_smap.restrict(
      vertices_smap.domain().difference(sorted_vertices)
    );
    for (const SBG::LIB::Map& m : vertices_smap) {
      if (!m.image().intersection(sorted_vertices).isEmpty()) {
        jth_vertices = m.domain();
        break;
      }
    }
  }

  return outerBounds(matchs, is_scalar);
}

CausalModel VerticalSortingResult::sortLoops() const
{
  CausalModel result;

  const SBG::LIB::PWMap& rmap = _builder.rmap();
  const SBG::LIB::Set reps = rmap.fixedPoints();
  SBG::LIB::PWMap sccs_smap = _builder.guess_offset()
    .composition(_sort.restrict(_builder.end_points()));
  SBG::LIB::Set unsorted_sccs = reps;
  SBG::LIB::Set jth_scc = sccs_smap.fixedPoints();
  while (!unsorted_sccs.isEmpty()) {
    SBG::LIB::Map jth_scc_smap = *(sccs_smap.restrict(jth_scc).begin());
    result.pushBack(sortLoop(jth_scc_smap));

    unsorted_sccs = unsorted_sccs.difference(rmap.image(jth_scc));
    sccs_smap = sccs_smap.restrict(rmap.preImage(unsorted_sccs));
    for (const SBG::LIB::Map& m : sccs_smap) {
      if (!m.image().intersection(jth_scc).isEmpty()) {
        jth_scc = m.domain();
        break;
      }
    }
  }

  return result;
}

// Transform to Modelica format ------------------------------------------------

void VerticalSortingResult::partitionSort()
{
  SBG::LIB::PWMap partitioned_sort;
  for (const SBG::LIB::Map& m : _sort) {
    for (const SetEdge& se : _modelica_bsbg.set_edges()) {
      partitioned_sort.insert(m.restrict(se.translatedDomain().set()));
    }
  }
  _sort = partitioned_sort;
}

CausalModel VerticalSortingResult::toModelicaFormat()
{
  partitionSort();
  return sortLoops();
}

////////////////////////////////////////////////////////////////////////////////
// Vertical sorting ------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

VerticalSorting::VerticalSorting(
  AlgebraicLoopsResult& loops_result, TearingResult& tearing_result
) : _loops_result(loops_result), _tearing_result(tearing_result) {}

VerticalSortingResult VerticalSorting::sort()
{
  ModelicaSBG modelica_bsbg;
  VerticalSortingBuilder vs_builder{_loops_result.scc_result()
    , _tearing_result.mfvs_result()};
  {
    SBG::Util::Internal::TimeProfiler profiler{"Vertical sorting SBG builder"};
    modelica_bsbg = vs_builder.build(_tearing_result.modelica_bsbg());
  }
  SBG::LIB::PWMap vertical_sort;
  {
    SBG::Util::Internal::TimeProfiler profiler{"Vertical sorting"};
    vertical_sort = SBG::LIB::TopologicalSorting{}
      .calculate(vs_builder.dsbg(), vs_builder.rmap());
  }

  return VerticalSortingResult{modelica_bsbg, vertical_sort, vs_builder};
}

}  // namespace Causalize

}  // namespace Modelica
