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

#include <util/debug.h>
#include <util/ast_visitors/contains_expression_flatter.h>
#include <boost/variant/get.hpp>

namespace Modelica {
ContainsExpressionFlatter::ContainsExpressionFlatter(Expression e) : exp(e){};

bool ContainsExpressionFlatter::operator()(Integer v) const { return exp == Expression(v); }
bool ContainsExpressionFlatter::operator()(Boolean v) const { return exp == Expression(v); }
bool ContainsExpressionFlatter::operator()(String v) const { return exp == Expression(v); }
bool ContainsExpressionFlatter::operator()(AddAll v) const
{
  if (exp == Expression(v)) return true;

  RefTuple rt = v.arr();
  Name nm = get<0>(rt);
  ExpList exps = get<1>(rt);
  ExpList::iterator itexps = exps.begin();

  for (; itexps != exps.end(); ++itexps) {
    bool ei = ApplyThis(*itexps);
    if (ei) return true;
  }

  return false;
}
bool ContainsExpressionFlatter::operator()(Name v) const { return exp == Expression(v); }
bool ContainsExpressionFlatter::operator()(Real v) const { return exp == Expression(v); }
bool ContainsExpressionFlatter::operator()(SubEnd v) const { return exp == Expression(v); }
bool ContainsExpressionFlatter::operator()(SubAll v) const { return exp == Expression(v); }
bool ContainsExpressionFlatter::operator()(BinOp v) const
{
  if (exp == Expression(v)) return true;
  Expression l = v.left(), r = v.right();
  bool rl = ApplyThis(l);
  bool ll = ApplyThis(r);
  return rl || ll;
}
bool ContainsExpressionFlatter::operator()(UnaryOp v) const
{
  if (exp == Expression(v)) return true;
  Expression e = v.exp();
  return ApplyThis(e);
}
bool ContainsExpressionFlatter::operator()(IfExp v) const
{
  if (exp == Expression(v)) return true;
  Expression cond = v.cond(), then = v.then(), elseexp = v.elseexp();
  const bool rc = ApplyThis(cond);
  const bool rt = ApplyThis(then);
  const bool re = ApplyThis(elseexp);
  return rc || rt || re;
}
bool ContainsExpressionFlatter::operator()(Range v) const
{
  if (exp == Expression(v)) return true;
  Expression start = v.start(), end = v.end();
  bool rs = ApplyThis(start);
  bool re = ApplyThis(end);
  return rs || re;
}
bool ContainsExpressionFlatter::operator()(Brace v) const
{
  if (exp == Expression(v)) return true;
  ERROR("ContainsExpressionFlatter: Brace expression not supported");
  // TODO
  return false;
}
bool ContainsExpressionFlatter::operator()(Bracket v) const
{
  if (exp == Expression(v)) return true;
  ERROR("ContainsExpressionFlatter: Bracket expression not supported");
  // TODO
  return false;
}
bool ContainsExpressionFlatter::operator()(Call v) const
{
  if (exp == Expression(v)) return true;
  foreach_(Expression e, v.args()) if (ApplyThis(e)) return true;
  return false;
}
bool ContainsExpressionFlatter::operator()(FunctionExp v) const
{
  if (exp == Expression(v)) return true;
  // TODO
  return false;
}
bool ContainsExpressionFlatter::operator()(ForExp v) const
{
  if (exp == Expression(v)) return true;
  ERROR("ContainsExpressionFlatter: For expression not supported");
  // TODO
  return false;
}
bool ContainsExpressionFlatter::operator()(Named v) const
{
  if (exp == Expression(v)) return true;
  // TODO
  return false;
}
bool ContainsExpressionFlatter::operator()(Output v) const
{
  if (exp == Expression(v)) return true;
  foreach_(OptExp oe, v.args()) if (oe && ApplyThis(oe.get())) return true;
  return false;
}
bool ContainsExpressionFlatter::operator()(Reference v) const
{
  if (exp == Expression(v)) return true;
  if (is<Reference>(exp)) {
    Reference ref = get<Reference>(exp);
    // Check whether a[1] is contained in a
    if (get<0>(v.ref().front()) == get<0>(ref.ref().front()) && get<1>(v.ref().front()).size() == 0) return true;

    // Check variable subscripts
    foreach_ (RefTuple r, v.ref()) {
      foreach_ (Expression e, get<1>(r))
        if (ApplyThis(e))
          return true;
      }
    }
  }
  return false;
}
}  // namespace Modelica
