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

#include <util/ast_visitors/constExp.h>
#include <boost/variant/apply_visitor.hpp>
#define apply(X) boost::apply_visitor(*this,X)

namespace Modelica {

    using namespace boost;
    constExp::constExp() {};
    bool constExp::operator()(Integer v) const { 
      return true;
    }
    
    bool constExp::operator()(Boolean v) const { 
      return true;
    }
    
    bool constExp::operator()(String v) const {
      return true;
    }
    
    bool constExp::operator()(Name v) const { 
      return true;
    }
    
    bool constExp::operator()(Real v) const { 
      return true;
    }
    
    bool constExp::operator()(SubEnd v) const { 
      return false;
    }
    
    bool constExp::operator()(SubAll v) const { 
      return false;
    }
    
    bool constExp::operator()(BinOp v) const { 
      Expression l=v.left(), r=v.right();
      return apply(l) && apply(r);
    }
     
    bool constExp::operator()(UnaryOp v) const { 
      Expression e =v.exp();
      return apply(e);
    } 
    
    bool constExp::operator()(IfExp v) const { 
      Expression cond = v.cond();
      Expression then = v.then();
      Expression elseexp = v.elseexp();
      bool list = true;
      foreach_(ExpPair p, v.elseif())
		list = list &&  apply(get<0>(p)) && apply(get<1>(p));
      return apply(cond) && apply(then) && list && apply(elseexp);
    }
    
    bool constExp::operator()(Range v) const { 
	  Expression start = v.start(),end=v.end();	
	  if (v.step()) {
		Expression step = v.step().get();  
		return apply(start) && apply(step) && apply(end);
	  } else 
		return apply(start) && apply(end);
    }
    
    bool constExp::operator()(Brace v) const { 
      bool list = true;
      foreach_(Expression e, v.args())
		list = list && apply(e);
      return list;
    }
    
    bool constExp::operator()(Bracket v) const { 
	  bool list = true;
	  foreach_(ExpList els, v.args()) 
		  foreach_(Expression e, els)
			list  &= apply(e);
      return list;
    }
    
    bool constExp::operator()(Call v) const { 
      bool list = true;
      foreach_(Expression e, v.args())
		list &= apply(e);
      return list;
    }
    
    bool constExp::operator()(FunctionExp v) const { 
      bool list = false;
      foreach_(Expression e, v.args())
		list &= apply(e);
      return list;
    }
    
    bool constExp::operator()(ForExp v) const {
      Expression exp = v.exp();
      bool indices = true;
      foreach_(Index i, v.indices().indexes()) {
        if (i.exp())
		      indices &= apply(i.exp().get());
      } 
      return indices;
    }
    
    bool constExp::operator()(Named v) const {
	  Expression exp = v.exp();	
      return apply(exp);
    }
    
    bool constExp::operator()(Output v) const {
      bool list = true;
      foreach_(OptExp e, v.args())
	     if (e)	
			list &= apply(e.get()); 
      return list;
    }
    
    bool constExp::operator()(Reference v) const {
		return false;
    }
}
