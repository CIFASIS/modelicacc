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

#ifndef AST_VISITOR_ALL
#define AST_VISITOR_ALL
#include <boost/variant/static_visitor.hpp>
#include <ast/expression.h>
#include <util/table.h>

namespace Modelica {
typedef std::list<Expression> Lexp;
using namespace Modelica::AST;
class AllExpressions : public boost::static_visitor<Lexp> {
  public:
  AllExpressions(Expression);
  AllExpressions(Expression, VarSymbolTable);
  Lexp operator()(AddAll v) const;
  Lexp operator()(Integer v) const;
  Lexp operator()(Boolean v) const;
  Lexp operator()(String v) const;
  Lexp operator()(Name v) const;
  Lexp operator()(Real v) const;
  Lexp operator()(SubEnd v) const;
  Lexp operator()(SubAll v) const;
  Lexp operator()(BinOp) const;
  Lexp operator()(UnaryOp) const;
  Lexp operator()(Brace) const;
  Lexp operator()(Bracket) const;
  Lexp operator()(Call) const;
  Lexp operator()(FunctionExp) const;
  Lexp operator()(ForExp) const;
  Lexp operator()(IfExp) const;
  Lexp operator()(Named) const;
  Lexp operator()(Output) const;
  Lexp operator()(Reference) const;
  Lexp operator()(Range) const;

  private:
  Expression exp;
  mutable VarSymbolTable st;
};
}  // namespace Modelica
#endif
