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

#include "util/ast_visitors/flatter_for.hpp"
#include "util/debug.hpp"

namespace Modelica {

AST::EquationList FlatterForVisitor::bounds(AST::Equation eq)
{
  AST::EquationList result;
  if (_bounds.empty()) {
    result.push_back(eq);
  } else {
    result.push_back(AST::ForEq{AST::Indexes{_bounds}, AST::EquationList{eq}});
  }

  return result;
}

AST::EquationList FlatterForVisitor::operator()(AST::Connect eq)
{
  return bounds(eq);
}

AST::EquationList FlatterForVisitor::operator()(AST::Equality eq)
{
  return bounds(eq);
}

AST::EquationList FlatterForVisitor::operator()(AST::CallEq eq)
{
  return bounds(eq);
}

AST::EquationList FlatterForVisitor::operator()(AST::IfEq eq)
{
  return bounds(eq);
}

AST::EquationList FlatterForVisitor::operator()(AST::WhenEq eq)
{
  return bounds(eq);
}

AST::EquationList FlatterForVisitor::operator()(AST::ForEq eq)
{
  AST::EquationList result;
  AST::IndexList eq_bounds = eq.range().indexes();
  _bounds.insert(_bounds.end(), eq_bounds.begin(), eq_bounds.end());
  for (const AST::Equation& jth_eq : eq.elements()) {
    AST::EquationList jth_eq_result = ApplyThis(jth_eq);
    result.insert(result.end(), jth_eq_result.begin(), jth_eq_result.end());
  }
  return result;
}

}  // namespace Modelica
