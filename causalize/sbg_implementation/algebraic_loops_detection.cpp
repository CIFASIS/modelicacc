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

#include "causalize/sbg_implementation/algebraic_loops_detection.hpp"
#include "causalize/sbg_implementation/equation_info.hpp"
#include "causalize/sbg_implementation/set_edge.hpp"
#include "causalize/sbg_implementation/set_vertex.hpp"
#include "util/debug.hpp"
#include "util/compact_set.hpp"

#include <algorithms/misc/causalization_builders.hpp>
#include <algorithms/scc/scc.hpp>
#include <sbg/directed_sbg.hpp>
#include <sbg/pw_map.hpp>
#include <sbg/set.hpp>

#include <algorithm>

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// ModelicaCC algebraic loops --------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

// AlgebraicLoop ---------------------------------------------------------------

AlgebraicLoop::AlgebraicLoop(EquationList equations, ExpList variables) : _equations(equations), _variables(variables) {}

const EquationList& AlgebraicLoop::equations() const { return _equations; }

const ExpList& AlgebraicLoop::variables() const { return _variables; }

bool AlgebraicLoop::isEmpty() const { return _equations.empty(); }

void AlgebraicLoop::pushBack(Equation equation, Expression variable)
{
  _equations.push_back(equation);
  _variables.push_back(variable);
}

void AlgebraicLoop::concatenation(AlgebraicLoop other)
{
  _equations.insert(_equations.end(), other._equations.begin(), other._equations.end());
  _variables.insert(_variables.end(), other._variables.begin(), other._variables.end());
}

std::ostream& operator<<(std::ostream& out, const AlgebraicLoop& loop)
{
  for (const Expression& var : loop.variables()) {
    out << var << "\n";
  }
  for (const Equation& eq : loop.equations()) {
    out << eq << "\n";
  }

  return out;
}

// AlgebraicLoops --------------------------------------------------------------

std::size_t AlgebraicLoops::size() const { return _loops.size(); }

AlgebraicLoop AlgebraicLoops::operator[](std::size_t k) const
{
  ERROR_UNLESS(k < _loops.size(), "AlgebraicLoops::operator[]: index ", k, " out of range");
  return _loops[k];
}

// auto AlgebraicLoops::begin() const { return _loops.begin(); }

// auto AlgebraicLoops::end() const { return _loops.end(); }

void AlgebraicLoops::pushBack(AlgebraicLoop loop)
{
  if (!loop.isEmpty()) {
    _loops.push_back(loop);
  }
}

void AlgebraicLoops::reverse() { std::reverse(_loops.begin(), _loops.end()); }

std::ostream& operator<<(std::ostream& out, const AlgebraicLoops& loops)
{
  for (const AlgebraicLoop& loop : loops) {
    out << loop << "\n";
  }

  return out;
}

////////////////////////////////////////////////////////////////////////////////
// Algebraic loops return structure --------------------------------------------
////////////////////////////////////////////////////////////////////////////////

AlgebraicLoopsResult::AlgebraicLoopsResult(ModelicaSBG modelica_bsbg, SBG::LIB::SCCData scc_result)
    : _modelica_bsbg(modelica_bsbg), _scc_result(scc_result)
{
}

const ModelicaSBG& AlgebraicLoopsResult::modelica_bsbg() const { return _modelica_bsbg; }

const SBG::LIB::SCCData& AlgebraicLoopsResult::scc_result() const { return _scc_result; }

AlgebraicLoop AlgebraicLoopsResult::loopToModelicaFormat(const SetEdge& se) const
{
  AlgebraicLoop result;

  SetVertex eq_sv = _modelica_bsbg.setVertex(se.eq_id());
  EquationInfo eq_info = eq_sv.info().value();
  result.pushBack(eq_info.restrictEquation(eq_info.indices()), se.access());

  return result;
}

AlgebraicLoops AlgebraicLoopsResult::toModelicaFormat() const
{
  AlgebraicLoops result;

  SBG::LIB::PWMap rmap = _scc_result.rmap();
  SBG::LIB::Set representatives = rmap.image();
  while (!representatives.isEmpty()) {
    SBG::LIB::Set min_elem_set{representatives.minElem()};
    CompactSet represented{rmap.preImage(min_elem_set)};
    // Get all equations and variables that belong to this loop
    AlgebraicLoop loop;
    for (const SetEdge& se : _modelica_bsbg.set_edges()) {
      CompactSet jth_domain = se.domain();
      jth_domain.intersection(represented);
      if (jth_domain.cardinal() > 0) {
        loop.concatenation(loopToModelicaFormat(se));
        representatives = representatives.difference(se.domain().set());
      }
    }
    result.pushBack(loop);
  }

  return result;
}

std::vector<LoopT> AlgebraicLoopsResult::toSBGFormat() const
{
  std::vector<LoopT> result;

  SBG::LIB::PWMap rmap = _scc_result.rmap();
  SBG::LIB::Set representatives = rmap.image();
  while (!representatives.isEmpty()) {
    SBG::LIB::Set min_elem_set{representatives.minElem()};
    SBG::LIB::Set represented = rmap.preImage(min_elem_set);
    // Get all equations and variables that belong to this loop
    LoopT loop;
    for (const SetEdge& se : _modelica_bsbg.set_edges()) {
      SBG::LIB::Set se_domain = se.domain().set();
      SBG::LIB::Set loop_in_se = se_domain.intersection(represented);
      if (loop_in_se.cardinal() > 0) {
        loop.push_back(se.domain().set());
        representatives = representatives.difference(se_domain);
      }
    }
    result.push_back(loop);
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
// Algebraic loops detector ----------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

AlgebraicLoopsDetector::AlgebraicLoopsDetector(HorizontalSortingResult& hs_result) : _hs_result(hs_result) {}

AlgebraicLoopsResult AlgebraicLoopsDetector::detect()
{
  SBG::LIB::DirectedSBG loops_dsbg = misc::buildLoopDetectionSBG(_hs_result.matching_result());
  SBG::LIB::SCCData scc_result = SBG::LIB::SCC{}.calculate(loops_dsbg);

  return AlgebraicLoopsResult{_hs_result.modelica_bsbg(), scc_result};
}

}  // namespace Causalize

}  // namespace Modelica
