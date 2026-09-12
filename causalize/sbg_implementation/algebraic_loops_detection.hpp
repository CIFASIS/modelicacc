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

#ifndef MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_ALGEBRAIC_LOOPS_DETECTION_HPP_
#define MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_ALGEBRAIC_LOOPS_DETECTION_HPP_

#include "causalize/sbg_implementation/algebraic_loops.hpp"
#include "causalize/sbg_implementation/horizontal_sorting.hpp"
#include "causalize/sbg_implementation/modelica_sbg.hpp"
#include "causalize/sbg_implementation/set_edge.hpp"

#include <algorithms/scc/scc_data.hpp>
#include <sbg/set.hpp>

#include <iosfwd>
#include <vector>

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// Algebraic loops return structure --------------------------------------------
////////////////////////////////////////////////////////////////////////////////

/**
 * @class AlgebraicLoopsResult
 * @brief Structure to return after calculating the algebraic loops, to pass to
 * the next stages of causalization.
 */
class AlgebraicLoopsResult {
public:
  AlgebraicLoopsResult(ModelicaSBG modelica_bsbg, SBG::LIB::SCCData scc_result);

  const ModelicaSBG& modelica_bsbg() const;
  const SBG::LIB::SCCData& scc_result() const;

  /**
   * @brief Converts the SBG result to a ModelicaCC representation of the
   * algebraic loops.
   */
  AlgebraicLoops toModelicaFormat() const;

private:
  /**
   * @brief Converts a single algebraic loop, or a single array of algebraic
   * loops.
   */
  EquationList loopToModelicaFormat(
    const SBG::LIB::Set& reps, const SBG::LIB::Set& represented
    , const AST::Indexes& indexes
  ) const;

  /**
   * @brief Converts all of the scalar (or array of) algebraic loops described
   * by a single set-edge, i.e. equations that share a repetitive definition. 
   */
  AlgebraicLoops loopsToModelicaFormat(
    const SetEdge& se, const SBG::LIB::Set& s
  ) const;

  ModelicaSBG _modelica_bsbg; ///< Output SBG after matching.
  SBG::LIB::SCCData _scc_result;
};

////////////////////////////////////////////////////////////////////////////////
// Algebraic loops detection ---------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

/**
 * @class AlgebraicLoopsDetector
 * @brief Identifies the algebraic loops of the model. 
 */
class AlgebraicLoopsDetector {
public:
  AlgebraicLoopsDetector(HorizontalSortingResult& hs_result);

  AlgebraicLoopsResult detect();

private:
  HorizontalSortingResult& _hs_result;
};

} // namespace Causalize

} // namespace Modelica

#endif // MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_ALGEBRAIC_LOOPS_DETECTION_HPP_
