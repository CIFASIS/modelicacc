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

#include "ast/queries.hpp"
#include "util/ast_visitors/compact_set_visitor.hpp"
#include "util/ast_visitors/contains_expression.hpp"
#include "util/ast_visitors/eval_expression.hpp"
#include "util/ast_visitors/eval_integer.hpp"
#include "util/ast_visitors/is_integer.hpp"
#include "util/debug.hpp"

namespace Modelica {

CompactSetVisitor::CompactSetVisitor(const VarSymbolTable& symbols)
  : _symbols(symbols) {}

CompactSet CompactSetVisitor::operator()(Integer v)
{
  return CompactSet{v, 1, v};
}

CompactSet CompactSetVisitor::operator()(Boolean v)
{
  if (v.val()) {
    return CompactSet{1, 1, 1};
  }
  return CompactSet{0, 1, 0};
}

CompactSet CompactSetVisitor::operator()(AddAll v) 
{
  ERROR("CompactSetVisitor: trying to visit an AddAll ", v);
  return CompactSet{};
}

CompactSet CompactSetVisitor::operator()(String v) 
{
  ERROR("CompactSetVisitor: trying to visit a String ", v);
  return CompactSet{};
}

CompactSet CompactSetVisitor::operator()(Name v) 
{
  ERROR("CompactSetVisitor: trying to visit a Name ", v);
  return CompactSet{};
}

CompactSet CompactSetVisitor::operator()(Real v) 
{
  EvalInteger eval_int{_symbols};
  Integer value = Apply(eval_int, Expression{v});
  return CompactSet{value, 1, value};
}

CompactSet CompactSetVisitor::operator()(SubEnd v)
{
  ERROR("CompactSetVisitor: SubEnd not supported");
  return CompactSet{};
}

CompactSet CompactSetVisitor::operator()(SubAll v)
{
  return CompactSet{1, 1, _dimension_size};
}

CompactSet CompactSetVisitor::operator()(IfExp v)
{
  EvalExpression eval_expr{_symbols};
  Real cond = Apply(eval_expr, v.cond());
  if (cond == 1.0) {
    return ApplyThis(v.then());
  }

  for (const ExpPair& elseif : v.elseif()) {
    cond = Apply(eval_expr, get<0>(elseif));
    if (cond == 1.0) {
      return ApplyThis(get<1>(elseif));
    }
  }

  return ApplyThis(v.elseexp());
}

CompactSet CompactSetVisitor::operator()(Range v)
{
  EvalInteger eval_int(_symbols);
  Integer start = Integer(Apply(eval_int, v.start()));
  Integer step = 1;
  Integer end = Integer(Apply(eval_int, v.end()));
  if (v.step()) {
    step = Integer(Apply(eval_int, v.step().get()));
  }

  if (start > end) { // Decreasing range
    ERROR("CompactSetVisitor: only increasing Range expressions supported");
  }

  return CompactSet{start, step, end};
}

CompactSet CompactSetVisitor::operator()(Brace v)
{
  CompactSet result;

  for (const Expression& expr : v.args()) {
    result.setUnion(ApplyThis(expr));
  }

  return result;
}

CompactSet CompactSetVisitor::operator()(Bracket v)
{
  CompactSet result;

  for (const ExpList& expr_list : v.args()) {
    CompactSet jth_set;
    for (const Expression& expr : expr_list) {
      jth_set.setUnion(ApplyThis(expr));
    }
    result.cartesianProduct(jth_set);
  }

  return result;
}

CompactSet CompactSetVisitor::operator()(Call v)
{
  EvalInteger eval_int{_symbols};
  Integer value = Apply(eval_int, Expression{v});
  return CompactSet{value, 1, value};
}

CompactSet CompactSetVisitor::operator()(FunctionExp v)
{
  ERROR("CompactSetVisitor: FunctionExp ", v, " inside loop indices");
  return CompactSet{};
}

CompactSet CompactSetVisitor::operator()(ForExp v)
{
  ERROR("CompactSetVisitor: ForExp not supported yet");
  return CompactSet{};
}

CompactSet CompactSetVisitor::operator()(Named v)
{
  ERROR("CompactSetVisitor: Named not supported");
  return CompactSet{};
}

CompactSet CompactSetVisitor::operator()(Output v)
{
  ERROR_UNLESS(v.args().size() == 1
    , "CompactSetVisitor: Output expressions with more than one element not "
    , "supported");

  if (v.args().front()) {
    return ApplyThis(v.args().front().value());
  }

  ERROR("CompactSetVisitor: Output with no expression");
  return CompactSet{};
}

CompactSet CompactSetVisitor::operator()(Reference v)
{
  CompactSet result;

  Ref ref = v.ref();
  ERROR_UNLESS(ref.size() == 1, "CompactSetVisitor: conversion of dotted "
    , "references not implemented");
  Option<ExpList> opt_subs = get<1>(ref[0]);
  ERROR_UNLESS(opt_subs.has_value() ? opt_subs.value().empty() : true
    , "CompactSetVisitor: conversion of subscripted references not implemented");

  if (!opt_subs) { // Non-subscripted access
    Name v_name = get<0>(ref[0]);
    ERROR_UNLESS(_symbols[v_name].has_value(), "CompactSetVisitor: variable "
      , v, " not found");
    Option<ExpList> v_indices = _symbols[v_name].value().indices();
    if (v_indices) { // Access to array variable
      std::size_t k = 0;
      for (const Expression& index : v_indices.value()) {
        if (k == 0) {
          result = ApplyThis(index);
        } else {
          result.cartesianProduct(ApplyThis(index));
        }
        ++k;
      }
    } else { // Access to scalar variable
      EvalInteger eval_int{_symbols};
      Integer value = Apply(eval_int, Expression{v});
      result = CompactSet{value, 1, value};
    }
  }

  return result;
}

CompactSet CompactSetVisitor::operator()(BinOp v)
{
  Integer value = 0;
  CompactSet result;
  IsInteger is_int{_symbols};
  if (Apply(is_int, v.left())) {
    value = Apply(EvalInteger{_symbols}, v.left());
    result = ApplyThis(v.right());
  } else if (Apply(is_int, v.right())) {
    value = Apply(EvalInteger{_symbols}, v.right());
    result = ApplyThis(v.left());
  } else {
   ERROR("CompactSetVisitor: result of BinOp ", v, " is not a compact set");
  }

  switch (v.op()) {
    case Add: {
      result.translate(value);
      break;
    }

    case Sub: {
      ERROR("CompactSetVisitor: Sub not supported yet");
      break;
    }

    case Div: {
      ERROR("CompactSetVisitor: Div not supported yet");
      break;
    }

    case Mult: {
      result.scale(value);
      break;
    }

    default: {
      ERROR("CompactSetVisitor: BinOp ", v.op(), " not supported");
      break;
    }
  }

  return result;
}

CompactSet CompactSetVisitor::operator()(UnaryOp v)
{
  CompactSet result = ApplyThis(v.exp());

  switch (v.op()) {
    case Minus: {
      result.reflection();
      break;
    }

    case Plus: {
      break;
    }

    default: {
      ERROR("EvalInteger: UnaryOp ", v.op(), " in ", v, " not supported");
      break;
    }
  }

  return result;
}

}  // namespace Modelica
