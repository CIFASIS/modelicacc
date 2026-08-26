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

/**
 * @class AlgebraicLoop
 * @brief Group of equations that must be solved simultaneously.
 *
 * This has to be done after a pairing of equations and variables has been
 * decided. So this result makes sense only in conjunction with that of the
 * matching stage.
 *
 * This structure describes large loops and also arrays of small algebraic
 * loops. For example:
 *   for ... loop
 *     eq1;
 *     ...;
 *     eqk;
 *   end for;
 * describes an array of small algebraic loops. So, here eq1[i], ..., eq[k]
 * must be solved together for all i described by the loop bounds. On the other
 * hand:
 *   eq1;
 *   ...;
 *   eqk;
 * describes a large algebraic loop. If eqi is an array equation, then
 * eqi[1] to eqi[j] must be solved together with the other remaining eqm.
 */
class AlgebraicLoop {
public:
  AlgebraicLoop() = default;
  AlgebraicLoop(EquationList equations);

  const EquationList& equations() const;
  bool isEmpty() const;

  void pushBack(Equation eq);

  void concatenation(AlgebraicLoop other);

private:
  EquationList _equations;
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

  void concatenation(AlgebraicLoops other);

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
  /**
   * @brief Converts a single algebraic loop, or a single array of algebraic
   * loops.
   */
  EquationList loopToModelicaFormat(
    const CompactSet& reps, const CompactSet& represented
    , const AST::Indexes& indexes
  ) const;

  /**
   * @brief Converts all of the scalar (or array of) algebraic loops described
   * by a single set-edge, i.e. equations that share a repetitive definition. 
   */
  AlgebraicLoops loopsToModelicaFormat(
    const SetEdge& se, const CompactSet& s
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
