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
// Algebraic loops return structure --------------------------------------------
////////////////////////////////////////////////////////////////////////////////

// Constructors/Destructors ----------------------------------------------------

AlgebraicLoopsResult::AlgebraicLoopsResult(
  ModelicaSBG modelica_bsbg, SBG::LIB::SCCData scc_result
) : _modelica_bsbg(modelica_bsbg), _scc_result(scc_result) {}

// Getters ---------------------------------------------------------------------

const ModelicaSBG& AlgebraicLoopsResult::modelica_bsbg() const
{
  return _modelica_bsbg;
}

const SBG::LIB::SCCData& AlgebraicLoopsResult::scc_result() const
{
  return _scc_result;
}

// Extra functions -------------------------------------------------------------

namespace {

AST::Equation innerBounds(
  const EquationInfo& eq_info, const CompactSet& reps
  , const AST::Indexes& indexes, const AST::Indexes& other_indexes
)
{
  if (reps.cardinal() == 1) { // Scalar algebraic loop.
    return eq_info.restrictBounds(other_indexes);
  }

  // Array of algebraic loops.
  return eq_info.adjustSubscripts(indexes, other_indexes);
}

} // namespace

EquationList AlgebraicLoopsResult::loopToModelicaFormat(
  const CompactSet& reps, const CompactSet& represented
  , const AST::Indexes& indexes
) const
{
  EquationList result;

  for (const SetEdge& se : _modelica_bsbg.set_edges()) {
    CompactSet se_scc = se.translatedDomain();
    se_scc.intersection(represented);
    if (se_scc.cardinal() > 0) {
      // Bounds for arrays of equations inside an algebraic loop.
      auto [eq_info, accesses] = getAccess(_modelica_bsbg, se, se_scc);
      for (const auto& [_, other_indexes] : accesses) {
        result.push_back(innerBounds(eq_info, reps, indexes, other_indexes));
      }
    }
  }

  return result;
}

namespace {

AlgebraicLoop outerBounds(
  const CompactSet& reps, const AST::EquationList& eq_list, const AST::Indexes indexes
)
{
  // Bounds for arrays of algebraic loops.
  if (reps.cardinal() == 1) { // Scalar algebraic loop.
    return AlgebraicLoop{eq_list};
  }

  // Array of algebraic loops.
  ForEq for_eq{indexes, eq_list};
  return AlgebraicLoop{EquationList{1, for_eq}};
}

} // namespace

AlgebraicLoops AlgebraicLoopsResult::loopsToModelicaFormat(
  const SetEdge& se, const CompactSet& se_scc
) const
{
  // Create the indices for an array of algebraic loops (possibly empty if there
  // is a large algebraic loop).
  auto [eq_info, accesses] = getAccess(_modelica_bsbg, se, se_scc);

  // Get other equations that belong to the current SCCs determined by
  // \p se_scc.
  AlgebraicLoops result;
  const SBG::LIB::PWMap& rmap = _scc_result.rmap();
  for (const auto& [reps, indexes] : accesses) {
    CompactSet represented{rmap.preImage(reps.set())};
    EquationList eq_list = loopToModelicaFormat(reps, represented, indexes);
    result.pushBack(outerBounds(reps, eq_list, indexes));
  }
  return result;
}

AlgebraicLoops AlgebraicLoopsResult::toModelicaFormat() const
{
  AlgebraicLoops result;

  CompactSet representatives{_scc_result.rmap().fixedPoints()};
  for (const SetEdge& se : _modelica_bsbg.set_edges()) {
    CompactSet se_scc = se.translatedDomain();
    se_scc.intersection(representatives);
    if (se_scc.cardinal() > 0) {
      result.concatenation(loopsToModelicaFormat(se, se_scc));
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
// Algebraic loops detector ----------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

AlgebraicLoopsDetector::AlgebraicLoopsDetector(
  HorizontalSortingResult& hs_result
) : _hs_result(hs_result) {}

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
