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

#include <string>

namespace Modelica {

namespace Causalize {

EquationInfo::EquationInfo(AST::IndexList indices, AST::Equation equation, bool scalar)
  : _indices(indices), _equation(equation), _scalar(scalar) {}

const AST::IndexList& EquationInfo::indices() const { return _indices; }

const AST::Equation& EquationInfo::equation() const { return _equation; }

bool EquationInfo::scalar() const { return _scalar; }

AST::Equation EquationInfo::restrictEquation(const AST::Indexes& indexes) const
{
  AST::Equation result;

  // Take out dummy counters that were added during SBG generation
  AST::IndexList index_list;
  for (const AST::Index& index : _indices) {
    if (index.name().substr(0, 6) != "*dummy") {
      index_list.push_back(index);
    }
  }
  AST::Indexes for_indices{index_list};

  if (for_indices.indexes().empty()) { // Scalar equation
    result = _equation;
  } else { // Array equation
    result = AST::ForEq{indexes, AST::EquationList{1, _equation}};
  }

  return result;
}

} // namespace Causalize

} // namespace Modelica
