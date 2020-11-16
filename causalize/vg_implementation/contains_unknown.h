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

/*
 * contains_unknown.h
 *
 *  Created on: 17 oct. 2016
 *      Author: D.Hollmann
 */


#ifndef AST_VISITOR_CONTAINS_UNKNOWN
#define AST_VISITOR_CONTAINS_UNKNOWN
#include <boost/variant/static_visitor.hpp>
#include <causalize/graph/graph_definition.h>
#include <flatter/mmo_graph.h>
#include <ast/expression.h>
#include <util/table.h>
#include <set>


namespace Causalize {

  using namespace Modelica;
  using namespace Modelica::AST;

  class ContainsUnknown: public boost::static_visitor<bool> {
  public:
    ContainsUnknown(std::vector<Expression>, const VarSymbolTable&);
    bool operator()(Modelica::AST::Integer v) const;
    bool operator()(Boolean v) const;
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

    std::set<int> getUsages() const;
    void clear();
  private:
    int IsUsed(Expression) const;
    mutable VarSymbolTable syms;
    std::map<std::string,int> definedUnks;
    mutable std::set<int> usedUnks;
  };
}
#endif
