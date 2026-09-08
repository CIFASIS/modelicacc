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

#ifndef MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_BUILDERS_CAUSALIZATION_BUILDERS_HPP_
#define MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_BUILDERS_CAUSALIZATION_BUILDERS_HPP_

#include <algorithms/matching/match_data.hpp>
#include <algorithms/scc/scc_data.hpp>
#include <sbg/directed_sbg.hpp>

#include <tuple>
#include <vector>

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// Algebraic loops detection builder -------------------------------------------
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Builds the directed SBG used to detect algebraic loops.
 * 
 * It merges the matched edges of the input SBG of \p data, adding them
 * as vertices of the new graphs. Then, it adds an edge (u, v) if in the
 * input bipartite SBG there was an unmatched edge between the matched edges
 * represented by u and v. The direction of (u, v) is from right to left
 * according to the input bipartite SBG, representing that the equation
 * referenced by u must be solved before the equation referenced by v.
 */
SBG::LIB::DirectedSBG buildLoopDetectionSBG(const SBG::LIB::MatchData& data);

////////////////////////////////////////////////////////////////////////////////
// Algebraic loops breaker builder ---------------------------------------------
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Builds the directed SBG used to identify tearing variables.
 */
SBG::LIB::DirectedSBG buildTearingSBG(const SBG::LIB::SCCData& data);

////////////////////////////////////////////////////////////////////////////////
// Vertical sorting builder ----------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

/**
 * @class VerticalSortingBuilder
 * @brief Builds the directed acyclic SBG used to order vertically equations.
 */
class VerticalSortingBuilder {
public:
  VerticalSortingBuilder(const SBG::LIB::SCCData& data
    , const SBG::LIB::Set& mfvs);

  void build();

  const SBG::LIB::DirectedSBG& dsbg() const;
  const SBG::LIB::PWMap& rmap() const;
  const SBG::LIB::Set& residual_vertices() const;
  const SBG::LIB::PWMap& guess_offset() const;
  const SBG::LIB::Set& end_points() const;

private:
  void addGuessVertices();

  /**
   * @brief Adds edges between tearing variables to induce an order between
   * them.
   */
  void addDependencies(const SBG::LIB::PWMap& residual_to_endpoint);

  /**
   * @brief Modify edges between different SCC so that the endings are start and
   * end points.
   */
  void redirectEdiff(const SBG::LIB::PWMap& reps_to_endpoint);

  SBG::LIB::DirectedSBG _input_dsbg;
  SBG::LIB::PWMap _input_rmap;
  SBG::LIB::Set _Ediff;
  SBG::LIB::DirectedSBG _output_dsbg;
  SBG::LIB::PWMap _output_rmap;
  SBG::LIB::Set _residual_vertices;
  SBG::LIB::PWMap _guess_offset;
  SBG::LIB::Set _end_points;
};

} // namespace Causalize

} // namespace Modelica

#endif // MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_BUILDERS_CAUSALIZATION_BUILDERS_HPP_
