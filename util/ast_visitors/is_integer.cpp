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

#include "util/ast_visitors/is_integer.hpp"
#include "util/ast_visitors/eval_expression.hpp"
#include "ast/modification.hpp"
#include "util/debug.hpp"

#include <boost/variant/get.hpp>

#include <cmath>
#include <limits>
#include <stdexcept>
#include <type_traits>

namespace Modelica {

IsInteger::IsInteger(const VarSymbolTable& vtable) : _vtable(vtable) {}

bool IsInteger::operator()(Integer v) const { return true; }

bool IsInteger::operator()(Boolean v) const { return true; }

bool IsInteger::operator()(AddAll v) const
{
  return false;
}

bool IsInteger::operator()(String v) const
{
  return false;
}

bool IsInteger::operator()(Name v) const
{
  return false;
}

bool IsInteger::operator()(Real v) const
{
  if (std::isnan(v) || std::isinf(v)) {
    return false;
  }
  
  double truncated = std::trunc(v);
  if (std::fabs(v - truncated) > std::numeric_limits<double>::epsilon()) {
    return false;
  }
  
  if (v < static_cast<double>(std::numeric_limits<Integer>::min()) ||
      v > static_cast<double>(std::numeric_limits<Integer>::max())) {
    return false;
  }
  
  return true;
}

bool IsInteger::operator()(SubAll v) const
{
  return false;
}

bool IsInteger::operator()(SubEnd v) const
{
  WARNING("IsInteger: SubEnd not supported yet");
  return false;
}

bool IsInteger::operator()(BinOp v) const
{
  Expression l = v.left();
  Expression r = v.right();
  switch (v.op()) {
    case Add: {
      return ApplyThis(l) && ApplyThis(r);
      break;
    }

    case Sub: {
      return ApplyThis(l) && ApplyThis(r);
      break;
    }

    case Div: {
      Integer q = ApplyThis(r);
      Integer p = ApplyThis(l);
      if (q == 0 || p % q != 0) {
        return false;
      }
      return true;
      break;
    }

    case Mult: {
      return ApplyThis(l) && ApplyThis(r);
      break;
    }

    case Exp: {
      return ApplyThis(l) && ApplyThis(r);
      break;
    }

    default: {
      ERROR("IsInteger: BinOp ", v.op(), " not supported");
      break;
    }
  }

  return false;
}

bool IsInteger::operator()(UnaryOp v) const
{
  switch (v.op()) {
    case Minus: {
      return ApplyThis(v.exp());
      break;
    }

    case Plus: {
      return ApplyThis(v.exp());
      break;
    }

    default: {
      ERROR("IsInteger: UnaryOp ", v.op(), " in ", v, " not supported");
      break;
    }
  }

  return false;
}

bool IsInteger::operator()(IfExp v) const
{
  EvalExpression eval_expr{_vtable};
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

bool IsInteger::operator()(Range v) const
{
  return false;
}

bool IsInteger::operator()(Brace v) const
{
  return false;
}

bool IsInteger::operator()(Bracket v) const
{
  return false;
}

bool IsInteger::operator()(Call v) const
{
  if ("integer" == v.name()) {
    return true;
  }

  return false;
}

bool IsInteger::operator()(FunctionExp v) const
{
  return false;
}

bool IsInteger::operator()(ForExp v) const
{
  return false;
}

bool IsInteger::operator()(Named v) const
{
  return false;
}

bool IsInteger::operator()(Output v) const
{
  ERROR_UNLESS(v.args().size() == 1, "IsInteger: trying to evaluate an "
    , "Output with more than one element");
  if (v.args().front()) {
    Expression e = v.args().front().get();
    return ApplyThis(e);
  }

  return false;
}

bool IsInteger::operator()(Reference v) const
{
  Ref ref = v.ref();
  if (ref.size() != 1) {
    WARNING("IsInteger: conversion of dotted references not implemented");
    return false;
  }
  Option<ExpList> opt_subs = get<1>(ref[0]);
  ERROR_UNLESS(opt_subs.has_value(), "IsInteger: conversion of subscripted "
    , "references not implemented");
  Name v_name = get<0>(ref[0]);

  Option<VarInfo> v_info = _vtable[v_name];
  if (!v_info) {
    ERROR("IsInteger: Variable ", v_name, " not found");
  }

  Option<Modification> opt_modification = v_info.value().modification();
  if (!opt_modification) {
    ERROR("IsInteger: Variable ", v_name, " without initial value");
  }
  Modification modification = opt_modification.value();

  if (is<ModEq>(modification)) {
    return ApplyThis(get<ModEq>(modification).exp());
  }

  if (is<ModAssign>(modification)) {
    return ApplyThis(get<ModAssign>(modification).exp());
  }

  if (is<ModClass>(modification)) {
    return false;
  }

  return false;
}

}  // namespace Modelica
