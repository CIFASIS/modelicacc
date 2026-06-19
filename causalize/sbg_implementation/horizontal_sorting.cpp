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
#include "causalize/sbg_implementation/equation_info.hpp"
#include "causalize/sbg_implementation/set_vertex.hpp"
#include "util/debug.hpp"

#include <algorithms/matching/match_data.hpp>
#include <algorithms/matching/matching.hpp>
#include <boost/variant/get.hpp>
#include <sbg/bipartite_sbg.hpp>

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// ModelicaCC variable-equation matching ---------------------------------------
////////////////////////////////////////////////////////////////////////////////

// EqVarMatch ------------------------------------------------------------------

EqVarMatch::EqVarMatch(Equation equation, Expression variable)
  : _equation(equation), _variable(variable) {}

const Equation& EqVarMatch::equation() const { return _equation; }

const Expression& EqVarMatch::variable() const { return _variable; }

std::ostream& operator<<(std::ostream& out, const EqVarMatch& match)
{
  out << match.variable() << " solved in " << match.equation();
  return out;
}

// ModelMatch ------------------------------------------------------------------

std::size_t ModelMatch::size() const { return _model_match.size(); }

EqVarMatch ModelMatch::operator[](std::size_t k) const
{
  ERROR_UNLESS(k < _model_match.size(), "ModelMatch::operator[]: index ", k
    , " out of range");
  return _model_match[k];
}

void ModelMatch::pushBack(EqVarMatch match)
{
  _model_match.push_back(match);
}

void ModelMatch::concatenation(ModelMatch other)
{
  _model_match.insert(_model_match.end(), other._model_match.begin()
    , other._model_match.end());
}

std::ostream& operator<<(std::ostream& out, const ModelMatch& match)
{
  for (std::size_t k = 0; k < match.size(); ++k) {
    out << match[k] << "\n";
  }

  return out;
}

////////////////////////////////////////////////////////////////////////////////
// Horizontal sorting return structure -----------------------------------------
////////////////////////////////////////////////////////////////////////////////

HorizontalSortingResult::HorizontalSortingResult(
  ModelicaSBG modelica_bsbg
  , SBG::LIB::MatchData matching_result)
    : _modelica_bsbg(modelica_bsbg), _matching_result(matching_result) {}

const ModelicaSBG& HorizontalSortingResult::modelica_bsbg() const
{
  return _modelica_bsbg;
}

const SBG::LIB::MatchData& HorizontalSortingResult::matching_result() const
{
  return _matching_result;
}

ModelMatch HorizontalSortingResult::equationToModelicaFormat(SetEdge se
  , CompactSet se_match) const
{
  // Get equation set-vertex referenced by the set-edge
  SetVertex eq_sv = _modelica_bsbg.setVertex(se.eq_id());

  // Get adjusted indices of the equation
  EquationInfo eq_info = eq_sv.info().value();
  std::vector<Name> counters;
  for (const Index& index : eq_info.indices()) {
    counters.push_back(index.name());
  }
  std::vector<Indexes> indices = toModelicaIndices(se_match, se.translation()
    , counters);

  // Create new equation and save it to current matching
  ModelMatch result;
  for (const Indexes& indexes : indices) {
    result.pushBack(EqVarMatch{eq_info.restrictEquation(indexes), se.access()});
  }
  return result;
}

ModelMatch HorizontalSortingResult::toModelicaFormat() const
{
  ModelMatch result;

  // Traverse set-edges to get equation-variable matching
  CompactSet sbg_match{_matching_result.M()}; 
  for (const SetEdge& se : _modelica_bsbg.set_edges()) {
    CompactSet jth_eq_var_match = se.domain();
    jth_eq_var_match.intersection(sbg_match);
    if (jth_eq_var_match.cardinal() > 0) {
      result.concatenation(equationToModelicaFormat(se, jth_eq_var_match));
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
// Horizontal sorting ----------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

HorizontalSorting::HorizontalSorting(SBGGenerationResult& sbg_generation_result)
  : _sbg_generation_result(sbg_generation_result) {}

namespace {

/**
 * @brief Constructs the new Modelica bipartite SBG with the necessary
 * partitions of the set-edges induced by the matching.
 */
ModelicaSBG constructNewModelicaSBG(ModelicaSBG old
  , SBG::LIB::MatchData matching_result)
{
  ModelicaSBG result{old.arity()};

  CompactSet M{matching_result.M()};
  result.addSetVertices(old.set_vertices());
  for (const SetEdge& se : old.set_edges()) {
    result.addSetEdge(se.restrict(M));
  }

  return result;
}

} // namespace

HorizontalSortingResult HorizontalSorting::sort()
{
  // Get SBG matching result
  const SBG::LIB::BipartiteSBG& bipartite_sbg
    = _sbg_generation_result.bipartite_sbg();
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

  ModelicaSBG old = _sbg_generation_result.modelica_bsbg();
  return HorizontalSortingResult{
    constructNewModelicaSBG(old, matching_result)
    , matching_result};
}

} // namespace Causalize

} // namespace Modelica
