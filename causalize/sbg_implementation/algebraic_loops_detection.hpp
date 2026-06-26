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

#include "ast/equation.hpp"
#include "ast/expression.hpp"
#include "causalize/sbg_implementation/horizontal_sorting.hpp"
#include "causalize/sbg_implementation/modelica_sbg.hpp"
#include "causalize/sbg_implementation/set_edge.hpp"

#include <algorithms/scc/scc_data.hpp>

#include <iosfwd>
#include <vector>

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// ModelicaCC algebraic loops --------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

class AlgebraicLoop {
public:
  AlgebraicLoop() = default;
  AlgebraicLoop(EquationList equations, ExpList variables);

  const EquationList& equations() const;
  const ExpList& variables() const;
  bool isEmpty() const;

  void pushBack(Equation eq, Expression variable);

  void concatenation(AlgebraicLoop other);

private:
  EquationList _equations;
  ExpList _variables;
};

std::ostream& operator<<(std::ostream& out, const AlgebraicLoop& loop);

/**
 * @class AlgebraicLoops
 * @brief Unordered collection of algebraic loops of the model.
 */
class AlgebraicLoops {
public:
  AlgebraicLoops() = default;

  std::size_t size() const;
  AlgebraicLoop operator[](std::size_t k) const;
  auto begin() const { return _loops.begin(); };
  auto end() const { return _loops.end(); };

  void pushBack(AlgebraicLoop loop);
  void reverse();

private:
  std::vector<AlgebraicLoop> _loops;
};

std::ostream& operator<<(std::ostream& out, const AlgebraicLoops& loops);

////////////////////////////////////////////////////////////////////////////////
// Algebraic loops return structure --------------------------------------------
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Type for a single algebraic loop, using SBG structures.
 */
using LoopT = std::vector<SBG::LIB::Set>;

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

  /**
   * @brief Converts the SBG result to an intermediate result that only uses SBG
   * structures. It will be used by the VerticalSorting module.
   */
  std::vector<LoopT> toSBGFormat() const;

private:
  AlgebraicLoop loopToModelicaFormat(const SetEdge& se) const;

  ModelicaSBG _modelica_bsbg; ///< TODO
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
