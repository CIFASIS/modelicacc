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

#include <util/ast_visitors/dotExpression.h>
#include <ast/queries.h>
#include <boost/variant/apply_visitor.hpp>
#include <util/type.h>
#define apply(X) boost::apply_visitor(*this,X)

namespace Modelica {

    using namespace boost;
    dotExpression::dotExpression(Option<MMO_Class &> m, Name n, ExpList xs): _class(m), prefix(n), index(xs) 
    {
	if (m)
		syms = m.get().syms_ref();
    };
    Expression dotExpression::operator()(Integer v) const { 
      return v;
    }
    Expression dotExpression::operator()(Boolean v) const { 
      return v;
    }
    Expression dotExpression::operator()(String v) const {
      return v;
    }
    Expression dotExpression::operator()(Name v) const { 
      return v;
    }
    Expression dotExpression::operator()(Real v) const { 
      return v;
    }
    Expression dotExpression::operator()(SubEnd v) const { 
      return v;
    }
    Expression dotExpression::operator()(SubAll v) const { 
      return v;
    }
    Expression dotExpression::operator()(BinOp v) const { 
      Expression l=v.left(), r=v.right();
      return BinOp(apply(l), v.op(), apply(r));
    } 
    Expression dotExpression::operator()(UnaryOp v) const { 
      Expression e =v.exp();
      return UnaryOp(apply(e),v.op());
    } 
    
    Expression dotExpression::operator()(IfExp v) const { 
      Expression cond = v.cond();
      Expression then = v.then();
      Expression elseexp = v.elseexp();
      List<ExpPair> list;
      foreach_(ExpPair p, v.elseif())
		list.push_back( ExpPair( apply(get<0>(p)) , apply(get<1>(p))  ) );
      return IfExp(apply(cond),apply(then),list,apply(elseexp));
    }
    
    Expression dotExpression::operator()(Range v) const { 
	  Expression start = v.start(),end=v.end();	
	  if (v.step()) {
		Expression step = v.step().get();  
		return Range(apply(start),apply(step),apply(end));
	  } else 
		return Range(apply(start),apply(end));
	  return v;
    }
    Expression dotExpression::operator()(Brace v) const { 
      ExpList list;
      foreach_(Expression e, v.args())
		list.push_back(apply(e));
      return Brace(list);
    }
    Expression dotExpression::operator()(Bracket v) const { 
	  ExpListList list;
	  foreach_(ExpList els, v.args()) {
		  ExpList l;
		  foreach_(Expression e, els)
			l.push_back(apply(e));
		  list.push_back(l); 	
	  }	  
      return Bracket(list);
    }
    Expression dotExpression::operator()(Call v) const { 
      ExpList list;
      foreach_(Expression e, v.args())
		list.push_back(apply(e));
      return Call(v.name(),list);
    }
    Expression dotExpression::operator()(FunctionExp v) const { 
      ExpList list;
      foreach_(Expression e, v.args())
		list.push_back(apply(e));
      return FunctionExp(v.name(),list);
    }

    Expression dotExpression::operator()(ForExp v) const {
      Expression exp = v.exp();
      ExpList indices;
      foreach_(Expression e, v.indices())
		indices.push_back(apply(e));
      return ForExp(apply(exp),indices);
    }
    
    Expression dotExpression::operator()(Named v) const {
	  Expression exp = v.exp();
      return Named(v.name(),apply(exp));
    }
    
    Expression dotExpression::operator()(Output v) const {
      OptExpList list;
      foreach_(OptExp e, v.args())
	     if (e)
			list.push_back(apply(e.get()));
		 else
			list.push_back(OptExp());
      return Output(list);
    }
  Expression dotExpression::operator()(Reference v) const {
	int i = 0,j = v.ref().size();
	Ref ref;
	Name name;
	if (syms) {
		Option<VarInfo> opt_vinfo = syms.get()[get<0>(v.ref()[0])];
		if (opt_vinfo && opt_vinfo.get().builtin()) { // If it is a builtin variable leave it as it is
          		return v;	
        	}

		OptExp opconst = findConst(v);
		if (opconst) return opconst.get();

        	if (!opt_vinfo) {
          		return v;
        	}
	}
	ExpList indices = index;
	foreach_(RefTuple p,v.ref()) {
		if (i==0 && syms)  {
			if (syms.get()[get<0>(p)])
				name = prefix + "_";
		}
    		name += get<0>(p) + ( i == j-1 ? "" : "_" );
		indices += get<1>(p);
		 i++;
	}
	ref.push_back(RefTuple(name,indices));
      	return Reference(ref);
  }

  Option<Expression> dotExpression::findConst(Reference v) const {
	ClassFinder cf = ClassFinder();
	MMO_Class c = _class.get();
	int i = 0;
	foreach_(RefTuple p,v.ref()) {
		Name n = get<0>(p);
		OptTypeDefinition otd = cf.resolveDependencies(c,n);
		if (otd) {
			typeDefinition td = otd.get();
			c = * boost::get<Type::Class>(get <1>(td)).clase();
		} else if ( i != 0 ) {
			Option<VarInfo> Opvv = c.syms_ref()[n];
			if (Opvv) {
				VarInfo vv = Opvv.get();
				if (vv.modification() && is<ModEq>(vv.modification().get()))  {
					Expression exp = get<ModEq>(vv.modification().get()).exp();
					dotExpression  visitor = dotExpression(Option<MMO_Class &>(c),"",ExpList() );
					Expression ret = boost::apply_visitor(visitor,exp);
					return OptExp(ret);
					//std::cerr << "Encontre al objetivo " << ret << std::endl;
				}
			} else return OptExp();
		} else return OptExp();
		i++;
	}
	return OptExp(); 
  }

}
