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
#include <util/ast_visitors/all_expressions.h>
#include <util/ast_visitors/eval_expression.h>
#include <boost/variant/get.hpp>

namespace Modelica {
    AllExpressions::AllExpressions(Expression e): exp(e) {};
    AllExpressions::AllExpressions(Expression e, VarSymbolTable vst): exp(e), st(vst) {};
    Lexp AllExpressions::operator()(Integer v) const { return (exp==Expression(v)?Lexp(1,v):Lexp()); } 
    Lexp AllExpressions::operator()(Boolean v) const { return (exp==Expression(v)?Lexp(1,v):Lexp()); } 
    Lexp AllExpressions::operator()(String v) const { return (exp==Expression(v)?Lexp(1,v):Lexp()); } 
    Lexp AllExpressions::operator()(Name v) const { return (exp==Expression(v)?Lexp(1,v):Lexp()); }
    Lexp AllExpressions::operator()(Real v) const { return (exp==Expression(v)?Lexp(1,v):Lexp()); } 
    Lexp AllExpressions::operator()(SubEnd v) const { return (exp==Expression(v)?Lexp(1,v):Lexp()); } 
    Lexp AllExpressions::operator()(SubAll v) const { return (exp==Expression(v)?Lexp(1,v):Lexp()); } 
    Lexp AllExpressions::operator()(BinOp v) const { 
      Lexp rta = (exp==Expression(v)?Lexp(1,v):Lexp()); 
      Expression l=v.left(), r=v.right();
      Lexp aux = ApplyThis(l);
      rta.insert(rta.end(), aux.begin(), aux.end());
      aux = ApplyThis(r);
      rta.insert(rta.end(), aux.begin(), aux.end());
      return rta;
    } 
    Lexp AllExpressions::operator()(UnaryOp v) const { 
      Lexp rta = (exp==Expression(v)?Lexp(1,v):Lexp()); 
      Expression e=v.exp();
      Lexp aux = ApplyThis(e);
      rta.insert(rta.end(), aux.begin(), aux.end());
      return rta;
    } 
    Lexp AllExpressions::operator()(IfExp v) const { 
      Lexp rta = (exp==Expression(v)?Lexp(1,v):Lexp()); 
      Expression cond=v.cond(), then=v.then(), elseexp=v.elseexp();
      Lexp aux = ApplyThis(cond);
      rta.insert(rta.end(), aux.begin(), aux.end());
      aux = ApplyThis(then);
      rta.insert(rta.end(), aux.begin(), aux.end());
      aux = ApplyThis(elseexp);
      rta.insert(rta.end(), aux.begin(), aux.end());
      return rta;
    }
    Lexp AllExpressions::operator()(Range v) const { 
      Lexp rta = (exp==Expression(v)?Lexp(1,v):Lexp()); 
      Expression start=v.start(), end=v.end();
      Lexp aux = ApplyThis(start);
      rta.insert(rta.end(), aux.begin(), aux.end());
      aux = ApplyThis(end);
      rta.insert(rta.end(), aux.begin(), aux.end());
      return rta;
    }
    Lexp AllExpressions::operator()(Brace v) const { 
      Lexp rta = (exp==Expression(v)?Lexp(1,v):Lexp()); 
      ERROR("ContainsExpression: Brace expression not supported");
      //TODO
      return rta;
    }
    Lexp AllExpressions::operator()(Bracket v) const { 
      Lexp rta = (exp==Expression(v)?Lexp(1,v):Lexp()); 
      ERROR("ContainsExpression: Bracket expression not supported");
      //TODO
      return rta;
    }
    Lexp AllExpressions::operator()(Call v) const { 
      Lexp rta = (exp==Expression(v)?Lexp(1,v):Lexp()); 
      foreach_ (Expression e, v.args()) {
         Lexp aux = ApplyThis(e);
         rta.insert(rta.end(), aux.begin(), aux.end());
 	  }
      return rta;
    }
    
    Lexp AllExpressions::operator()(FunctionExp v) const { 
      Lexp rta = (exp==Expression(v)?Lexp(1,v):Lexp()); 
      //TODO
      return rta;
    }
    Lexp AllExpressions::operator()(ForExp v) const {
      Lexp rta = (exp==Expression(v)?Lexp(1,v):Lexp()); 
      Indexes indexes = v.indices();
      ERROR_UNLESS(indexes.indexes().size() == 1, "Only unidimensional for expression supported");
      Index ind = indexes.indexes().front();
      if (!ind.exp())
        ERROR("Index with no expression");
      Expression exp_ind = ind.exp().get();
      ERROR_UNLESS(is<Range>(exp_ind), "Only range supported in for expression");
      EvalExpression ev(st);
      Expression start_exp = get<Range>(exp_ind).start(), end_exp = get<Range>(exp_ind).end();
      int start = Apply(ev,start_exp);
      int end = Apply(ev,end_exp);
      int i;
      for (i=start; i<= end ; i++) {
        VarInfo vinfo = VarInfo(TypePrefixes(1,parameter), "Integer", Option<Comment>(), Modification(ModEq(Expression(i))));
        st.insert(ind.name(), vinfo);
        Expression exp_for = v.exp();
        Lexp aux = ApplyThis(exp_for);
        rta.insert(rta.end(), aux.begin(), aux.end());
      }
      return rta;
    }
    Lexp AllExpressions::operator()(Named v) const {
      Lexp rta = (exp==Expression(v)?Lexp(1,v):Lexp()); 
      //TODO
      return rta;
    }
    Lexp AllExpressions::operator()(Output v) const {
      Lexp rta = (exp==Expression(v)?Lexp(1,v):Lexp()); 
      foreach_ (OptExp oe, v.args()) 
        if (oe){
	     Lexp aux = ApplyThis(oe.get());
         rta.insert(rta.end(), aux.begin(), aux.end());
	 }
      return rta;
    }
    Lexp AllExpressions::operator()(Reference v) const {
      Lexp rta = (exp==Expression(v)?Lexp(1,v):Lexp()); 
      if (is<Reference>(exp)) {
        Reference ref = get<Reference>(exp);
        // Check wheter a[1] is contained in a
        //~ if (get<0>(v.ref().front())==get<0>(ref.ref().front()))
			rta.push_back(v);
		//~ if (get<0>(v.ref().front())==get<0>(ref.ref().front()) && get<1>(v.ref().front()).size()==1 && get<1>(ref.ref().front()).size()==1)
			//~ return get<1>(v.ref().front()) == get<1>(ref.ref().front());

      }
      return rta;
    }
}
