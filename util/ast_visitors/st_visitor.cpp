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

#include <util/ast_visitors/st_visitor.h>
#include <boost/variant/apply_visitor.hpp>
#define apply(X) boost::apply_visitor(*this,X)
#define applyExp(X) boost::apply_visitor(v,X)

namespace Modelica {

    using namespace boost;

    template <typename Visit>
    StVisitor<Visit>::StVisitor(Visit visit): v(visit) {};
    
    template <typename Visit> Statement StVisitor<Visit>::operator()(Assign st) const { 
      Expression left=st.left();
      OptExp right=st.right();
      ExpList rl;
      
      if (st.rl()) {
		  foreach_(Expression e, st.rl().get())
			rl.push_back(applyExp(e));
		  return Assign(applyExp(left), Option<ExpList>(rl));		
	  } else if (right) 
		  return Assign(applyExp(left), OptExp(applyExp(right.get())));
		else 
		   return Assign(applyExp(left));	
    };
    
    template <typename Visit> Statement StVisitor<Visit>::operator()(Break st) const { 
      return st;
    };
    
    template <typename Visit> Statement StVisitor<Visit>::operator()(Return st) const { 
      return st;
    }
    
    template <typename Visit> Statement StVisitor<Visit>::operator()(CallSt st) const { 
		Expression n = st.n();
		ExpList list; 
		OptExpList out;
		foreach_(Expression s, st.arg())
			list.push_back(applyExp(s));
		
		foreach_(OptExp s, st.out())
			if (s) out.push_back(OptExp(applyExp(s.get())));
			else out.push_back(OptExp());
		return CallSt(applyExp(n),list, out);	
    }
    
    template <typename Visit> Statement StVisitor<Visit>::operator()(ForSt st) const { 
	  StatementList eqs;
	  IndexList index;
	  foreach_(Statement e,st.elements())
		eqs.push_back(apply(e));			
	  foreach_(Index i,st.range().indexes())	
		if (i.exp()) index.push_back(Index(i.name(), OptExp(applyExp(i.exp().get())))); 
		else index.push_back(Index(i.name(), OptExp())); 
      return ForSt(Indexes(index),eqs);
    }
    
    template <typename Visit> Statement StVisitor<Visit>::operator()(IfSt st) const { 
		typedef tuple<Expression,std::vector<Statement> >  Else;
        typedef std::vector<Else> ElseList;
		Expression cond = st.cond();
		StatementList elements,elses;
		ElseList elseIf;
		
		foreach_(Statement e, st.elements() )
			elements.push_back(apply(e));
			
		foreach_(Statement e, st.ifnot() )
			elses.push_back(apply(e));	
			
		foreach_(Else el, st.elseif()) {
			StatementList list;
			foreach_(Equation e, get<1>(el))
				list.push_back(apply(e));
			elseIf.push_back(Else(applyExp(get<0>(el)),list));		
		}			
			
		return IfSt(applyExp(cond) , elements, elseIf,elses);
    }
    
    template <typename Visit> Statement StVisitor<Visit>::operator()(WhenSt st) const { 
		typedef tuple<Expression,std::vector<Statement> >  Else;
        typedef std::vector<Else> ElseList;
		Expression cond = st.cond();
		StatementList elements;
		ElseList elsewhen;
		
		foreach_(Statement e, st.elements() )
			elements.push_back(apply(e));
			
		foreach_(Else el, st.elsewhen()) {
			StatementList list;
			foreach_(Statement e, get<1>(el))
				list.push_back(apply(e));
			elsewhen.push_back(Else(applyExp(get<0>(el)),list));		
		}			
			
		return WhenSt(applyExp(cond) , elements, elsewhen);
    }
    
    template <typename Visit> Statement StVisitor<Visit>::operator()(WhileSt st) const { 
		Expression cond = st.cond();
		StatementList elements;
		foreach_(Statement e, st.elements() )
			elements.push_back(apply(e));			
		return WhileSt(applyExp(cond) , elements);
    }
    
    
}
