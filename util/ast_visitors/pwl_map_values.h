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

#include <boost/variant/static_visitor.hpp>
#include <ast/expression.h>

#ifndef AST_VISITOR_PWL_MAP_VALUES
#define AST_VISITOR_PWL_MAP_VALUES

namespace Modelica {

using namespace Modelica::AST;
class PWLMapValues : public boost::static_visitor<bool> {
  public:
  PWLMapValues(VarSymbolTable symbols);
  bool operator()(Integer v) const;
  bool operator()(Boolean v) const;
  bool operator()(AddAll v) const;
  bool operator()(String v) const;
  bool operator()(Name v) const;
  bool operator()(Real v) const;
  bool operator()(SubEnd v) const;
  bool operator()(SubAll v) const;
  bool operator()(BinOp) const;
  bool operator()(UnaryOp) const;
  bool operator()(Brace) const;
  bool operator()(Bracket) const;
  bool operator()(Call) const;
  bool operator()(FunctionExp) const;
  bool operator()(ForExp) const;
  bool operator()(IfExp) const;
  bool operator()(Named) const;
  bool operator()(Output) const;
  bool operator()(Reference) const;
  bool operator()(Range) const;

  int constant() const;
  int slope() const;

  protected:
  void assign(Expression left, Expression right, bool var_left, bool var_right, int sign) const;

  mutable int _constant;
  mutable int _slope;
  VarSymbolTable _symbols;

};

}  // namespace Modelica
#endif
