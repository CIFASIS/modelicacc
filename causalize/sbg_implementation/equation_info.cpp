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

#include "causalize/sbg_implementation/equation_info.hpp"
#include "util/debug.hpp"

#include <string>

namespace Modelica {

namespace Causalize {

EquationInfo::EquationInfo(AST::Indexes indices, AST::Equation equation, bool scalar)
  : _indices(indices), _equation(equation), _scalar(scalar) {}

const AST::Indexes& EquationInfo::indices() const { return _indices; }

const AST::Equation& EquationInfo::equation() const { return _equation; }

bool EquationInfo::scalar() const { return _scalar; }

AST::Equation EquationInfo::restrictBounds(const AST::Indexes& indexes) const
{
  AST::Equation result;

  if (_scalar) { // Scalar equation.
    result = _equation;
  } else { // Array equation.
    result = AST::ForEq{indexes, AST::EquationList{1, _equation}};
  }

  return result;
}

AST::Equation EquationInfo::adjustSubscripts(
  const AST::Indexes& old_indexes, const AST::Indexes& new_indexes
) const
{
  ERROR_UNLESS(old_indexes == new_indexes, "EquationInfo::adjustSubscripts:"
    , " conversion not yet supported");
  return _equation;
}

} // namespace Causalize

} // namespace Modelica
