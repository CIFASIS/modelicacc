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

#include <ast/queries.h>
#include <util/ast_visitors/eval_expression.h>
#include <util/ast_visitors/pwl_map_values.h>
#include <util/debug.h>

namespace Modelica {

PWLMapValues::PWLMapValues(VarSymbolTable symbols) : _constant(0), _slope(1), _symbols(symbols){};

bool PWLMapValues::operator()(Integer v) const { return false; }

bool PWLMapValues::operator()(Boolean v) const { return false; }

bool PWLMapValues::operator()(AddAll v) const { return false; }

bool PWLMapValues::operator()(String v) const { return false; }

bool PWLMapValues::operator()(Name v) const { return false; }

bool PWLMapValues::operator()(Real v) const { return false; }

bool PWLMapValues::operator()(SubEnd v) const { return false; }

bool PWLMapValues::operator()(SubAll v) const { return false; }

bool PWLMapValues::operator()(IfExp v) const { return false; }

bool PWLMapValues::operator()(Range v) const { return false; }

bool PWLMapValues::operator()(Brace v) const { return false; }

bool PWLMapValues::operator()(Bracket v) const { return false; }

bool PWLMapValues::operator()(Call v) const { return false; }

bool PWLMapValues::operator()(FunctionExp v) const { return false; }

bool PWLMapValues::operator()(ForExp v) const { return false; }

bool PWLMapValues::operator()(Named v) const { return false; }

bool PWLMapValues::operator()(Output v) const { return false; }

bool PWLMapValues::operator()(Reference v) const
{
  std::string var_name = get<0>(v.ref().front());
  if (isConstantNoCheck(var_name, _symbols)) {
    EvalExpression eval_exp(_symbols);
    _constant = Apply(eval_exp, (Expression)v);
    _slope = 0;
    return false;
  }
  return true;
}

void PWLMapValues::assign(Expression left, Expression right, bool var_left, bool var_right, int sign) const
{
  EvalExpression eval_exp(_symbols);
  if (var_left) {
    _constant = Apply(eval_exp, right);
  } else if (var_right) {
    _constant = Apply(eval_exp, left);
  } else {
    _constant = Apply(eval_exp, left) + sign * Apply(eval_exp, right);
  }
}

bool PWLMapValues::operator()(BinOp v) const
{
  EvalExpression eval_exp(_symbols);
  Expression l = v.left(), r = v.right();
  bool var_left = ApplyThis(l);
  bool var_right = ApplyThis(r);
  BinOpType type = v.op();
  if (var_left && var_right) {
    ERROR("Wrong map value!");
    return false;
  }
  if (type == BinOpType::Mult) {
    if (var_left) {
      _slope = Apply(eval_exp, r);
    } else if (var_right) {
      _slope = Apply(eval_exp, l);
    } else {
      _constant = Apply(eval_exp, l) * Apply(eval_exp, r);
    }
  }
  if (type == BinOpType::Add) {
    assign(l, r, var_left, var_right, 1);
  }
  if (type == BinOpType::Sub) {
    assign(l, r, var_left, var_right, -1);
  }
  return var_left && var_right;
}

bool PWLMapValues::operator()(UnaryOp v) const
{
  Expression e = v.exp();
  bool ret = ApplyThis(e);
  if (ret) {
    _slope *= -1;
  } else {
    _constant *= -1;
  }
  return ret;
}

int PWLMapValues::constant() const  { return _constant; }

int PWLMapValues::slope() const  { return _slope; }

}  // namespace Modelica
