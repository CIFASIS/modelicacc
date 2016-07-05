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
    along with Modelica C Compiler.  If not, see <http:  www.gnu.org/licenses/>.

******************************************************************************/

#include <causalize/vector/contains_vector.h>
#include <util/debug.h>
#include <boost/variant/get.hpp>
#include <boost/icl/discrete_interval.hpp>
#include <causalize/vector/vector_graph_definition.h>
#include <util/ast_visitors/evalexp.h>
#include <boost/variant/apply_visitor.hpp>
#include <util/ast_visitors/part_evalexp.h>

using namespace boost;
using namespace boost::icl;
using namespace Modelica::AST ;

namespace Causalize {
    contains_vector::contains_vector(Expression e, VectorVertexProperties v, const VarSymbolTable &s): exp(e), var(v), syms(s), foreq(false) {};
    bool contains_vector::operator()(Modelica::AST::Integer v) const { return exp==Expression(v); } 
    bool contains_vector::operator()(Boolean v) const { return exp==Expression(v); } 
    bool contains_vector::operator()(String v) const { return exp==Expression(v); } 
    bool contains_vector::operator()(Name v) const { return exp==Expression(v); }
    bool contains_vector::operator()(Real v) const { return exp==Expression(v); } 
    bool contains_vector::operator()(SubEnd v) const { return exp==Expression(v); } 
    bool contains_vector::operator()(SubAll v) const { return exp==Expression(v); } 
    bool contains_vector::operator()(BinOp v) const { 
      if (exp==Expression(v)) return true; 
      Expression l=v.left(), r=v.right();
      bool rl = apply(l);
      bool rr = apply(r);
      return rr || rl;
      } 
      bool contains_vector::operator()(UnaryOp v) const { 
        if (exp==Expression(v)) return true; 
        Expression e=v.exp();
        return apply(e);
      } 
      bool contains_vector::operator()(IfExp v) const { 
        if (exp==Expression(v)) return true; 
        Expression cond=v.cond(), then=v.then(), elseexp=v.elseexp();
        const bool rc = apply(cond);
        const bool rt = apply(then);
        const bool re = apply(elseexp);
        return rc || rt || re;
      }
      bool contains_vector::operator()(Range v) const { 
        if (exp==Expression(v)) return true; 
        Expression start=v.start(), end=v.end();
        bool rs = apply(start);
        bool re = apply(end);
        return rs || re;
      }
      bool contains_vector::operator()(Brace v) const { 
        if (exp==Expression(v)) return true; 
        return false;
      }
      bool contains_vector::operator()(Bracket v) const { 
        if (exp==Expression(v)) return true; 
        return false;
      }
    bool contains_vector::operator()(Call v) const { 
      if (is<Call>(exp)) {
        Call call=get<Call>(exp);
        if (v.name()=="der" && call.name()=="der") {
          ERROR_UNLESS(is<Reference>(v.args().front()) && is<Reference>(call.args().front()), "Arguments to call must be a reference");
          Reference vv=get<Reference>(v.args().front());
          Reference r=get<Reference>(call.args().front());
          if (get<0>(vv.ref().front())==get<0>(r.ref().front())) {
            //TODO: 
            if (/*var.count==*/0) {
              VectorEdgeProperties newEdge;
              if (!foreq)
                newEdge.p_e +=discrete_interval<int>::closed(1, 1);
              else
                newEdge.p_e += forIndexInterval;
              newEdge.p_v += discrete_interval<int>::closed(1, 1);
              edgeList.insert(newEdge);
              return true;
            } else {
              ExpList el = get<1>(vv.ref().front());
              if (el.size()==0) { // If there are no sub-indices the complete array is used
                ERROR("Derivative of array not suported");
              }
              Expression i = el.front();
              Modelica::PartEvalExp pe(syms);
              Expression ind = boost::apply_visitor(pe,i);
              if (is<Modelica::AST::Integer>(ind)) {
                VectorEdgeProperties newEdge;
                newEdge.p_v += discrete_interval<int>::closed(get<Modelica::AST::Integer>(ind), get<Modelica::AST::Integer>(ind));
                if (!foreq)
                  newEdge.p_e +=discrete_interval<int>::closed(1, 1);
                else
                  newEdge.p_e += forIndexInterval;
                edgeList.insert(newEdge);
                return true;
              } else if (is<Reference>(ind)) {
                ERROR_UNLESS(foreq, "Generic index used outside for equation");
                VectorEdgeProperties newEdge;
                if (!foreq)
                  newEdge.p_e +=discrete_interval<int>::closed(1, 1);
                else
                  newEdge.p_e += 
                newEdge.p_v += forIndexInterval;
                edgeList.insert(newEdge);
                return true;
              } else if (is<BinOp>(ind)) {
                 ERROR("BinOp Indexed expression not supported yet");
                return true;
              } else ERROR("Indexed expression not supported yet");
            }
          } 
          return false;
        }
      }
      if (exp==Expression(v)) return true; 
      return false;
    }
      bool contains_vector::operator()(FunctionExp v) const { 
        if (exp==Expression(v)) return true; 
        return false;
      }
      bool contains_vector::operator()(ForExp v) const {
        if (exp==Expression(v)) return true; 
        return false;
      }
      bool contains_vector::operator()(Named v) const {
        if (exp==Expression(v)) return true; 
        return false;
      }
      bool contains_vector::operator()(Output v) const {
        if (exp==Expression(v)) return true; 
        foreach_(OptExp oe, v.args()) 
          if (oe && apply(oe.get())) return true;
        return false;
      }
    bool contains_vector::operator()(Reference v) const {
      if (is<Reference>(exp)) {
        if (get<0>(v.ref().front())==get<0>(get<Reference>(exp).ref().front())) {
          //std::cerr << "Checking " << v << " againt " << exp << " count= " << var.count << "\n";
          //TODO: 
          if (/*var.count==*/0) {
            // The variabl is a scalar
            VectorEdgeProperties newEdge;
            if (!foreq)
              newEdge.p_e +=discrete_interval<int>::closed(1, 1);
            else
              newEdge.p_e += forIndexInterval;
            newEdge.p_v += discrete_interval<int>::closed(1, 1);
            edgeList.insert(newEdge);
            return true;
          } else {
            ExpList el = get<1>(v.ref().front());
            if (el.size()==0) { // If there are no sub-indices the complete array is used
              VectorEdgeProperties newEdge;
              //TODO: 
              newEdge.p_v += discrete_interval<int>::closed(1, /*var.count*/0);
              if (!foreq)
                newEdge.p_e +=discrete_interval<int>::closed(1, 1);
              else
                newEdge.p_e += forIndexInterval;
              edgeList.insert(newEdge);
              return true;
            } 
            Expression i = el.front();
            Modelica::PartEvalExp pe(syms);
            Expression ind = boost::apply_visitor(pe,i);
            if (is<Modelica::AST::Integer>(ind)) {
              VectorEdgeProperties newEdge;
              newEdge.p_v += discrete_interval<int>::closed(get<Modelica::AST::Integer>(ind),get<Modelica::AST::Integer>(ind));
              if (!foreq)
                newEdge.p_e +=discrete_interval<int>::closed(1, 1);
              else
                newEdge.p_e += forIndexInterval;
              edgeList.insert(newEdge);
              return true;
            } else if (is<Reference>(ind)) {
              ERROR_UNLESS(foreq, "Generic index used outside for equation");
              VectorEdgeProperties newEdge;
              newEdge.p_v += forIndexInterval;
              newEdge.p_e += forIndexInterval;
              edgeList.insert(newEdge);
              return true;
            } else if (is<BinOp>(ind)) {
              BinOp bop = get<BinOp>(ind);
              if (is<Output>(bop.right())) {
                ERROR_UNLESS(get<Output>(bop.right()).args().size()==1, "BinOp index expression not supported yet");
                Expression plus = get<Output>(bop.right()).args().front().get();
                ERROR_UNLESS(is<Modelica::AST::Integer>(plus), "BinOp index expression not supported yet");
                int val = get<Modelica::AST::Integer>(plus);
                if (val <0 && bop.op() == Sub) {
                  bop.op_ref()=Add;
                  bop.right_ref()=-val;
                }
              }
              ERROR_UNLESS(is<Modelica::AST::Integer>(bop.right()), "BinOp index expression not supported yet");
              int offset = get<Modelica::AST::Integer>(bop.right());
              if (bop.op()==Sub) {
                bop = BinOp(bop.left(),Add, -offset);
                offset*=-1;
              }
              ERROR_UNLESS(bop.op()==Add, "BinOp index expression not supported yet");
              VectorEdgeProperties newEdge;
              newEdge.p_e += forIndexInterval;
              int l = forIndexInterval.lower();
              int u = forIndexInterval.upper();
              newEdge.p_v += discrete_interval<int>::closed(l+offset, u+offset);
              edgeList.insert(newEdge);
              return true;
            } else {
              std::cerr << ind << "\n";
              ERROR("Indexed expression not supported yet");
            }
          }
        }
      }
      return false;
    }

    void contains_vector::setForIndex(Expression a, Expression b) {
       Modelica::EvalExp ev(syms); 
       int start=boost::apply_visitor(ev,a);
       int end=boost::apply_visitor(ev,b);
       forIndexInterval =  discrete_interval<int>::closed(start,end);
       foreq=true;
    }

    void contains_vector::addGenericIndex(BinOp binop) const {
      ERROR("addGenericIndex not implemented");
      /*
      int a=0,b=0;
      EvalExp ev(syms); 
      if (binop.op()==Add || binop.op()==Sub) {
        if (is<Reference>(binop.left()) && is<Modelica::AST::Integer>(binop.right())) {
          a  = 1;
          b  = boost::apply_visitor(ev,binop.right_ref());
          b *= (binop.op()==Add ? 1 : -1);
        }
        if (is<Reference>(binop.right()) && is<Modelica::AST::Integer>(binop.left())) {
          a  = 1;
          b  = boost::apply_visitor(ev,binop.left_ref());
          b *= (binop.op()==Add ? 1 : -1);
        }
 
      }
      EdgeProperties newEdge;
      newEdge.genericIndex = std::pair<int,int>(a,b);
      newEdge.indexInterval.add(forIndexInterval);
      edgeList.insert(newEdge);
      */


    }
}
