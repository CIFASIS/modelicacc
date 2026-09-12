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
#include "ast/equation.hpp"
#include "ast/modification.hpp"
#include "causalize/sbg_implementation/set_edge.hpp"
#include "causalize/sbg_implementation/set_vertex.hpp"
#include "causalize/sbg_implementation/ast_visitors/residual_equation.hpp"
#include "causalize/sbg_implementation/ast_visitors/variable_renamer.hpp"
#include "util/debug.hpp"

#include <algorithms/sorting/topological/topological_sorting.hpp>
#include <sbg/directed_sbg.hpp>
#include <sbg/expression.hpp>
#include <sbg/integer.hpp>
#include <sbg/pw_map.hpp>
#include <sbg/set.hpp>
#include <util/time_profiler.hpp>

#include <algorithm>
#include <string>
#include <variant>

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// ModelicaSBG modifier --------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

// Getters ---------------------------------------------------------------------

const ModelicaSBG& ModelicaSBGModifier::modelica_bsbg() const
{
  return _output_modelica_bsbg;
}

const std::vector<std::pair<AST::Name, VarInfo>>&
  ModelicaSBGModifier::added_variables() const
{
  return _added_variables;
}

// Member functions ------------------------------------------------------------

void ModelicaSBGModifier::addVarsDeclarations()
{
  for (SetEdge& se : _input_modelica_bsbg.set_edges()) {
    SBG::LIB::Set se_res = se.translatedDomain()
      .intersection(_residual_vertices);
    if (!se_res.isEmpty()) {
      AST::Name start_mod = "start";
      AST::ClassModification class_mod;
      class_mod.push_back(AST::ElMod{start_mod, AST::ModEq{AST::Expression{1}}});
      SetVertex var_sv = _input_modelica_bsbg.setVertex(se.var_id());
      VarInfo var_info = std::get<VarInfo>(var_sv.info());
      var_info.set_modification(AST::Modification{AST::ModClass{class_mod}});
      _added_variables.push_back(
        {"guess_" + var_sv.name(), var_info}
      );
      _added_variables.push_back(
        {"res_" + var_sv.name(), var_info}
      );
    }
  }
}

void ModelicaSBGModifier::partition(const SBG::LIB::Set& not_residual)
{
  // Partition equation/variable matchings according to residual vertices, i.e,
  // tearing variables.
  SetVertices svs = _input_modelica_bsbg.set_vertices();
  for (const SetVertex& sv : svs) {
    _output_modelica_bsbg.addSetVertex(sv);
  }

  SetEdges ses = _input_modelica_bsbg.set_edges();
  for (const SetEdge& se : ses) {
    _output_modelica_bsbg.addSetEdge(se.restrict(
      SBG::LIB::Set{_residual_vertices}
    ));
    _output_modelica_bsbg.addSetEdge(se.restrict(not_residual));
  }
}

void ModelicaSBGModifier::addGuess(
  const SetEdge& se, const SBG::LIB::Set& se_res
  , const SBG::LIB::IntTuple& max_elem
)
{
  // Add guess equation vertices.
  SetVertex original_eq_sv = _output_modelica_bsbg.setVertex(se.eq_id());
  std::string name = "guess(" + original_eq_sv.name() + ")";
  VariableRenamer renamer{"guess_"};
  AST::Equation guess_eq{AST::Equality{
    se.access(), Apply(renamer, se.access())
  }};
  EquationInfo original_info = std::get<EquationInfo>(original_eq_sv.info());
  EquationInfo guess_info{
    original_info.indices(), guess_eq, original_info.scalar()
  };
  SBG::LIB::IntTuple neg_translation;
  for (const SBG::LIB::Int x : se.translation()) {
    neg_translation.pushBack(-x);
  }
  SBG::LIB::Set guess_vertices = se_res;
  guess_vertices = guess_vertices.translate(neg_translation);
  int guess_eq_id = _output_modelica_bsbg.addSetVertex(
    guess_vertices, name, guess_info
  );

  // Add guess/equation matching edges.
  SBG::LIB::Set edges = se.domain();
  SBG::LIB::IntTuple se_translation = se.translation();
  std::size_t arity = se_res.arity();
  SBG::LIB::IntTuple t;
  for (std::size_t k = 0; k < arity; ++k) {
    t.pushBack(max_elem[k] + se_translation[k]);
  }
  name = "guess equation of " + se.name();
  _output_modelica_bsbg.addSetEdge(
    se.var_id(), guess_eq_id, edges, t, se.access(), name
  );
}

void ModelicaSBGModifier::addGuessMatchs(const SBG::LIB::IntTuple& max_elem)
{
  SetEdges ses = _input_modelica_bsbg.set_edges(); 
  for (const SetEdge& se : ses) {
    SBG::LIB::Set se_res = se.translatedDomain()
      .intersection(_residual_vertices);
    if (!se_res.isEmpty()) {
      addGuess(se, se_res, max_elem);
    }
  }
}

void ModelicaSBGModifier::modifyResidual(
  SetEdge& se, const SBG::LIB::Set& se_res
)
{
  // Modify equation expression.
  SetVertex& eq_sv = _output_modelica_bsbg.setVertex(se.eq_id());
  EquationInfo eq_info = std::get<EquationInfo>(eq_sv.info());
  ResidualEqVisitor res_visit{se.access()};
  eq_sv.set_info(EquationInfo{
    eq_info.indices()
    , Apply(res_visit, eq_info.equation())
    , eq_info.scalar()
  });

  // Add residual variable vertices.
  std::string name = "res_" + _output_modelica_bsbg
    .setVertex(se.var_id()).name();
  SBG::LIB::IntTuple neg_translation;
  for (const SBG::LIB::Int x : se.translation()) {
    neg_translation.pushBack(-x);
  }
  SBG::LIB::Set res_vertices = se_res;
  res_vertices = res_vertices.translate(neg_translation);
  int res_var_id = _output_modelica_bsbg.addSetVertex(res_vertices, name);

  // Modify matched variable.
  se.set_var_id(res_var_id);
  VariableRenamer renamer{"res_"};
  se.set_access(Apply(renamer, se.access()));
}

void ModelicaSBGModifier::modifyResidualMatchs()
{
  SetEdges ses = _output_modelica_bsbg.set_edges(); 
  for (SetEdge& se : ses) {
    SBG::LIB::Set se_res = se.translatedDomain()
      .intersection(_residual_vertices);
    if (!se_res.isEmpty()) {
      modifyResidual(se, se_res);
    }
  }
}

ModelicaSBG ModelicaSBGModifier::modify(
  ModelicaSBG modelica_bsbg, const VerticalSortingBuilder& builder
)
{
  _input_modelica_bsbg = modelica_bsbg;
  _output_modelica_bsbg = ModelicaSBG{};
  _added_variables = std::vector<std::pair<AST::Name, VarInfo>>{};
  _residual_vertices = builder.residual_vertices();

  addVarsDeclarations();
  SBG::LIB::Set not_residual = builder.dsbg().V()
    .difference(_residual_vertices);
  partition(not_residual);
  SBG::LIB::Set guess_vertices = builder.guess_offset()
    .image(_residual_vertices);
  SBG::LIB::IntTuple max_elem = builder.dsbg().V().difference(guess_vertices)
    .maxElem();
  addGuessMatchs(max_elem);
  modifyResidualMatchs();

  return _output_modelica_bsbg;
}

////////////////////////////////////////////////////////////////////////////////
// Converter to Modelica code of an equation/variable pairing ------------------
////////////////////////////////////////////////////////////////////////////////

MatchToModelicaFormat::MatchToModelicaFormat(
  const ModelicaSBG& modelica_bsbg, const SBG::LIB::PWMap& sort
  , const VerticalSortingBuilder& builder, const SBG::LIB::Map& jth_scc_smap
) : _modelica_bsbg(modelica_bsbg), _sort(sort), _builder(builder)
    , _jth_scc_smap(jth_scc_smap) {}

detail::SortedMatchs MatchToModelicaFormat::equationToModelicaFormat(
  const SetEdge& se, const SBG::LIB::Set& se_match
  , const SBG::LIB::Expression& expr
)
{
  bool is_residual = !se_match.intersection(
    _builder.residual_vertices()
  ).isEmpty();
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
    SBG::LIB::Set jth_eq_var_match = se.translatedDomain()
      .intersection(match_domain);
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
) : _modelica_bsbg(modelica_bsbg), _sort(sort), _builder(builder) {}

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
    vertices_smap = vertices_smap.restrict(unsorted_vertices);
    for (const SBG::LIB::Map& m : vertices_smap) {
      if (!m.image().intersection(sorted_vertices).isEmpty()
         && m.domain().intersection(sorted_vertices).isEmpty()) {
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
      partitioned_sort.insert(m.restrict(se.translatedDomain()));
    }
  }
  _sort = partitioned_sort;
}

CausalModel VerticalSortingResult::toModelicaFormat()
{
  ModelicaSBGModifier modifier;
  modifier.modify(_modelica_bsbg, _builder);
  _modelica_bsbg = modifier.modelica_bsbg();

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
  VerticalSortingBuilder vs_builder{_loops_result.scc_result()
    , _tearing_result.mfvs_result()};
  {
    SBG::Util::Internal::TimeProfiler profiler{"Vertical sorting SBG builder"};
    vs_builder.build();
  }
  SBG::LIB::PWMap vertical_sort;
  {
    SBG::Util::Internal::TimeProfiler profiler{"Vertical sorting"};
    vertical_sort = SBG::LIB::TopologicalSorting{}
      .calculate(vs_builder.dsbg(), vs_builder.rmap());
  }

  return VerticalSortingResult{
    _tearing_result.modelica_bsbg(), vertical_sort, vs_builder
  };
}

}  // namespace Causalize

}  // namespace Modelica
