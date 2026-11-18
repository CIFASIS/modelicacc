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

#include <util/ast_visitors/contains_variable.hpp>
#include <ast/queries.hpp>
#include <boost/variant/get.hpp>

namespace Modelica {
ContainsVariable::ContainsVariable(Name v) : var(v){};

bool ContainsVariable::operator()(Integer v) const { return false; }
bool ContainsVariable::operator()(Boolean v) const { return false; }
bool ContainsVariable::operator()(String v) const { return false; }
bool ContainsVariable::operator()(AddAll v) const
{
  RefTuple rt = v.arr();
  if (get<0>(rt) == var) return true;
  foreach_(Expression e, get<1>(rt)) if (ApplyThis(e)) return true;
  return false;
}
bool ContainsVariable::operator()(Name v) const { return v == var; }
bool ContainsVariable::operator()(Real v) const { return false; }
bool ContainsVariable::operator()(SubEnd v) const { return false; }
bool ContainsVariable::operator()(SubAll v) const { return false; }
bool ContainsVariable::operator()(BinOp v) const
{
  Expression l = v.left(), r = v.right();
  return ApplyThis(l) || ApplyThis(r);
}
bool ContainsVariable::operator()(UnaryOp v) const
{
  Expression e = v.exp();
  return ApplyThis(e);
}
bool ContainsVariable::operator()(IfExp v) const
{
  Expression cond = v.cond(), then = v.then(), elseexp = v.elseexp();
  return ApplyThis(cond) || ApplyThis(then) || ApplyThis(elseexp);
}
bool ContainsVariable::operator()(Range v) const
{
  Expression start = v.start(), end = v.end();
  if (ApplyThis(start) || ApplyThis(end)) return true;
  if (v.step()) {
    Expression step = v.step().get();
    return ApplyThis(step);
  }
  return false;
}
bool ContainsVariable::operator()(Brace v) const
{
  foreach_(Expression e, v.args()) if (ApplyThis(e)) return true;
  return false;
}
bool ContainsVariable::operator()(Bracket v) const
{
  foreach_(ExpList el, v.args()) foreach_(Expression e, el) if (ApplyThis(e)) return true;
  return false;
}
bool ContainsVariable::operator()(Call v) const
{
  foreach_(Expression e, v.args()) if (ApplyThis(e)) return true;
  return false;
}
bool ContainsVariable::operator()(FunctionExp v) const
{
  foreach_(Expression e, v.args()) if (ApplyThis(e)) return true;
  return false;
}
bool ContainsVariable::operator()(ForExp v) const
{
  Expression e = v.exp();
  return ApplyThis(e);
}
bool ContainsVariable::operator()(Named v) const
{
  Expression e = v.exp();
  return ApplyThis(e);
}
bool ContainsVariable::operator()(Output v) const
{
  foreach_(OptExp oe, v.args()) if (oe && ApplyThis(oe.get())) return true;
  return false;
}
bool ContainsVariable::operator()(Reference v) const
{
  if (refName(v) == var) return true;
  // Check whether the variable is used in a subscript, e.g. a[i]
  foreach_(RefTuple rt, v.ref()) foreach_(Expression e, get<1>(rt)) if (ApplyThis(e)) return true;
  return false;
}
}  // namespace Modelica
