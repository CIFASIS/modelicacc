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
#include "causalize/sbg_implementation/causalization_utils.hpp"
#include "causalize/sbg_implementation/equation_info.hpp"
#include "causalize/sbg_implementation/set_edge.hpp"
#include "causalize/sbg_implementation/set_vertex.hpp"
#include "causalize/sbg_implementation/builders/causalization_builders.hpp"
#include "util/debug.hpp"
#include "util/compact_set.hpp"

#include <algorithms/scc/scc.hpp>
#include <sbg/directed_sbg.hpp>
#include <sbg/pw_map.hpp>
#include <sbg/set.hpp>
#include <util/time_profiler.hpp>

#include <algorithm>

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// ModelicaCC algebraic loops --------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

// AlgebraicLoop ---------------------------------------------------------------

AlgebraicLoop::AlgebraicLoop(EquationList equations) : _equations(equations) {}

const EquationList& AlgebraicLoop::equations() const { return _equations; }

bool AlgebraicLoop::isEmpty() const { return _equations.empty(); }

void AlgebraicLoop::pushBack(Equation equation)
{
  _equations.push_back(equation);
}

void AlgebraicLoop::concatenation(AlgebraicLoop other)
{
  _equations.insert(
    _equations.end(), other._equations.begin(), other._equations.end()
  );
}

std::ostream& operator<<(std::ostream& out, const AlgebraicLoop& loop)
{
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

void AlgebraicLoops::pushBack(AlgebraicLoop loop)
{
  if (!loop.isEmpty()) {
    _loops.push_back(loop);
  }
}

void AlgebraicLoops::reverse() { std::reverse(_loops.begin(), _loops.end()); }

void AlgebraicLoops::concatenation(AlgebraicLoops other)
{
  _loops.insert(_loops.end(), other.begin(), other.end());
}

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

AlgebraicLoopsResult::AlgebraicLoopsResult(
  ModelicaSBG modelica_bsbg, SBG::LIB::SCCData scc_result
) : _modelica_bsbg(modelica_bsbg), _scc_result(scc_result) {}

const ModelicaSBG& AlgebraicLoopsResult::modelica_bsbg() const
{
  return _modelica_bsbg;
}

const SBG::LIB::SCCData& AlgebraicLoopsResult::scc_result() const
{
  return _scc_result;
}

EquationList AlgebraicLoopsResult::loopToModelicaFormat(
  const CompactSet& reps, const CompactSet& represented
  , const AST::Indexes& indexes
) const
{
  EquationList result;

  for (const SetEdge& other_se : _modelica_bsbg.set_edges()) {
    CompactSet other_se_scc = other_se.domain();
    other_se_scc.intersection(represented);
    if (other_se_scc.cardinal() > 0) {
      // Bounds for arrays of equations inside an algebraic loop.
      auto [other_eq_info, other_accesses] = getAccess(
        _modelica_bsbg, other_se, other_se_scc
      );
      for (const auto& [_, other_indexes] : other_accesses) {
        if (reps.cardinal() == 1) { // Scalar algebraic loop.
          result.push_back(
            other_eq_info.restrictBounds(other_indexes)
          );
        } else { // Array of algebraic loops.
          ERROR_UNLESS(reps.cardinal() == other_se_scc.cardinal()
          , "AlgebraicLoopsResult::loopToModelicaFormat: incompatible"
          , " repetitives structures");
          result.push_back(other_eq_info
            .adjustSubscripts(indexes, other_indexes));
        }
      }
    }
  }

  return result;
}

AlgebraicLoops AlgebraicLoopsResult::loopsToModelicaFormat(
  const SetEdge& se, const CompactSet& se_scc
) const
{
  // Create the indices for an array of algebraic loops (possibly unary if there
  // is a large algebraic loop).
  auto [eq_info, accesses] = getAccess(_modelica_bsbg, se, se_scc);

  // Get other equations that belong to the current SCCs determined by
  // \p se_scc.
  SBG::LIB::PWMap rmap = _scc_result.rmap();
  AlgebraicLoops result;
  for (const auto& [reps, indexes] : accesses) {
    CompactSet represented{rmap.preImage(reps.set())};
    EquationList eq_list = loopToModelicaFormat(reps, represented, indexes);

    // Bounds for arrays of algebraic loops.
    if (reps.cardinal() == 1) { // Scalar algebraic loop.
      result.pushBack(AlgebraicLoop{eq_list});
    } else { // Array of algebraic loops.
      ForEq for_eq{indexes, eq_list};
      result.pushBack(AlgebraicLoop{EquationList{1, for_eq}});
    }
  }
  return result;
}

AlgebraicLoops AlgebraicLoopsResult::toModelicaFormat() const
{
  AlgebraicLoops result;

  CompactSet representatives{_scc_result.rmap().fixedPoints()};
  for (const SetEdge& se : _modelica_bsbg.set_edges()) {
    CompactSet se_scc = se.domain();
    se_scc.intersection(representatives);
    if (se_scc.cardinal() > 0) {
      result.concatenation(loopsToModelicaFormat(se, se_scc));
    }
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
  SBG::LIB::DirectedSBG loops_dsbg;
  {
    SBG::Util::Internal::TimeProfiler profiler{"Algebraic loops SBG builder"};
    loops_dsbg = buildLoopDetectionSBG(_hs_result.matching_result());
  }
  SBG::LIB::SCCData scc_result{SBG::LIB::DirectedSBG{}, SBG::LIB::PWMap{}
    , SBG::LIB::Set{}};
  {
    SBG::Util::Internal::TimeProfiler profiler{"Algebraic loops detection"};
    scc_result = SBG::LIB::SCC{}.calculate(loops_dsbg);
  }

  return AlgebraicLoopsResult{_hs_result.modelica_bsbg(), scc_result};
}

}  // namespace Causalize

}  // namespace Modelica
