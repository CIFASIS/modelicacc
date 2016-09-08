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
#include <causalize/vector/vector_graph_definition.h>
#include <util/ast_visitors/eval_expression.h>
#include <boost/icl/discrete_interval.hpp>
#include <util/ast_visitors/partial_eval_expression.h>
#include <ast/queries.h>

using namespace boost;
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
    exp(unk.unknown()),
    unk2find(unk),
    syms(s),
    foreq(true),
    indexes(indexes) {
      if (debugIsEnabled('c')) {
        std::cout << "Looking for exp: " << exp <<"\n";
      }
      EvalExpression ev(syms);
      foreach_(Index i, indexes) {
        if (!i.exp())
          ERROR("No index in for equation");
        ERROR_UNLESS(is<Range>(i.exp().get()), "Only range expressions supported");
        Range range = get<Range>(i.exp().get());
        forIndexIntervalList.push_back(Interval::closed(Apply(ev,range.start_ref()), Apply(ev,range.end_ref())));
      }
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
    bool rl = ApplyThis(l);
    bool rr = ApplyThis(r);
    return rr || rl;
  }

  bool ContainsVector::operator()(UnaryOp v) const {
    if (exp==Expression(v)) return true;
    Expression e=v.exp();
    return ApplyThis(e);
  }

  bool ContainsVector::operator()(IfExp v) const {
    if (exp==Expression(v)) return true;
    Expression cond=v.cond(), then=v.then(), elseexp=v.elseexp();
    const bool rc = ApplyThis(cond);
    const bool rt = ApplyThis(then);
    const bool re = ApplyThis(elseexp);
    return rc || rt || re;
  }

  bool ContainsVector::operator()(Range v) const {
    if (exp==Expression(v)) return true;
    Expression start=v.start(), end=v.end();
    bool rs = ApplyThis(start);
    bool re = ApplyThis(end);
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

  bool ContainsVector::operator()(Call call) const {
    if (is<Call>(exp)) { //exp is a derivative expression
      Call callExpr=get<Call>(exp);
      if (call.name()=="der" && callExpr.name()=="der") {  //call and exp are derivative expressions
        ERROR_UNLESS(is<Reference>(call.args().front()) && is<Reference>(callExpr.args().front()), "Arguments to call must be a reference");
        Reference callRef=get<Reference>(call.args().front());
        Reference callExprRef=get<Reference>(callExpr.args().front());
        if (get<0>(callRef.ref().front())==get<0>(callExprRef.ref().front())) { //The references are the same
          if (debugIsEnabled('c')) {
            std::cout << "build pairs with: " << callRef << "\n";
          }
          BuildPairs(callRef);
          return true;
        }
        //The references are not the same
        return false;
      }
    } else { //exp is not a derivative expression, find the occurrence of exp inside the call arguments
      if (exp==Expression(call)) return true;
      bool findOccur = false;
      foreach_ (Expression e, call.args()) {
        findOccur|=(ApplyThis(e));
      }
      return findOccur;
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
      if (oe && ApplyThis(oe.get())) return true;
    return false;
  }


  bool ContainsVector::operator()(Reference ref) const {
    if (is<Reference>(exp)) {
      if (get<0>(ref.ref().front())==get<0>(get<Reference>(exp).ref().front())) { //The references are the same
        BuildPairs(ref);
        return true;
      }
    }
    return false;
  }

  void ContainsVector::BuildPairs(Reference unkRef) const {
    if (foreq) { //The equation is a for-equation
      if (unk2find.unknown.dimension==0) { //the unknown is a scalar
          labels.insert(IndexPair(MDI(forIndexIntervalList), MDI(0), Offset()));
      } else { //the unknown is a vector
        ERROR_UNLESS(unk2find.unknown.dimension==(int)get<1>(unkRef.ref().front()).size(), "Only complete usage of vectors are allowed");
        VarSymbolTable vst=syms;
        std::vector<Name> iterator_names;
        std::vector<int> usage_eq, usage_unk;
        foreach_(Index i, indexes) {
           iterator_names.push_back(i.name());
           VarInfo vinfo = VarInfo(TypePrefixes(), "Integer", Option<Comment>(), Modification());
           vst.insert(i.name(),vinfo);
        }
        usage_eq.resize(indexes.size());
        IntervalList unk_indexes;
        Expression unkRef_exp = unkRef;
        Expression evaluated_expr=Apply(Modelica::PartialEvalExpression(vst),unkRef_exp);
        ERROR_UNLESS(is<Reference>(evaluated_expr), "Evaluated expression is not a reference");
        Reference r = get<Reference>(evaluated_expr);
        ExpList indexes_list = get<1>(r.ref().front());
        unsigned int total_index_uses = 0;
        unsigned int index_count = 0;
        std::list<int> offset_list;
        foreach_(Expression val,indexes_list) {
          if (is<Modelica::AST::Integer>(val)) {
             int v = get<Modelica::AST::Integer>(val);
             unk_indexes.push_back(Interval::closed(v,v)); 
             usage_eq[index_count] = -1;
             offset_list.push_back(0);
             usage_unk.push_back(-1);
          } else if (is<Reference>(val)) {
            Reference ind_ref = get<Reference>(val);
            std::vector<Name>::iterator index_name = std::find(iterator_names.begin(), iterator_names.end(), get<0>(ind_ref.ref().front()));
            ERROR_UNLESS(index_name!=iterator_names.end(), "Usage of variable in index expression not found");
            Index i = indexes.at(index_name-iterator_names.begin());
            if (!i.exp())
              ERROR("Implicit range not supported in for equations");
            ERROR_UNLESS(is<Range>(i.exp().get()), "Only range supported in for equations");
            Range range = get<Range>(i.exp().get());
            EvalExpression ev(syms);
            int pos = index_name - iterator_names.begin();
            usage_unk.push_back(pos);
            usage_eq[pos] = index_count;
            unk_indexes.push_back(Interval::closed(Apply(ev,range.start_ref()), Apply(ev,range.end_ref())));
            total_index_uses++;
            offset_list.push_back(0);
          } else if (is<BinOp>(val)) {
            BinOp binop = get<BinOp>(Apply(Modelica::PartialEvalExpression(vst), val));
            ERROR_UNLESS(binop.op()==Add ||
                         binop.op()==Sub , "Only addition and substraction are supported in indexes");
            ERROR_UNLESS(is<Reference>(binop.left()),"Index must be of the form i+/-K where K is a constant");
            ERROR_UNLESS(is<Modelica::AST::Integer>(binop.right()),"Index must be of the form i+/-K where K is a constant");
            int offset = get<Modelica::AST::Integer>(binop.right());
            if (binop.op()==Sub) // Transform everything to an addition
              offset *= -1;
            offset_list.push_back(offset);
            Reference ind_ref = get<Reference>(binop.left());
            std::vector<Name>::iterator index_name = std::find(iterator_names.begin(), iterator_names.end(), get<0>(ind_ref.ref().front()));
            ERROR_UNLESS(index_name!=iterator_names.end(), "Usage of variable in index expression not found");
            Index i = indexes.at(index_name-iterator_names.begin());
            if (!i.exp())
              ERROR("Implicit range not supported in for equations");
            ERROR_UNLESS(is<Range>(i.exp().get()), "Only range supported in for equations");
            Range range = get<Range>(i.exp().get());
            EvalExpression ev(syms);
            int pos = index_name - iterator_names.begin();
            usage_unk.push_back(pos);
            usage_eq[pos] = index_count;
            //unk_indexes.push_back(CreateInterval(Apply(ev,range.start_ref())+offset, Apply(ev,range.end_ref())+offset));
            total_index_uses++;
          } else {
            ERROR("Index expression not supported");
          }
          index_count++;
        }
        ERROR_UNLESS(total_index_uses==forIndexIntervalList.size(), "The number of indexes does not match the number of uses");
        MDI mdi_eq(forIndexIntervalList),  mdi_unk(unk_indexes);
        ERROR_UNLESS(mdi_eq.Size() == mdi_unk.Size(), "Edge of different size");
        labels.insert(IndexPair(mdi_eq, mdi_unk, offset_list));
      }
    } else { //The equation is not a for-equation
      if (unk2find.unknown.dimension==0) { //the unknown is a scalar
        labels.insert(IndexPair(MDI(0), MDI(0), Offset()));
      } else { //the unknown is a vector
        ERROR_UNLESS(unk2find.unknown.dimension==(int)get<1>(unkRef.ref().front()).size(), "Only complete usage of vectors are allowed");
        VarSymbolTable vst=syms;
        Expression unkRef_exp = unkRef;
        Expression evaluated_expr=Apply(Modelica::PartialEvalExpression(vst),unkRef_exp);
        IntervalList unk_indexes;
        ERROR_UNLESS(is<Reference>(evaluated_expr), "Evaluated expression is not a reference");
        Reference r = get<Reference>(evaluated_expr);
        ExpList indexes = get<1>(r.ref().front());
        foreach_(Expression val, indexes) {
          ERROR_UNLESS(is<Modelica::AST::Integer>(val), "Expression index could not be evaluated"); //TODO: See this error message
          int v = get<Modelica::AST::Integer>(val);
          unk_indexes.push_back(Interval::closed(v,v));
        }
        labels.insert(IndexPair(MDI(0), MDI(unk_indexes), Offset()));
      }
    }
  }

}
