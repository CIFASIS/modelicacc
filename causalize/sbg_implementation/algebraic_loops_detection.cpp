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
#include "util/debug.hpp"

#include <algorithms/matching/matching.hpp>
#include <algorithms/matching/match_data.hpp>
#include <algorithms/mfvs/min_feedback_vertex_set.hpp>
#include <algorithms/misc/causalization_builders.hpp>
#include <algorithms/scc/scc.hpp>
#include <algorithms/scc/scc_data.hpp>
#include <algorithms/sorting/topological/topological_sorting.hpp>
#include <sbg/bipartite_sbg.hpp>
#include <sbg/directed_sbg.hpp>
#include <sbg/pw_map.hpp>
#include <sbg/set.hpp>

#include <algorithm>

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// Auxiliar definitions --------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

// AlgebraicLoop ---------------------------------------------------------------

AlgebraicLoop::AlgebraicLoop(EquationList equations, ExpList variables)
  : _equations(equations), _variables(variables) {}

const EquationList& AlgebraicLoop::equations() const { return _equations; }

const ExpList& AlgebraicLoop::variables() const { return _variables; }

bool AlgebraicLoop::isEmpty() const { return _equations.empty(); }

void AlgebraicLoop::pushBack(Equation equation, Expression variable)
{
  _equations.push_back(equation);
  _variables.push_back(variable);
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
  ERROR_UNLESS(k < _loops.size(), "AlgebraicLoops::operator[]: index ", k
    , " out of range");
  return _loops[k];
}

//auto AlgebraicLoops::begin() const { return _loops.begin(); }

//auto AlgebraicLoops::end() const { return _loops.end(); }

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

// AlgebraicLoopsInfo ----------------------------------------------------------

AlgebraicLoopsInfo::AlgebraicLoopsInfo(
  const std::vector<SetVertex>& set_vertices
  , const std::vector<SetEdge>& set_edges
  , SBG::LIB::SCCData scc_result, AlgebraicLoops loops)
  : _set_vertices(set_vertices), _set_edges(set_edges), _scc_result(scc_result)
    , _loops(loops) {}

const std::vector<SetVertex>& AlgebraicLoopsInfo::set_vertices() const
{
  return _set_vertices;
}

const std::vector<SetEdge>& AlgebraicLoopsInfo::set_edges() const
{
  return _set_edges;
}

const SBG::LIB::SCCData& AlgebraicLoopsInfo::scc_result() const
{
  return _scc_result;
}

const AlgebraicLoops& AlgebraicLoopsInfo::loops() const { return _loops; }

////////////////////////////////////////////////////////////////////////////////
// Algebraic Loops Detector ----------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

AlgebraicLoopsDetector::AlgebraicLoopsDetector(HorizontalSortingInfo& hs_info)
  : _hs_info(hs_info) {}

void AlgebraicLoopsDetector::detectLoop(SetEdge se, AlgebraicLoop& loop)
{
  // Get equation set-vertex referenced by the set-edge
  SetVertex eq_sv{-1};
  for (const SetVertex& sv : _hs_info.set_vertices()) {
    if (sv.node_id() == se.eq_id()) {
      eq_sv = sv;
      break;
    }
  }

  const auto& eqs_info = _hs_info.equations_info();
  EquationInfo eq_info = eqs_info.at(eq_sv.node_id());
  Equation eq = eq_info.equation();
  if (se.domain().cardinal() > 1) { // Array equation
    eq = ForEq{eq_info.indices(), EquationList{1, eq_info.equation()}};
  }
  loop.pushBack(eq, se.access());
}

AlgebraicLoopsInfo AlgebraicLoopsDetector::detect()
{
  SBG::LIB::DirectedSBG loops_dsbg
    = misc::buildLoopDetectionSBG(_hs_info.matching_result()); 
  SBG::LIB::SCCData scc_result = SBG::LIB::SCC{}.calculate(loops_dsbg);

  SBG::LIB::PWMap rmap = scc_result.rmap();
  SBG::LIB::Set representatives = rmap.image();
  while (!representatives.isEmpty()) {
    SBG::LIB::Set min_elem_set{representatives.minElem()};
    CompactSet represented{rmap.preImage(min_elem_set)};
    // Get all equations and variables that belong to this loop
    AlgebraicLoop loop;
    for (const SetEdge& se : _hs_info.set_edges()) {
      CompactSet jth_domain = se.domain();
      jth_domain.intersection(represented);
      if (jth_domain.cardinal() > 0) {
        detectLoop(se, loop);
        representatives = representatives.difference(se.domain().set());
      }
    }
    _loops.pushBack(loop);
  }

  return AlgebraicLoopsInfo{_hs_info.set_vertices()
    , _hs_info.set_edges(), scc_result, _loops};
}

} // namespace Causalize

} // namespace Modelica
