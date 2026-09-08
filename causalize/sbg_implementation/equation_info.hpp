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
  EquationInfo(AST::Indexes indices, AST::Equation equation, bool scalar);

  const AST::Indexes& indices() const;
  const AST::Equation& equation() const;
  bool scalar() const;

  /**
   * @brief If it is a scalar equation, it leaves the equation unchanged. If it
   * is an array equation, sets the bounds to \p indexes.
   */
  AST::Equation restrictBounds(const AST::Indexes& indexes) const;

  /**
   * @brief Calculates the transformation to convert \p old_indexes to
   * \p new_indexes, and applies it to the subscripts of _equation.
   *
   * @return Equation that accesses the subscripts described by \p new_indexes.
   */
  AST::Equation adjustSubscripts(
    const AST::Indexes& old_indexes, const AST::Indexes& new_indexes
  ) const;

private:
  AST::Indexes _indices;
  AST::Equation _equation;
  bool _scalar;
};

} // namespace Causalize

} // namespace Modelica

#endif // MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_EQUATION_INFO_HPP_
