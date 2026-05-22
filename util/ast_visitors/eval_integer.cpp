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

#include "util/ast_visitors/eval_integer.hpp"
#include "util/ast_visitors/eval_expression.hpp"
#include "ast/modification.hpp"
#include "util/debug.hpp"

#include <boost/variant/get.hpp>

#include <cmath>
#include <limits>
#include <stdexcept>
#include <type_traits>

namespace Modelica {

EvalInteger::EvalInteger(const VarSymbolTable& vtable)
  : _vtable(vtable), _name(Option<Name>{}), _value(Option<Integer>{}) {}

EvalInteger::EvalInteger(const VarSymbolTable& vtable, Name name, Integer value)
  : _vtable(vtable), _name(name), _value(value) {}

Integer EvalInteger::operator()(Integer v) const { return v; }

Integer EvalInteger::operator()(Boolean v) const
{
  return v.val() ? 1 : 0;
}

Integer EvalInteger::operator()(AddAll v) const
{
  ERROR("EvalInteger: trying to evaluate a AddAll");
  return 0;
}

Integer EvalInteger::operator()(String v) const
{
  ERROR("EvalInteger: trying to evaluate a String");
  return 0;
}

Integer EvalInteger::operator()(Name v) const
{
  ERROR("EvalInteger: trying to evaluate a Name");
  return 0;
}

Integer EvalInteger::operator()(Real v) const
{
  if (std::isnan(v) || std::isinf(v)) {
    ERROR("EvalInteger: value ", v, " is nan or inf");
  }
  
  double truncated = std::trunc(v);
  if (std::fabs(v - truncated) > std::numeric_limits<double>::epsilon()) {
    ERROR("EvalInteger: value ", v, " is not an integer");
  }
  
  if (v < static_cast<double>(std::numeric_limits<Integer>::min()) ||
      v > static_cast<double>(std::numeric_limits<Integer>::max())) {
    ERROR("EvalInteger: value ", v, " is not an integer");
  }
  
  return static_cast<Integer>(truncated);
}

Integer EvalInteger::operator()(SubAll v) const
{
  ERROR("EvalInteger: trying to evaluate a SubAll");
  return 0;
}

Integer EvalInteger::operator()(SubEnd v) const
{
  ERROR("EvalInteger: trying to evaluate a SubEnd");
  return 0;
}

Integer EvalInteger::operator()(BinOp v) const
{
  Expression l = v.left();
  Expression r = v.right();
  switch (v.op()) {
    case Add: {
      return ApplyThis(l) + ApplyThis(r);
      break;
    }

    case Sub: {
      return ApplyThis(l) - ApplyThis(r);
      break;
    }

    case Div: {
      Integer q = ApplyThis(r);
      ERROR_UNLESS(q != 0, "EvalInteger: division by 0");
      Integer p = ApplyThis(l);
      ERROR_UNLESS(p % q == 0, "EvalInteger: division is not integer");
      return p/q;
      break;
    }

    case Mult: {
      return ApplyThis(l) * ApplyThis(r);
      break;
    }

    case Exp: {
      return pow(ApplyThis(l), ApplyThis(r));
      break;
    }

    default: {
      ERROR("EvalInteger: BinOp ", v.op(), " not supported");
      break;
    }
  }

  return 0;
}

Integer EvalInteger::operator()(UnaryOp v) const
{
  switch (v.op()) {
    case Minus: {
      return -ApplyThis(v.exp());
      break;
    }

    case Plus: {
      return ApplyThis(v.exp());
      break;
    }

    default: {
      ERROR("EvalInteger: UnaryOp ", v.op(), " in ", v, " not supported");
      break;
    }
  }

  return 0;
}

Integer EvalInteger::operator()(IfExp v) const
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

Integer EvalInteger::operator()(Range v) const
{
  ERROR("EvalInteger: trying to evaluate a Range");
  return 0;
}

Integer EvalInteger::operator()(Brace v) const
{
  ERROR("EvalInteger: trying to evaluate a Brace");
  return 0;
}

Integer EvalInteger::operator()(Bracket v) const
{
  ERROR("EvalInteger: trying to evaluate a Bracket");
  return 0;
}

Integer EvalInteger::operator()(Call v) const
{
  if ("integer" == v.name()) {
    return ApplyThis(v.args().front());
  } else if ("exp" == v.name()) {
    return exp(ApplyThis(v.args().front()));
  }

  ERROR("EvalInteger: trying to evaluate a Call");
  return 0;
}

Integer EvalInteger::operator()(FunctionExp v) const
{
  ERROR("EvalInteger: trying to evaluate a FunctionExp");
  return 0;
}

Integer EvalInteger::operator()(ForExp v) const
{
  ERROR("EvalInteger: trying to evaluate a ForExp");
  return 0;
}

Integer EvalInteger::operator()(Named v) const
{
  ERROR("EvalInteger: trying to evaluate a Named");
  return 0;
}

Integer EvalInteger::operator()(Output v) const
{
  ERROR_UNLESS(v.args().size() == 1, "EvalInteger: trying to evaluate an "
    , "Output with more than one element");
  if (v.args().front()) {
    Expression e = v.args().front().get();
    return ApplyThis(e);
  }

  ERROR("EvalInteger: Output with no expression");
  return 0;
}

Integer EvalInteger::operator()(Reference v) const
{
  Ref ref = v.ref();
  ERROR_UNLESS(ref.size() == 1, "EvalInteger: conversion of dotted references "
    , "not implemented");
  Option<ExpList> opt_subs = get<1>(ref[0]);
  ERROR_UNLESS(opt_subs.has_value(), "EvalInteger: conversion of subscripted "
    , "references not implemented");
  Name v_name = get<0>(ref[0]);

  if (_name && _name.value() == v_name) {
    return _value.value();
  }

  Option<VarInfo> v_info = _vtable[v_name];
  if (!v_info) {
    ERROR("EvalInteger: Variable ", v_name, " not found");
  }

  Option<Modification> opt_modification = v_info.value().modification();
  if (!opt_modification) {
    ERROR("EvalInteger: Variable ", v_name, " without initial value");
  }
  Modification modification = opt_modification.value();

  if (is<ModEq>(modification)) {
    return ApplyThis(get<ModEq>(modification).exp());
  }

  if (is<ModAssign>(modification)) {
    return ApplyThis(get<ModAssign>(modification).exp());
  }

  if (is<ModClass>(modification)) {
    OptExp opt_expr = get<ModClass>(modification).exp();
    if (opt_expr) {
      return ApplyThis(opt_expr.value());
    }

    for (const Argument& arg : get<ModClass>(modification).modification_ref()) {
      if (is<ElMod>(arg)) {
        ElMod element_mod = get<ElMod>(arg);
        opt_modification = element_mod.modification();
        if (element_mod.name() == "start" && opt_modification) {
          modification = opt_modification.value();
          if (is<ModEq>(modification)) {
            return ApplyThis(get<ModEq>(modification).exp());
          }
        }
      }
    }
  }

  ERROR("EvalInteger: cannot evaluate class modification ", modification);
  return 0;
}

}  // namespace Modelica
