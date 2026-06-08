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

#include "causalize/sbg_implementation/horizontal_sorting.hpp"
#include "util/debug.hpp"

#include <algorithms/matching/matching.hpp>
#include <boost/variant/get.hpp>

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// Auxiliar definitions --------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

// EqVarMatch ------------------------------------------------------------------

std::size_t EqVarMatch::size() const { return _equation_list.size(); }

std::tuple<Equation, Expression> EqVarMatch::operator[](std::size_t k) const
{
  ERROR_UNLESS(k < _equation_list.size(), "EqVarMatch::operator[]: index ", k
    , " out of range");
  return {_equation_list[k], _expression_list[k]};
}

void EqVarMatch::pushBack(Equation eq, Expression expr)
{
  _equation_list.push_back(eq);
  _expression_list.push_back(expr);
}

std::ostream& operator<<(std::ostream& out, const EqVarMatch& match)
{
  for (std::size_t k = 0; k < match.size(); ++k) {
    std::tuple<Equation, Expression> solve_pair = match[k];
    out << std::get<1>(solve_pair) << " solved in " << std::get<0>(solve_pair)
      << "\n";
  }

  return out;
}

// HorizontalSortingInfo -------------------------------------------------------

HorizontalSortingInfo::HorizontalSortingInfo(
  const std::vector<SetVertex>& set_vertices
  , const std::vector<SetEdge>& set_edges
  , const std::map<int, EquationInfo>& equations_info
  , SBG::LIB::MatchData matching_result
  , EqVarMatch horizontal_sorting)
  : _set_vertices(set_vertices), _set_edges(set_edges)
    , _equations_info(equations_info), _matching_result(matching_result)
    , _horizontal_sorting(horizontal_sorting) {}

const std::vector<SetVertex>& HorizontalSortingInfo::set_vertices() const
{
  return _set_vertices;
}

const std::vector<SetEdge>& HorizontalSortingInfo::set_edges() const
{
  return _set_edges;
}

const std::map<int, EquationInfo>& HorizontalSortingInfo::equations_info() const
{
  return _equations_info;
}

const SBG::LIB::MatchData& HorizontalSortingInfo::matching_result() const
{
  return _matching_result;
}

const EqVarMatch& HorizontalSortingInfo::horizontal_sorting() const
{
  return _horizontal_sorting;
}

////////////////////////////////////////////////////////////////////////////////
// Horizontal Sorting ----------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

HorizontalSorting::HorizontalSorting(SBGGenerationInfo& sbg_gen_info)
  : _sbg_gen_info(sbg_gen_info) {}

void HorizontalSorting::sortEquation(SetEdge se, CompactSet se_match)
{
  // Get variable and equation set-vertices referenced by the set-edge
  SetVertex eq_sv{-1};
  for (const SetVertex& sv : _sbg_gen_info.set_vertices()) {
    if (sv.node_id() == se.eq_id()) {
      eq_sv = sv;
      break;
    }
  }

  // Get adjusted indices of the equation
  const std::map<int, EquationInfo>& eqs_info = _sbg_gen_info.equations_info();
  EquationInfo eq_info = eqs_info.at(eq_sv.node_id());
  std::vector<Name> counters;
  for (const Index& index : eq_info.indices()) {
    counters.push_back(index.name());
  }
  std::vector<Indexes> indices = se_match.toModelicaIndices(se.translation()
    , counters);

  // Create new equation and save it to current matching
  for (const Indexes& indexes : indices) {
    Equation new_eq = eq_info.toEquation();
    if (is<ForEq>(new_eq)) {
      ForEq for_eq = get<ForEq>(new_eq);
      ERROR_UNLESS(for_eq.elements().size() == 1
        , "HorizontalSorting::sortEquation: ForEq should have should be "
        , "composed by a singleton list");
      new_eq = ForEq{indexes, EquationList{1, for_eq.elements().front()}};
    }
    _sort.pushBack(new_eq, se.access());
  }
}

HorizontalSortingInfo HorizontalSorting::sort()
{
  // Get SBG matching result
  const SBG::LIB::BipartiteSBG& bipartite_sbg = _sbg_gen_info.bipartite_sbg();
  unsigned int num_variables = bipartite_sbg.Y().cardinal();
  unsigned int num_equations = bipartite_sbg.X().cardinal();
  ERROR_UNLESS(num_variables == num_equations
    , "HorizontalSorting::sort: unbalanced system of equations.\n"
    , "Number of variables: ", num_variables, "\n"
    , "Number of equations: ", num_equations);

  SBG::LIB::MatchData matching_result
    = SBG::LIB::Matching{}.calculate(bipartite_sbg);
  ERROR_UNLESS(matching_result.full_match(), "HorizontalSorting::sort: "
    , "higher index system");

  // Traverse set-edges to get equation-variable matching
  CompactSet sbg_match{matching_result.M()}; 
  for (const SetEdge& se : _sbg_gen_info.set_edges()) {
    CompactSet jth_eq_var_match = se.domain();
    jth_eq_var_match.intersection(sbg_match);
    if (jth_eq_var_match.cardinal() > 0) {
      sortEquation(se, jth_eq_var_match);
    }
  }

  return HorizontalSortingInfo{_sbg_gen_info.set_vertices()
    , _sbg_gen_info.set_edges(), _sbg_gen_info.equations_info()
    , matching_result, _sort};
}

} // namespace Causalize

} // namespace Modelica
