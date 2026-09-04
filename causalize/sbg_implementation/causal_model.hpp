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

#ifndef MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_CAUSAL_MODEL_HPP_
#define MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_CAUSAL_MODEL_HPP_

#include "ast/equation.hpp"
#include "ast/expression.hpp"
#include "causalize/sbg_implementation/model_match.hpp"

#include <iosfwd>
#include <tuple>
#include <vector>

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// ModelicaCC group of causalized equations ------------------------------------
////////////////////////////////////////////////////////////////////////////////

using EqVarMatchs = std::vector<EqVarMatch>;

/**
 * @class SortedAlgebraicLoop
 * @brief Ordered collection of (equation, variables) pairings that must be
 * solved simultaneously.
 *
 * This has to be done after a pairing of equations and variables has been
 * decided, and the algebraic loops have been detected.
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
class SortedAlgebraicLoop {
public:
  SortedAlgebraicLoop() = default;
  SortedAlgebraicLoop(EqVarMatchs matchs);

  const EqVarMatchs& matchs() const;
  bool empty() const;

  void pushBack(EqVarMatch match);

  void concatenation(SortedAlgebraicLoop other);

private:
  EqVarMatchs _matchs;
};

std::ostream& operator<<(std::ostream& out, const SortedAlgebraicLoop& loop);

////////////////////////////////////////////////////////////////////////////////
// ModelicaCC causalized model -------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

using SortedAlgebraicLoops = std::vector<SortedAlgebraicLoop>;

/**
 * @class CausalModel
 * @brief Final causalized model that can be used to generate code.
 */
class CausalModel {
public:
  CausalModel() = default;

  std::size_t size() const;
  SortedAlgebraicLoop operator[](std::size_t k) const;

  auto begin() const { return _loops.begin(); }
  auto end() const { return _loops.end(); }

  void pushBack(SortedAlgebraicLoop loop);

private:
  SortedAlgebraicLoops _loops;
};

std::ostream& operator<<(std::ostream& out, const CausalModel& causal_model);

} // namespace Causalize

} // namespace Modelica

#endif // MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_CAUSAL_MODEL_HPP_
