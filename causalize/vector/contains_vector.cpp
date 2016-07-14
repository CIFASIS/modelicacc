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
using namespace Modelica;
using namespace Modelica::AST ;

namespace Causalize {
  ContainsVector::ContainsVector(Expression e, VectorVertexProperty v, const VarSymbolTable &s):
    exp(e),
		unk2find(v),
		syms(s),
		foreq(false),
		indexes() {};
  /**
   * Builds a ContainsVector class to find occurrences of the Expression 'e'
   * using the table of symbols 's' and index list 'indexes' in 'for-equations'.
   */
  ContainsVector::ContainsVector(VectorVertexProperty unk, VarSymbolTable &s, IndexList indexes):
    exp(unk.unknowns.front()),
    unk2find(unk),
    syms(s),
    foreq(true),
    indexes(indexes) {
      ERROR_UNLESS(indexes.size()==1, "For Loop with more than one index is not supported yet\n");
      ERROR_UNLESS(indexes.front().exp(), "No index in for equation");
      ERROR_UNLESS(is<Range>(indexes.front().exp().get()), "Only range expressions supported");
      Range range = get<Range>(indexes.front().exp().get());
      EvalExp ev(syms);
      forIndexInterval =  discrete_interval<int>::closed(boost::apply_visitor(ev,range.start_ref()), boost::apply_visitor(ev,range.end_ref()));
  };
  bool ContainsVector::operator()(Modelica::AST::Integer v) const { return exp==Expression(v); }
  bool ContainsVector::operator()(Boolean v) const { return exp==Expression(v); }
  bool ContainsVector::operator()(String v) const { return exp==Expression(v); }
  bool ContainsVector::operator()(Name v) const { return exp==Expression(v); }
  bool ContainsVector::operator()(Real v) const { return exp==Expression(v); }
  bool ContainsVector::operator()(SubEnd v) const { return exp==Expression(v); }
  bool ContainsVector::operator()(SubAll v) const { return exp==Expression(v); }
  bool ContainsVector::operator()(BinOp v) const {
    if (exp==Expression(v)) return true;
    Expression l=v.left(), r=v.right();
    bool rl = apply(l);
    bool rr = apply(r);
    return rr || rl;
  }
  bool ContainsVector::operator()(UnaryOp v) const {
    if (exp==Expression(v)) return true;
    Expression e=v.exp();
    return apply(e);
  }
  bool ContainsVector::operator()(IfExp v) const {
    if (exp==Expression(v)) return true;
    Expression cond=v.cond(), then=v.then(), elseexp=v.elseexp();
    const bool rc = apply(cond);
    const bool rt = apply(then);
    const bool re = apply(elseexp);
    return rc || rt || re;
  }
  bool ContainsVector::operator()(Range v) const {
    if (exp==Expression(v)) return true;
    Expression start=v.start(), end=v.end();
    bool rs = apply(start);
    bool re = apply(end);
    return rs || re;
  }
  bool ContainsVector::operator()(Brace v) const {
    if (exp==Expression(v)) return true;
    return false;
  }
  bool ContainsVector::operator()(Bracket v) const {
    if (exp==Expression(v)) return true;
    return false;
  }
  bool ContainsVector::operator()(Call expr) const {
    if (exp==Expression(expr)) return true;
    if (is<Call>(exp)) {
      Call call=get<Call>(exp);
      if (expr.name()=="der" && call.name()=="der") {  //Is a derivative expression
        ERROR_UNLESS(is<Reference>(expr.args().front()) && is<Reference>(call.args().front()), "Arguments to call must be a reference");
        Reference vRef=get<Reference>(expr.args().front());
        Reference callRef=get<Reference>(call.args().front());
        if (get<0>(vRef.ref().front())==get<0>(callRef.ref().front())) { //The references are the same
          buildPairs(vRef);
          return true;
        }
        //The references are not the same
        return false;
      }
      else { //Is not a derivative expression, find the occurrence inside the parameters



      }
    }
    return false;
  }
  bool ContainsVector::operator()(FunctionExp v) const {
    if (exp==Expression(v)) return true;
    return false;
  }
  bool ContainsVector::operator()(ForExp v) const {
    if (exp==Expression(v)) return true;
    return false;
  }
  bool ContainsVector::operator()(Named v) const {
    if (exp==Expression(v)) return true;
    return false;
  }
  bool ContainsVector::operator()(Output v) const {
    if (exp==Expression(v)) return true;
    foreach_(OptExp oe, v.args())
      if (oe && apply(oe.get())) return true;
    return false;
  }
  bool ContainsVector::operator()(Reference v) const {
    return false;
    ERROR("Contains for non states not supported yet");
    if (is<Reference>(exp)) {
      if (get<0>(v.ref().front())==get<0>(get<Reference>(exp).ref().front())) {
        //std::cerr << "Checking " << v << " againt " << exp << " count= " << var.count << "\n";
        if (unk2find.count==0) {
          // The variabl is a scalar
          VectorEdgeProperty newEdge;
          if (!foreq)
            newEdge.p_e +=discrete_interval<int>::closed(1, 1);
          else
            newEdge.p_e += forIndexInterval;
          newEdge.p_v += discrete_interval<int>::closed(0, 0);
          edgeList.insert(newEdge);
          return true;
        } else {
          ExpList el = get<1>(v.ref().front());
          if (el.size()==0) { // If there are no sub-indices the complete array is used
            VectorEdgeProperty newEdge;
            // TODO:
            //newEdge.p_v += discrete_interval<int>::closed(1 var.count);
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
            VectorEdgeProperty newEdge;
            newEdge.p_v += discrete_interval<int>::closed(get<Modelica::AST::Integer>(ind),get<Modelica::AST::Integer>(ind));
            if (!foreq)
              newEdge.p_e +=discrete_interval<int>::closed(1, 1);
            else
              newEdge.p_e += forIndexInterval;
            edgeList.insert(newEdge);
            return true;
          } else if (is<Reference>(ind)) {
            ERROR_UNLESS(foreq, "Generic index used outside for equation");
            VectorEdgeProperty newEdge;
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
            VectorEdgeProperty newEdge;
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

//  void ContainsVector::setForIndex(Expression a, Expression b, Name v) {
//     Modelica::EvalExp ev(syms);
//     forStart=boost::apply_visitor(ev,a);
//     forEnd=boost::apply_visitor(ev,b);
//     forIndexVar=v;
//     forIndexInterval =  discrete_interval<int>::closed(forStart,forEnd);
//     foreq=true;
//  }

  void ContainsVector::buildPairs(Reference unkRef) const {
    if (unk2find.count==1) { // The unknown is a scalar (or array of size 1)
      VectorEdgeProperty newEdge;
      if (foreq) { //The equation is a for-equation
        buildPairsNto1();
      }
      else { //The equation is not a for-equation
        buildPairs1to1();
      }
    }
    else { // The unknown is an array
      ExpList el = get<1>(unkRef.ref().front());
      if (el.size()==0) { // If there are no sub-indices the complete array is used
        if (foreq) { //The equation is a for-equation
          buildPairsNtoN();
        }
        else { //The equation is not a for-equation
          buildPairs1toN();
        }
      }
      else { // The unknown has a sub-index
        Expression i = el.front();
        Modelica::PartEvalExp pe(syms);
        Expression ind = boost::apply_visitor(pe,i);
        if (is<Modelica::AST::Integer>(ind)) { // The index is a constant value
          int index = get<Modelica::AST::Integer>(ind);
          ERROR_UNLESS(index<=unk2find.count && index >= 1,"Index out of range");
          if (foreq) { //The equation a for-equation
            buildPairsNto1(index);
          }
          else {  //The equation is not a for-equation
            buildPairs1to1(index);
          }
        }
        else if (is<Reference>(ind)) { // The index is a reference
          ERROR_UNLESS(foreq, "Generic index used outside for equation");
          //TODO: Check that the index is the same variable than the for
          buildPairsNtoN();
        }
        else if (is<BinOp>(ind)) {
           ERROR("BinOp Indexed expression not supported yet");
        }
        else ERROR("Indexed expression not supported yet");
      }
    }
  }

  void ContainsVector::buildPairs1to1(int index) const {
    labels.insert(std::make_pair(1,index));
  }

  void ContainsVector::buildPairsNto1(int index) const {
    for (int i=forIndexInterval.lower();i<=forIndexInterval.upper();i++) {
      labels.insert(std::make_pair(i,index));
    }
  }

  void ContainsVector::buildPairs1toN() const {
    for (int i=1;i<=unk2find.count;i++) {
      labels.insert(std::make_pair(1,i));
    }
  }

  void ContainsVector::buildPairsNtoN() const {
    for (int i=forIndexInterval.lower();i<=forIndexInterval.upper();i++) {
      for (int j=1;j<=unk2find.count;j++) {
        labels.insert(std::make_pair(i,j));
      }
    }
  }

  void ContainsVector::addGenericIndex(BinOp binop) const {
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
    EdgeProperty newEdge;
    newEdge.genericIndex = std::pair<int,int>(a,b);
    newEdge.indexInterval.add(forIndexInterval);
    edgeList.insert(newEdge);
    */
  }

}
