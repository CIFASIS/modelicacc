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

#ifndef MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_TEARING_HPP_
#define MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_TEARING_HPP_

#include "ast/expression.hpp"
#include "causalize/sbg_implementation/algebraic_loops_detection.hpp"
#include "causalize/sbg_implementation/modelica_sbg.hpp"
#include "causalize/sbg_implementation/tearing_variables.hpp"

#include <sbg/set.hpp>

#include <iosfwd>
#include <map>
#include <set>

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// Tearing return structure ----------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

/**
 * @class TearingResult
 * @brief Structure to return after calculating the tearing variables, to pass
 * to the next stages of causalization.
 */
class TearingResult {
public:
  TearingResult(ModelicaSBG modelica_bsbg, SBG::LIB::Set mfvs_result);

  const ModelicaSBG& modelica_bsbg() const;
  const SBG::LIB::Set& mfvs_result() const;

  /**
   * @brief Converts the SBG obtained result to a ModelicaCC set of expressions
   * that represent the tearing variables.
   */
  TearingVariables toModelicaFormat() const;

private:
  TearingVariables variableToModelicaFormat(const SetEdge& se
    , SBG::LIB::Set jth_tear) const;

  ModelicaSBG _modelica_bsbg;
  SBG::LIB::Set _mfvs_result;
};

////////////////////////////////////////////////////////////////////////////////
// Tearing variables detector --------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

/**
 * @class TearingDetector
 * @brief Identifies tearing variables of the model.
 */
class TearingDetector {
public:
  TearingDetector(AlgebraicLoopsResult loops_result);

  TearingResult detect();

private:
  AlgebraicLoopsResult _loops_result;
};

} // namespace Causalize

} // namespace Modelica

#endif // MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_TEARING_HPP_
