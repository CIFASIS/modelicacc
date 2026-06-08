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

#include "ast/equation.hpp"
#include "ast/expression.hpp"
#include "causalize/sbg_implementation/equation_info.hpp"
#include "causalize/sbg_implementation/generate_sbg_input.hpp"
#include "causalize/sbg_implementation/set_edge.hpp"
#include "causalize/sbg_implementation/set_vertex.hpp"
#include "mmo/mmo_class.hpp"
#include "sbg/bipartite_sbg.hpp"

#include <algorithms/matching/match_data.hpp>

#include <iosfwd>
#include <map>
#include <tuple>
#include <vector>

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// Auxiliary definitions -------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

// EqVarMatch ------------------------------------------------------------------

/**
 * @class EqVarMatch
 * @brief Represents the pairing between an equation and the unknown that will
 * be solved for it.
 */
class EqVarMatch {
public:
  EqVarMatch() = default;

  std::size_t size() const;
  std::tuple<Equation, Expression> operator[](std::size_t k) const;

  void pushBack(Equation eq, Expression expr);

private:
  EquationList _equation_list;
  ExpList _expression_list;
};

std::ostream& operator<<(std::ostream& out, const EqVarMatch& match);

// HorizontalSortingInfo -------------------------------------------------------

/**
 * @class HorizontalSortingInfo
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
class HorizontalSortingInfo {
public:
  HorizontalSortingInfo(const std::vector<SetVertex>& set_vertices
    , const std::vector<SetEdge>& set_edges
    , const std::map<int, EquationInfo>& equations_info
    , SBG::LIB::MatchData matching_result
    , EqVarMatch horizontal_sorting);

  const std::vector<SetVertex>& set_vertices() const;
  const std::vector<SetEdge>& set_edges() const;
  const std::map<int, EquationInfo>& equations_info() const;
  const SBG::LIB::MatchData& matching_result() const;
  const EqVarMatch& horizontal_sorting() const;

private:
  const std::vector<SetVertex>& _set_vertices;
  const std::vector<SetEdge>& _set_edges;
  const std::map<int, EquationInfo>& _equations_info;
  SBG::LIB::MatchData _matching_result;
  EqVarMatch _horizontal_sorting;
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
  explicit HorizontalSorting(SBGGenerationInfo& sbg_gen_info);
  ~HorizontalSorting() = default;
  HorizontalSortingInfo sort();

private:
  void sortEquation(SetEdge se, CompactSet se_match);

  SBGGenerationInfo& _sbg_gen_info;
  EqVarMatch _sort; ///< Result of sorting
};

} // namespace Causalize

} // namespace Modelica

#endif // MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_HORIZONTAL_SORTING_HPP_
