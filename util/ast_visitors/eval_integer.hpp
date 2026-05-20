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

#ifndef MODELICACC_UTIL_AST_VISITORS_EVAL_INTEGER_HPP_
#define MODELICACC_UTIL_AST_VISITORS_EVAL_INTEGER_HPP_

#include <boost/variant/static_visitor.hpp>

#include <ast/expression.hpp>
#include <util/table.hpp>

namespace Modelica {

class EvalInteger : public boost::static_visitor<Integer> {
public:
  explicit EvalInteger(const VarSymbolTable&);
  EvalInteger(const VarSymbolTable&, Name, Integer);

  Integer operator()(Integer v) const;
  Integer operator()(Boolean v) const;
  Integer operator()(String v) const;
  Integer operator()(AddAll v) const;
  Integer operator()(Name v) const;
  Integer operator()(Real v) const;
  Integer operator()(SubEnd v) const;
  Integer operator()(SubAll v) const;
  Integer operator()(BinOp) const;
  Integer operator()(UnaryOp) const;
  Integer operator()(Brace) const;
  Integer operator()(Bracket) const;
  Integer operator()(Call) const;
  Integer operator()(FunctionExp) const;
  Integer operator()(ForExp) const;
  Integer operator()(IfExp) const;
  Integer operator()(Named) const;
  Integer operator()(Output) const;
  Integer operator()(Reference) const;
  Integer operator()(Range) const;

private:
  const VarSymbolTable& _vtable;
  Option<Name> _name;
  Option<Integer> _value;
};

}  // namespace Modelica

#endif // MODELICACC_UTIL_AST_VISITORS_EVAL_INTEGER_HPP_
