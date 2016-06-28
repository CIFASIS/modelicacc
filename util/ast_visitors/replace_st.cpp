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
#include <util/ast_visitors/replace_st.h>
#include <boost/variant/apply_visitor.hpp>
#define apply(X) boost::apply_visitor(*this,X)

namespace Modelica {

    using namespace boost;
    replace_st::replace_st(Expression l, Expression r): look(l), rep(r), replace_exp(look,rep) {};
    Statement replace_st::operator()(Break v) const {
      return v;
    };
    Statement replace_st::operator()(Return v) const {
      return v;
    };
    Statement replace_st::operator()(Assign v) const {
      if (v.rl()) {
        ExpList & el = v.rl_ref().get();
        foreach_(Expression &e, el) {
          e =boost::apply_visitor(replace_exp, e);
        }
      }
      v.left_ref()=boost::apply_visitor(replace_exp, v.left_ref());
      if (v.right()) 
        v.right_ref()=boost::apply_visitor(replace_exp, v.right_ref().get());
      return v;
    };
    Statement replace_st::operator()(IfSt v) const {
      v.cond_ref()=boost::apply_visitor(replace_exp, v.cond_ref());
      foreach_ (Statement &st, v.elements_ref()) {
        st=apply(st);
      }
      foreach_ (Statement &st, v.ifnot_ref()) {
        st=apply(st);
      }
      foreach_ (If<Statement>::Else &els, v.elseif_ref()) {
        get<0>(els) = boost::apply_visitor(replace_exp, get<0>(els));
        foreach_ (Statement &st, get<1>(els)) 
          st=apply(st);
      }
      return v;
    }
    Statement replace_st::operator()(CallSt v) const {
      v.n_ref()=boost::apply_visitor(replace_exp, v.n_ref());
      foreach_(Expression &e, v.arg_ref()) {
        e=boost::apply_visitor(replace_exp, e);
      }
      foreach_(OptExp &oe, v.out_ref()) {
        if (oe) 
            oe=boost::apply_visitor(replace_exp, oe.get());
      }
      return v;
    };
    Statement replace_st::operator()(ForSt v) const {
      foreach_(Index &i, v.range_ref().indexes_ref()) {
        if (i.exp()) {
          i.exp_ref()=boost::apply_visitor(replace_exp, i.exp_ref().get());
        }
      }
      foreach_(Statement &s, v.elements_ref()) {
        s = apply(s);
      }
      return v;
    };
    Statement replace_st::operator()(WhenSt v) const {
      v.cond_ref()=boost::apply_visitor(replace_exp, v.cond_ref());
      foreach_ (Statement &st, v.elements_ref()) {
        st=apply(st);
      }
      foreach_ (When<Statement>::Else &els, v.elsewhen_ref()) {
        get<0>(els) = boost::apply_visitor(replace_exp, get<0>(els));
        foreach_ (Statement &st, get<1>(els)) 
          st=apply(st);
      }
      return v;
    }
    Statement replace_st::operator()(WhileSt v) const {
      ERROR("Replace in while statement not implemented\n");
      return v;
    }
 
}
