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
#include <boost/variant/get.hpp>

#include <util/ast_visitors/matching_exps.h>

using namespace std;

namespace Modelica {
MatchingExps::MatchingExps(string variable_name, bool state_var) : _matched_exps(), _variable_name(variable_name), _state_var(state_var), _der_call(false), _has_der(false) {};

bool MatchingExps::operator()(Integer v) const { return false; }

bool MatchingExps::operator()(Boolean v) const { return false; }

bool MatchingExps::operator()(String v) const { return false; }

bool MatchingExps::operator()(AddAll v) const { return false; }

bool MatchingExps::operator()(Name v) const { return false; }

bool MatchingExps::operator()(Real v) const { return false; }

bool MatchingExps::operator()(SubEnd v) const { return false; }

bool MatchingExps::operator()(SubAll v) const { return false; }

bool MatchingExps::operator()(BinOp v) const
{
  Expression l = v.left(), r = v.right();
  bool rl = ApplyThis(l);
  bool ll = ApplyThis(r);
  return rl || ll;
}

bool MatchingExps::operator()(UnaryOp v) const
{
  Expression e = v.exp();
  return ApplyThis(e);
}

bool MatchingExps::operator()(IfExp v) const
{
  Expression cond = v.cond(), then = v.then(), elseexp = v.elseexp();
  const bool rc = ApplyThis(cond);
  const bool rt = ApplyThis(then);
  const bool re = ApplyThis(elseexp);
  return rc || rt || re;
}

bool MatchingExps::operator()(Range v) const
{
  Expression start = v.start(), end = v.end();
  bool rs = ApplyThis(start);
  bool re = ApplyThis(end);
  return rs || re;
}

bool MatchingExps::operator()(Brace v) const
{ 
  bool matched = false;
  foreach_(Expression e, v.args()) { matched = matched || ApplyThis(e); }
  return matched;
}

bool MatchingExps::operator()(Bracket v) const { return false; }

bool MatchingExps::operator()(Call v) const
{
  if (v.name() == "der") {
    _der_call = true;
    _has_der = true;
  }
  bool matched = false;
  foreach_(Expression e, v.args()) { matched = matched || ApplyThis(e); }
  if (v.name() == "der") {
    _der_call = false;
  }
  return matched;
}

bool MatchingExps::operator()(FunctionExp v) const { return false; }

bool MatchingExps::operator()(ForExp v) const { return false; }

bool MatchingExps::operator()(Named v) const { return false; }

bool MatchingExps::operator()(Output v) const
{
  bool matched = false;
  foreach_(OptExp out_exp, v.args()) {
    if (out_exp) {
      matched = matched || ApplyThis(out_exp.get());
    }
  }
  return matched; 
}

bool MatchingExps::operator()(Reference v) const
{
  if (get<0>(v.ref().front()) == _variable_name) {
    if ((_state_var && _der_call) || !_state_var) {
      _matched_exps.insert(v);
      return true;      
    }
  } 
  return false;
}

set<Expression> MatchingExps::matchedExps() const
{
  if (!_state_var && _has_der) {
    return set<Expression>();
  } 
  return _matched_exps; 
}

}  // namespace Modelica
