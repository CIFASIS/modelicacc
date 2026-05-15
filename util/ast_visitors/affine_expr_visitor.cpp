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

#include "util/ast_visitors/affine_expr_visitor.hpp"
#include "util/ast_visitors/eval_expression.hpp"
#include "util/debug.hpp"

namespace Modelica {

AffineExprVisitor::AffineExprVisitor(VarSymbolTable symbols
  , std::vector<std::string> order) : _symbols(symbols), _order(order) {}

Util::AffineExpr AffineExprVisitor::operator()(Integer v) const
{
  Util::AffineExpr result{_order};
  result.set_offset(v);
  return result;
}

Util::AffineExpr AffineExprVisitor::operator()(Boolean v) const
{
  ERROR("AffineExprVisitor: Boolean is not an affine expression");
  return Util::AffineExpr{};
}

Util::AffineExpr AffineExprVisitor::operator()(AddAll v) const 
{
  ERROR("AffineExprVisitor: AddAll is not an affine expression");
  return Util::AffineExpr{};
}

Util::AffineExpr AffineExprVisitor::operator()(String v) const 
{
  ERROR("AffineExprVisitor: String is not an affine expression");
  return Util::AffineExpr{};
}

Util::AffineExpr AffineExprVisitor::operator()(Name v) const 
{
  ERROR("AffineExprVisitor: Name is not an affine expression");
  return Util::AffineExpr{};
}

Util::AffineExpr AffineExprVisitor::operator()(Real v) const 
{
  ERROR("AffineExprVisitor: Real is not an affine expression");
  return Util::AffineExpr{};
}

Util::AffineExpr AffineExprVisitor::operator()(SubEnd v) const
{
  ERROR("AffineExprVisitor: SubEnd is not an affine expression");
  return Util::AffineExpr{};
}

Util::AffineExpr AffineExprVisitor::operator()(SubAll v) const
{
  ERROR("AffineExprVisitor: SubAll is not an affine expression");
  return Util::AffineExpr{};
}

Util::AffineExpr AffineExprVisitor::operator()(IfExp v) const
{
  ERROR("AffineExprVisitor: IfExp is not an affine expression");
  return Util::AffineExpr{};
}

Util::AffineExpr AffineExprVisitor::operator()(Range v) const
{
  ERROR("AffineExprVisitor: Range is not an affine expression");
  return Util::AffineExpr{};
}

Util::AffineExpr AffineExprVisitor::operator()(Brace v) const
{
  ERROR("AffineExprVisitor: Brace is not an affine expression");
  return Util::AffineExpr{};
}

Util::AffineExpr AffineExprVisitor::operator()(Bracket v) const
{
  ERROR("AffineExprVisitor: Bracket is not an affine expression");
  return Util::AffineExpr{};
}

Util::AffineExpr AffineExprVisitor::operator()(Call v) const
{
  ERROR("AffineExprVisitor: Call is not an affine expression");
  return Util::AffineExpr{};
}

Util::AffineExpr AffineExprVisitor::operator()(FunctionExp v) const
{
  ERROR("AffineExprVisitor: FunctionExp is not an affine expression");
  return Util::AffineExpr{};
}

Util::AffineExpr AffineExprVisitor::operator()(ForExp v) const
{
  ERROR("AffineExprVisitor: ForExp is not an affine expression");
  return Util::AffineExpr{};
}

Util::AffineExpr AffineExprVisitor::operator()(Named v) const
{
  ERROR("AffineExprVisitor: Named is not an affine expression");
  return Util::AffineExpr{};
}

Util::AffineExpr AffineExprVisitor::operator()(Output v) const
{
  ERROR("AffineExprVisitor: Output is not an affine expression");
  return Util::AffineExpr{};
}

Util::AffineExpr AffineExprVisitor::operator()(Reference v) const
{
  Util::AffineExpr result{_order};
  Name v_name = get<0>(v.ref().front());

  bool in_order = false; // Is counter of a loop?
  for (const std::string& index_name : _order) {
    if (v_name == index_name) {
      in_order = true;
    }
  }

  if (in_order) {
    result.set_slope(v_name, 1);
  } else {
    EvalExpression eval_expr{_symbols};
    result.set_offset(static_cast<AST::Integer>(Apply(eval_expr, Expression{v})));
  }

  return result;
}

Util::AffineExpr AffineExprVisitor::operator()(BinOp v) const
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
      return ApplyThis(l) * ApplyThis(r);
      break;
    }

    default: {
      ERROR("AffineExprVisitor: operation ", v.op(), " not supported");
      break;
    }
  }

  return Util::AffineExpr{};
}

Util::AffineExpr AffineExprVisitor::operator()(UnaryOp v) const
{
  switch (v.op()) {
    case UnaryOpType::Minus: {
      return -ApplyThis(v.exp());
    }

    default: {
      ERROR("AffineExprVisitor: operation ", v.op(), " not supported");
      break;
    }
  }

  return Util::AffineExpr{};
}

}  // namespace Modelica
