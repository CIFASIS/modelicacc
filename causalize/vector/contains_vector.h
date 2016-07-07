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

#ifndef AST_VISITOR_CONTAINS_VECTOR
#define AST_VISITOR_CONTAINS_VECTOR
#include <boost/variant/static_visitor.hpp>
#include <causalize/vector/vector_graph_definition.h>
#include <ast/expression.h>
#include <util/ast_visitors/contains.h>
#include <boost/icl/discrete_interval.hpp>
#include <set>
#define apply(X) boost::apply_visitor(*this,X)

namespace Causalize {

  using namespace Modelica::AST;
  class ContainsVector: public boost::static_visitor<bool> {
  public:
    ContainsVector(Expression, VectorVertexProperty, const VarSymbolTable &);
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
    IndexPairSet getOccurrenceIndexes() { return labels; }
    void setForIndex(Expression a, Expression b);
  private:
    void addGenericIndex(BinOp b) const;
    Expression exp;
    boost::icl::discrete_interval<int> forIndexInterval;
    mutable std::set<VectorEdgeProperty> edgeList;
    mutable IndexPairSet labels;
    VectorVertexProperty var;
    const VarSymbolTable &syms;
    bool foreq;
  }; 
}
#endif 
