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

#include "algorithms/matching/matching.hpp"
#include "algorithms/matching/match_data.hpp"

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// Auxiliar definitions --------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////
// Horizontal Sorting ----------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

HorizontalSorting::HorizontalSorting(GenerateSBGInput& sbg_generator)
  : _mmo_class(sbg_generator.mmo_class())
{
  _bipartite_sbg = sbg_generator.buildFromModel();
  _max_dim = sbg_generator.max_dim();
  _set_vertices = sbg_generator.set_vertices();
  _set_edges = sbg_generator.set_edges();
  _equations_info = sbg_generator.equations_info();
}

void HorizontalSorting::sortEquation(SetEdge se, CompactSet se_match)
{
  // Matched all edges of a set-edge, indices can be used without
  // modifications (the equation remains as it is)
  if (se_match == se.domain()) {
    // Get variable and equation set-vertices referenced by the set-edge
    SetVertex var_sv{-1};
    SetVertex eq_sv{-1};
    for (const SetVertex& sv : _set_vertices) {
      if (sv.node_id() == se.var_id()) {
        var_sv = sv;
      }
      if (sv.node_id() == se.eq_id()) {
        eq_sv = sv;
      }
    }

    _match.pushBack(_equations_info[eq_sv.node_id()].equation()
      , se.access());
  } else {
    ERROR("HorizontalSorting::sort: conversion not yet supported");
  }
}

EqVarMatch HorizontalSorting::sort()
{
  unsigned int num_variables = _bipartite_sbg.Y().cardinal();
  unsigned int num_equations = _bipartite_sbg.X().cardinal();
  ERROR_UNLESS(num_variables == num_equations
    , "HorizontalSorting::sort: unbalanced system of equations.\n"
    , "Number of variables: ", num_variables, "\n"
    , "Number of equations: ", num_equations);

  SBG::LIB::MatchData matching_result
    = SBG::LIB::Matching{}.calculate(_bipartite_sbg);
  ERROR_UNLESS(matching_result.full_match(), "HorizontalSorting::sort: "
    , "higher index system");

  CompactSet sbg_match{matching_result.M()}; 
  for (const SetEdge& se : _set_edges) {
    CompactSet jth_eq_var_match = se.domain();
    jth_eq_var_match.intersection(sbg_match);
    if (jth_eq_var_match.cardinal() > 0) {
      sortEquation(se, jth_eq_var_match);
    }
  }

  return _match;
}

} // namespace Causalize

} // namespace Modelica
