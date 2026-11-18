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

/**
 * @file
 * @brief Ouput representation of Modelica code after performing horizontal
 * sorting.
 */

#ifndef MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_MODEL_MATCH_HPP_
#define MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_MODEL_MATCH_HPP_

#include "ast/equation.hpp"
#include "ast/expression.hpp"

#include <iosfwd>
#include <vector>

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// ModelicaCC variable-equation matching ---------------------------------------
////////////////////////////////////////////////////////////////////////////////

/**
 * @class EqVarMatch
 * @brief Represents the pairing between an equation and the unknown that will
 * be solved for it.
 *
 * As there equations such as for equations that have a list of equations
 * inside, we use an ExpList to indicate which variable will be solved for
 * each element of the list.
 */
class EqVarMatch {
public:
  EqVarMatch(AST::Equation equation, AST::ExpList variable);

  const AST::Equation& equation() const;
  const AST::ExpList& variables() const;

private:
  AST::Equation _equation;
  AST::ExpList _variables;
};

std::ostream& operator<<(std::ostream& out, const EqVarMatch& match);

////////////////////////////////////////////////////////////////////////////////
// ModelicaCC matching of a model ----------------------------------------------
////////////////////////////////////////////////////////////////////////////////

/**
 * @class ModelMatch
 * @brief Represents the pairing between equations and variables for a whole
 * Modelica model.
 */
class ModelMatch {
public:
  ModelMatch() = default;

  std::size_t size() const;
  EqVarMatch operator[](std::size_t k) const;

  void pushBack(EqVarMatch match);

  void concatenation(ModelMatch other);

private:
  std::vector<EqVarMatch> _model_match;
};

std::ostream& operator<<(std::ostream& out, const ModelMatch& match);

} // namespace Causalize

} // namespace Modelica

#endif // MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_MODEL_MATCH_HPP_
