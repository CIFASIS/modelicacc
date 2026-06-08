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

#ifndef MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_EQUATION_INFO_HPP_
#define MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_EQUATION_INFO_HPP_

#include "ast/expression.hpp"
#include "ast/equation.hpp"

namespace Modelica {

namespace Causalize {

/**
 * @brief Helper that saves all Modelica information about an array of equations
 * (the array can be of size 1). It saves the indices of the array and the
 * contained equation separately.
 */
class EquationInfo {
public:
  EquationInfo() = default;
  EquationInfo(AST::IndexList indices, AST::Equation equation);

  const AST::IndexList& indices() const;
  const AST::Equation& equation() const;

  AST::Equation restrictEquation(const AST::Indexes& indexes) const;

private:
  AST::IndexList _indices;
  AST::Equation _equation;
};

} // namespace Causalize

} // namespace Modelica

#endif // MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_EQUATION_INFO_HPP_
