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

#include "causalize/sbg_implementation/ast_visitors/residual_equation.hpp"
#include "util/debug.hpp"

namespace Modelica {

namespace Causalize {

ResidualEqVisitor::ResidualEqVisitor(AST::Expression expr)
  : _original_expr(expr) {}

AST::Equation ResidualEqVisitor::operator()(AST::Connect eq)
{
  ERROR("ResidualEqVisitor: Connect not yet supported");
  return AST::Equation{};
}

AST::Equation ResidualEqVisitor::operator()(AST::Equality eq)
{
  AST::Expression left{AST::BinOp{
    AST::Call{"res", _original_expr}, AST::BinOpType::Add, eq.left()
  }};

  return AST::Equation{AST::Equality{left, eq.right()}};
}

AST::Equation ResidualEqVisitor::operator()(AST::CallEq eq)
{
  ERROR("ResidualEqVisitor: trying to convert a CallEq");
  return AST::Equation{};
}

AST::Equation ResidualEqVisitor::operator()(AST::ForEq eq)
{
  ERROR("ResidualEqVisitor: trying to convert a ForEq");
  return AST::Equation{};
}

AST::Equation ResidualEqVisitor::operator()(AST::IfEq eq)
{
  ERROR("ResidualEqVisitor: trying to convert an IfEq");
  return AST::Equation{};
}

AST::Equation ResidualEqVisitor::operator()(AST::WhenEq eq)
{
  ERROR("ResidualEqVisitor: trying to convert a WhenEq");
  return AST::Equation{};
}

} // namespace Causalize

} // namespace Modelica
