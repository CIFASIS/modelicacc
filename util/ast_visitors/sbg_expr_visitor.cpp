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

#include "util/ast_visitors/sbg_expr_visitor.hpp"
#include "util/ast_visitors/eval_expression.hpp"
#include "util/debug.hpp"

#include <sbg/integer.hpp>
#include <sbg/rational.hpp>

namespace Modelica {

ExprVisitor::ExprVisitor(VarSymbolTable symbols
  , std::vector<std::string> order) : _symbols(symbols), _order(order) {}

SBG::LIB::Expression ExprVisitor::operator()(Integer v) const
{
  return SBG::LIB::Expression{SBG::LIB::Rational{0}, SBG::LIB::Rational{v}};
}

SBG::LIB::Expression ExprVisitor::operator()(Boolean v) const
{
  ERROR("ExprVisitor: Boolean is not an affine expression");
  return SBG::LIB::Expression{};
}

SBG::LIB::Expression ExprVisitor::operator()(AddAll v) const 
{
  ERROR("ExprVisitor: AddAll is not an affine expression");
  return SBG::LIB::Expression{};
}

SBG::LIB::Expression ExprVisitor::operator()(String v) const 
{
  ERROR("ExprVisitor: String is not an affine expression");
  return SBG::LIB::Expression{};
}

SBG::LIB::Expression ExprVisitor::operator()(Name v) const 
{
  ERROR("ExprVisitor: Name is not an affine expression");
  return SBG::LIB::Expression{};
}

SBG::LIB::Expression ExprVisitor::operator()(Real v) const 
{
  ERROR("ExprVisitor: Real is not an affine expression");
  return SBG::LIB::Expression{};
}

SBG::LIB::Expression ExprVisitor::operator()(SubEnd v) const
{
  ERROR("ExprVisitor: SubEnd is not an affine expression");
  return SBG::LIB::Expression{};
}

SBG::LIB::Expression ExprVisitor::operator()(SubAll v) const
{
  ERROR("ExprVisitor: SubAll is not an affine expression");
  return SBG::LIB::Expression{};
}

SBG::LIB::Expression ExprVisitor::operator()(IfExp v) const
{
  ERROR("ExprVisitor: IfExp is not an affine expression");
  return SBG::LIB::Expression{};
}

SBG::LIB::Expression ExprVisitor::operator()(Range v) const
{
  ERROR("ExprVisitor: Range is not an affine expression");
  return SBG::LIB::Expression{};
}

SBG::LIB::Expression ExprVisitor::operator()(Brace v) const
{
  ERROR("ExprVisitor: Brace is not an affine expression");
  return SBG::LIB::Expression{};
}

SBG::LIB::Expression ExprVisitor::operator()(Bracket v) const
{
  ERROR("ExprVisitor: Bracket is not an affine expression");
  return SBG::LIB::Expression{};
}

SBG::LIB::Expression ExprVisitor::operator()(Call v) const
{
  ERROR("ExprVisitor: Call is not an affine expression");
  return SBG::LIB::Expression{};
}

SBG::LIB::Expression ExprVisitor::operator()(FunctionExp v) const
{
  ERROR("ExprVisitor: FunctionExp is not an affine expression");
  return SBG::LIB::Expression{};
}

SBG::LIB::Expression ExprVisitor::operator()(ForExp v) const
{
  ERROR("ExprVisitor: ForExp is not an affine expression");
  return SBG::LIB::Expression{};
}

SBG::LIB::Expression ExprVisitor::operator()(Named v) const
{
  ERROR("ExprVisitor: Named is not an affine expression");
  return SBG::LIB::Expression{};
}

SBG::LIB::Expression ExprVisitor::operator()(Output v) const
{
  ERROR("ExprVisitor: Output is not an affine expression");
  return SBG::LIB::Expression{};
}

SBG::LIB::Expression ExprVisitor::operator()(Reference v) const
{
  Name v_name = get<0>(v.ref().front());

  bool in_order = false; // Is counter of a loop?
  for (const std::string& index_name : _order) {
    if (v_name == index_name) {
      in_order = true;
    }
  }

  SBG::LIB::Expression result;
  if (in_order) {
    result = result.cartesianProduct(SBG::LIB::Expression{
      SBG::LIB::Rational{1}, SBG::LIB::Rational{0}
    });
  } else {
    EvalExpression eval_expr{_symbols};
    result = result.cartesianProduct(SBG::LIB::Expression{
      SBG::LIB::Rational{0}, SBG::LIB::Rational{Apply(eval_expr, Expression{v})}
    });
  }

  return result;
}

SBG::LIB::Expression ExprVisitor::operator()(BinOp v) const
{
  Expression l = v.left();
  Expression r = v.right();

  switch (v.op()) {
    case BinOpType::Add: {
      return ApplyThis(l) + ApplyThis(r);
      break;
    }

    case BinOpType::Sub: {
      return ApplyThis(l) - ApplyThis(r);
      break;
    }

    case BinOpType::Mult: {
      //TODO return ApplyThis(l) * ApplyThis(r);
      break;
    }

    default: {
      ERROR("ExprVisitor: operation ", v.op(), " not supported");
      break;
    }
  }

  return SBG::LIB::Expression{};
}

SBG::LIB::Expression ExprVisitor::operator()(UnaryOp v) const
{
  switch (v.op()) {
    case UnaryOpType::Minus: {
      SBG::LIB::Rational zero{0};
      SBG::LIB::Expression expr = ApplyThis(v.exp());
      return SBG::LIB::Expression{expr.arity(), zero, zero};
    }

    default: {
      ERROR("ExprVisitor: operation ", v.op(), " not supported");
      break;
    }
  }

  return SBG::LIB::Expression{};
}

}  // namespace Modelica
