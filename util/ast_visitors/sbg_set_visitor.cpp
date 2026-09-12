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

#include "util/ast_visitors/sbg_set_visitor.hpp"
#include "ast/queries.hpp"
#include "util/ast_visitors/contains_expression.hpp"
#include "util/ast_visitors/eval_expression.hpp"
#include "util/ast_visitors/eval_integer.hpp"
#include "util/ast_visitors/is_integer.hpp"
#include "util/debug.hpp"

namespace Modelica {

SBGSetVisitor::SBGSetVisitor(const VarSymbolTable& symbols)
  : _symbols(symbols) {}

SBG::LIB::Set SBGSetVisitor::operator()(Integer v)
{
  return SBG::LIB::Set{v, 1, v};
}

SBG::LIB::Set SBGSetVisitor::operator()(Boolean v)
{
  if (v.val()) {
    return SBG::LIB::Set{1, 1, 1};
  }
  return SBG::LIB::Set{0, 1, 0};
}

SBG::LIB::Set SBGSetVisitor::operator()(AddAll v) 
{
  ERROR("SBGSetVisitor: trying to visit an AddAll ", v);
  return SBG::LIB::Set{};
}

SBG::LIB::Set SBGSetVisitor::operator()(String v) 
{
  ERROR("SBGSetVisitor: trying to visit a String ", v);
  return SBG::LIB::Set{};
}

SBG::LIB::Set SBGSetVisitor::operator()(Name v) 
{
  ERROR("SBGSetVisitor: trying to visit a Name ", v);
  return SBG::LIB::Set{};
}

SBG::LIB::Set SBGSetVisitor::operator()(Real v) 
{
  EvalInteger eval_int{_symbols};
  Integer value = Apply(eval_int, Expression{v});
  return SBG::LIB::Set{value, 1, value};
}

SBG::LIB::Set SBGSetVisitor::operator()(SubEnd v)
{
  ERROR("SBGSetVisitor: SubEnd not supported");
  return SBG::LIB::Set{};
}

SBG::LIB::Set SBGSetVisitor::operator()(SubAll v)
{
  return SBG::LIB::Set{1, 1, _dimension_size};
}

SBG::LIB::Set SBGSetVisitor::operator()(IfExp v)
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

SBG::LIB::Set SBGSetVisitor::operator()(Range v)
{
  EvalInteger eval_int(_symbols);
  Integer start = Integer(Apply(eval_int, v.start()));
  Integer step = 1;
  Integer end = Integer(Apply(eval_int, v.end()));
  if (v.step()) {
    step = Integer(Apply(eval_int, v.step().get()));
  }

  if (start > end) { // Decreasing range
    ERROR("SBGSetVisitor: only increasing Range expressions supported");
  }

  return SBG::LIB::Set{start, step, end};
}

SBG::LIB::Set SBGSetVisitor::operator()(Brace v)
{
  SBG::LIB::Set result;

  for (const Expression& expr : v.args()) {
    result = result.disjointCup(ApplyThis(expr));
  }

  return result;
}

SBG::LIB::Set SBGSetVisitor::operator()(Bracket v)
{
  SBG::LIB::Set result;

  for (const ExpList& expr_list : v.args()) {
    SBG::LIB::Set jth_set;
    for (const Expression& expr : expr_list) {
      jth_set = jth_set.disjointCup(ApplyThis(expr));
    }
    result.cartesianProduct(jth_set);
  }

  return result;
}

SBG::LIB::Set SBGSetVisitor::operator()(Call v)
{
  EvalInteger eval_int{_symbols};
  Integer value = Apply(eval_int, Expression{v});
  return SBG::LIB::Set{value, 1, value};
}

SBG::LIB::Set SBGSetVisitor::operator()(FunctionExp v)
{
  ERROR("SBGSetVisitor: FunctionExp ", v, " inside loop indices");
  return SBG::LIB::Set{};
}

SBG::LIB::Set SBGSetVisitor::operator()(ForExp v)
{
  ERROR("SBGSetVisitor: ForExp not supported yet");
  return SBG::LIB::Set{};
}

SBG::LIB::Set SBGSetVisitor::operator()(Named v)
{
  ERROR("SBGSetVisitor: Named not supported");
  return SBG::LIB::Set{};
}

SBG::LIB::Set SBGSetVisitor::operator()(Output v)
{
  ERROR_UNLESS(v.args().size() == 1
    , "SBGSetVisitor: Output expressions with more than one element not "
    , "supported");

  if (v.args().front()) {
    return ApplyThis(v.args().front().value());
  }

  ERROR("SBGSetVisitor: Output with no expression");
  return SBG::LIB::Set{};
}

SBG::LIB::Set SBGSetVisitor::operator()(Reference v)
{
  SBG::LIB::Set result;

  Ref ref = v.ref();
  ERROR_UNLESS(ref.size() == 1, "SBGSetVisitor: conversion of dotted "
    , "references not implemented");
  Option<ExpList> opt_subs = get<1>(ref[0]);
  ERROR_UNLESS(opt_subs.has_value() ? opt_subs.value().empty() : true
    , "SBGSetVisitor: conversion of subscripted references not implemented");

  if (!opt_subs) { // Non-subscripted access
    Name v_name = get<0>(ref[0]);
    ERROR_UNLESS(_symbols[v_name].has_value(), "SBGSetVisitor: variable "
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
      result = SBG::LIB::Set{value, 1, value};
    }
  }

  return result;
}

SBG::LIB::Set SBGSetVisitor::operator()(BinOp v)
{
  Integer value = 0;
  SBG::LIB::Set result;
  IsInteger is_int{_symbols};
  if (Apply(is_int, v.left())) {
    value = Apply(EvalInteger{_symbols}, v.left());
    result = ApplyThis(v.right());
  } else if (Apply(is_int, v.right())) {
    value = Apply(EvalInteger{_symbols}, v.right());
    result = ApplyThis(v.left());
  } else {
   ERROR("SBGSetVisitor: result of BinOp ", v, " is not a compact set");
  }

  switch (v.op()) {
    case Add: {
      result.translate(value);
      break;
    }

    case Sub: {
      ERROR("SBGSetVisitor: Sub not supported yet");
      break;
    }

    case Div: {
      ERROR("SBGSetVisitor: Div not supported yet");
      break;
    }

    case Mult: {
      ERROR("SBGSetVisitor: Mult not supported yet");
      break;
    }

    default: {
      ERROR("SBGSetVisitor: BinOp ", v.op(), " not supported");
      break;
    }
  }

  return result;
}

SBG::LIB::Set SBGSetVisitor::operator()(UnaryOp v)
{
  SBG::LIB::Set result = ApplyThis(v.exp());

  switch (v.op()) {
    case Minus: {
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
