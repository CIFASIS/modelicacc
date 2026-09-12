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

#ifndef MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_HORIZONTAL_SORTING_HPP_
#define MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_HORIZONTAL_SORTING_HPP_

#include "causalize/sbg_implementation/generate_sbg_input.hpp"
#include "causalize/sbg_implementation/modelica_sbg.hpp"
#include "causalize/sbg_implementation/model_match.hpp"
#include "causalize/sbg_implementation/set_edge.hpp"

#include <algorithms/matching/match_data.hpp>
#include <sbg/set.hpp>

#include <iosfwd>
#include <vector>

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// Horizontal sorting return structure -----------------------------------------
////////////////////////////////////////////////////////////////////////////////

/**
 * @class HorizontalSortingResult
 * @brief Saves information of the newly generated equations from the matching.
 * For example, if the array equation:
 *  for i in 1:N loop
 *    a[i] + b[i] = 0;
 *  end for;
 * matches with a[1:N/2] and b[N/2+1:N], it will be splitted into:
 *  for i in 1:N/2 loop
 *    a[i] + b[i] = 0;
 *  end for;
 *  for i in N/2+1:N loop
 *    a[i] + b[i] = 0;
 *  end for;
 * so, this structure keeps track of how set-vertices and set-edges are modified
 * to achieve this. 
 */
class HorizontalSortingResult {
public:
  HorizontalSortingResult(ModelicaSBG modelica_bsbg
    , SBG::LIB::MatchData matching_result);

  const ModelicaSBG& modelica_bsbg() const;
  const SBG::LIB::MatchData& matching_result() const;

  /**
   * @brief Converts the SBG result to a ModelicaCC representation of the
   * matching between equations and variables.
   */
  ModelMatch toModelicaFormat() const;

private:
  ModelMatch equationToModelicaFormat(SetEdge se, SBG::LIB::Set se_match) const;

  ModelicaSBG _modelica_bsbg; ///< Resulting bipartite Modelica SBG
    ///< after horizontal sorting
  SBG::LIB::MatchData _matching_result; ///< SBG matching result
};

////////////////////////////////////////////////////////////////////////////////
// Horizontal Sorting ----------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

/**
 * @class HorizontalSorting
 * @brief Returns a pairing between variables and the equations from which
 * they can be solved.
 */
class HorizontalSorting {
public:
  HorizontalSorting(SBGGenerationResult& sbg_generation_result);

  HorizontalSortingResult sort();

private:
  SBGGenerationResult& _sbg_generation_result;
};

} // namespace Causalize

} // namespace Modelica

#endif // MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_HORIZONTAL_SORTING_HPP_
