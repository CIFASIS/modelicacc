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

#include "util/sbg/ast_visitors/access_to_sbg_expr.hpp"
#include "ast/ast_types.hpp"
#include "util/debug.hpp"
#include "util/sbg/ast_visitors/sbg_expr_visitor.hpp"

#include <algorithm>

namespace Modelica {

AccessToSBGExpr::AccessToSBGExpr(
  const VarSymbolTable& symbols, const Counters& counters
) : _max_dim(0), _symbols(symbols), _counters(counters)
{
  for (const auto& [name, var_info] : symbols) {
    Option<AST::ExpList> opt_indices = var_info.indices();
    if (opt_indices) {
      _max_dim = std::max<unsigned int>(_max_dim, (*opt_indices).size());
    }
  }
}

SBG::LIB::Expression AccessToSBGExpr::operator()(AST::Integer v) const
{
  ERROR("AccessToSBGExpr: Integer is not an ");
  return SBG::LIB::Expression{};
}

SBG::LIB::Expression AccessToSBGExpr::operator()(AST::Boolean v) const
{
  ERROR("AccessToSBGExpr: Boolean is not an access to a variable");
  return SBG::LIB::Expression{};
}

SBG::LIB::Expression AccessToSBGExpr::operator()(AST::AddAll v) const 
{
  ERROR("AccessToSBGExpr: AddAll is not an access to a variable");
  return SBG::LIB::Expression{};
}

SBG::LIB::Expression AccessToSBGExpr::operator()(AST::String v) const 
{
  ERROR("AccessToSBGExpr: String is not an access to a variable");
  return SBG::LIB::Expression{};
}

SBG::LIB::Expression AccessToSBGExpr::operator()(AST::Name v) const 
{
  ERROR("AccessToSBGExpr: Name is not an access to a variable");
  return SBG::LIB::Expression{};
}

SBG::LIB::Expression AccessToSBGExpr::operator()(AST::Real v) const 
{
  ERROR("AccessToSBGExpr: Real is not an access to a variable");
  return SBG::LIB::Expression{};
}

SBG::LIB::Expression AccessToSBGExpr::operator()(AST::SubEnd v) const
{
  ERROR("AccessToSBGExpr: SubEnd is not an access to a variable");
  return SBG::LIB::Expression{};
}

SBG::LIB::Expression AccessToSBGExpr::operator()(AST::SubAll v) const
{
  ERROR("AccessToSBGExpr: SubAll is not an access to a variable");
  return SBG::LIB::Expression{};
}

SBG::LIB::Expression AccessToSBGExpr::operator()(AST::IfExp v) const
{
  ERROR("AccessToSBGExpr: IfExp is not an access to a variable");
  return SBG::LIB::Expression{};
}

SBG::LIB::Expression AccessToSBGExpr::operator()(AST::Range v) const
{
  ERROR("AccessToSBGExpr: Range is not an access to a variable");
  return SBG::LIB::Expression{};
}

SBG::LIB::Expression AccessToSBGExpr::operator()(AST::Brace v) const
{
  ERROR("AccessToSBGExpr: Brace is not an access to a variable");
  return SBG::LIB::Expression{};
}

SBG::LIB::Expression AccessToSBGExpr::operator()(AST::Bracket v) const
{
  ERROR("AccessToSBGExpr: Bracket is not an access to a variable");
  return SBG::LIB::Expression{};
}

SBG::LIB::Expression AccessToSBGExpr::operator()(AST::Call v) const
{
  ERROR_UNLESS(v.name() == "der", "AccessToSBGExpr/Call: ", v
    , " is not a variable or a derivative");
  ExpList args = v.args();
  ERROR_UNLESS(args.size() == 1, "AccessToSBGExpr/Call: der applied to more "
    , " than one argument in ", args);
  return ApplyThis(args.front());
}

SBG::LIB::Expression AccessToSBGExpr::operator()(AST::FunctionExp v) const
{
  ERROR("AccessToSBGExpr: FunctionExp is not an access to a variable");
  return SBG::LIB::Expression{};
}

SBG::LIB::Expression AccessToSBGExpr::operator()(AST::ForExp v) const
{
  ERROR("AccessToSBGExpr: ForExp is not an access to a variable");
  return SBG::LIB::Expression{};
}

SBG::LIB::Expression AccessToSBGExpr::operator()(AST::Named v) const
{
  ERROR("AccessToSBGExpr: Named is not an access to a variable");
  return SBG::LIB::Expression{};
}

SBG::LIB::Expression AccessToSBGExpr::operator()(AST::Output v) const
{
  ERROR("AccessToSBGExpr: Output is not an access to a variable");
  return SBG::LIB::Expression{};
}

SBG::LIB::Expression AccessToSBGExpr::operator()(AST::Reference v) const
{
  // Get expression of subscripts.
  AST::Ref ref = v.ref();
  assert(ref.size() > 0);
  ERROR_UNLESS(ref.size() == 1, "AccessToSBGExpr/Reference: conversion of "
    , "dotted references not implemented");
  AST::ExpList indexes = get<1>(ref.front());

  SBG::LIB::Rational zero{0};
  SBG::LIB::Expression sbg_expr;
  if (indexes.empty()) { // Access to scalar variable.
    for (std::size_t k = 0; k < _max_dim; ++k) {
      sbg_expr = sbg_expr.cartesianProduct(SBG::LIB::Expression{zero, zero});
    }
  } else { // Access to array variable.
    ExprVisitor expr_visitor{_symbols, _counters};
    for (Expression index : indexes) {
      sbg_expr = sbg_expr.cartesianProduct(Apply(expr_visitor, index));
    }
  }

  return sbg_expr; 
}

SBG::LIB::Expression AccessToSBGExpr::operator()(AST::BinOp v) const
{
  ERROR("AccessToSBGExpr: BinOp is not an access to a variable");
  return SBG::LIB::Expression{};
}

SBG::LIB::Expression AccessToSBGExpr::operator()(AST::UnaryOp v) const
{
  ERROR("AccessToSBGExpr: UnaryOp is not an access to a variable");
  return SBG::LIB::Expression{};
}

}  // namespace Modelica
