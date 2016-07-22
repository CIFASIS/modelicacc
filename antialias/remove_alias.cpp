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

#include <antialias/remove_alias.h>
#include <ast/equation.h>
#include <ast/queries.h>
#include <boost/variant/get.hpp>
#include <util/ast_visitors/partial_eval_expression.h>
#include <util/ast_visitors/replace_equation.h>
#include <util/ast_visitors/replace_statement.h>
#include <causalize/state_variables_finder.h>
#include <algorithm>
#include <vector>


namespace Modelica {
RemoveAlias::RemoveAlias(MMO_Class &c): _c(c) {
  StateVariablesFinder svf(c); 
  svf.findStateVariables();
}
void RemoveAlias::removeAliasEquations() {
  EquationList &el = _c.equations_ref().equations_ref();
  VarSymbolTable &syms = _c.syms_ref();
  int aliased;
  do {
  aliased=0;
  foreach_(Equation &e, el) {
    if (is<Equality>(e)) {
      Equality &eq = boost::get<Equality>(e);
      PartialEvalExpression eval(syms, true);
      Expression left= Apply(eval,eq.left_ref());      
      Expression right= Apply(eval,eq.right_ref());      
      eq.left_ref()=left;
      eq.right_ref()=right;
      if (is<UnaryOp>(left)) { // -a = ... ---> a = - ...
        UnaryOp u = get<UnaryOp>(left);
        if (u.op()==Minus) {
          Expression exp = u.exp();
          if (is<Reference>(exp)) {
            left = exp;
            right = UnaryOp(right,Minus);
            right= Apply(eval,right);      
          }
        }
      }
      if (is<Real>(left) || is<Integer>(left)) { // Always put contant values on the right
        Expression t=right;
        right=left;
        left=t;
      }
      if (isZero(right) && is<BinOp>(left)) {
        BinOp bop = get<BinOp>(left);
        if (bop.op()==Add) {
          right = UnaryOp(bop.right(),Minus);
          right= Apply(eval,right);      
          left = bop.left();
        }
        if (bop.op()==Sub) {
          right = bop.right();
          left = bop.left();
        }
      }
      if (is<UnaryOp>(left) && get<UnaryOp>(left).op()==Minus && is<Reference>(get<UnaryOp>(left).exp())) {
        left = get<UnaryOp>(left).exp();
        right = UnaryOp(right,Minus);
        right= Apply(eval,right);      
      } 
      if (is<Reference>(left) && is<Reference>(right)) { // a = b case
        Reference l = get<Reference>(left);
        Reference r = get<Reference>(right);
        if (l.ref().size()>1) 
          ERROR("antialias must be run on a flat model");
        if (r.ref().size()>1) 
          ERROR("antialias must be run on a flat model");
        if ((get<1>(l.ref().front()).size()==0) && (get<1>(r.ref().front()).size()==0)) {
          if (isVariable(refName(l),syms) && isVariable(refName(r),syms)) {
            if (!isState(refName(l),syms)) {
              el.erase(std::find(el.begin(),el.end(),e)); 
              alias(l,r);
              aliased=1;
              break;
            } else if (!isState(refName(r),syms)) {
              el.erase(std::find(el.begin(),el.end(),e)); 
              alias(r,l);
              aliased=1;
              break;
            }
          }
        }
      } else  if (is<Reference>(left) && is<UnaryOp>(right)) { //  a= -b case
        Reference l = get<Reference>(left);
        UnaryOp u = get<UnaryOp>(right);
        if (is<Reference>(u.exp())) {
          Reference r = get<Reference>(u.exp());
          if (l.ref().size()>1) ERROR("antialias must be run on a flat model");
          if (r.ref().size()>1) ERROR("antialias must be run on a flat model");
          if ((get<1>(l.ref().front()).size()==0) && (get<1>(r.ref().front()).size()==0)) {
            if (isVariable(refName(l),syms) && isVariable(refName(r),syms)) {
              if (!isState(refName(l),syms)) {
                el.erase(std::find(el.begin(),el.end(),e)); 
                alias(l,right);
                aliased=1;
                break;
              }
            }
          }
        }
      } else  if (is<Reference>(left) && (is<Real>(right) || is<Integer>(right))) { //  a= K case
        Reference l = get<Reference>(left);
        if (l.ref().size()>1) 
          ERROR("antialias must be run on a flat model");
        if (get<1>(l.ref().front()).size()>0) continue;
          el.erase(std::find(el.begin(),el.end(),e)); 
          alias(l,right);
          aliased=1;
          break;
      }
    } else if (is<ForEq>(e)) {
      ForEq feq = get<ForEq>(e);
      ERROR_UNLESS(feq.elements().size()==1, "Antialias not supported on multi-equation for");
      ERROR_UNLESS(is<Equality>(feq.elements().front()), "Antialias not supported on non equality equation inside for");
      Equality eq = get<Equality>(feq.elements().front());
      Expression left= eq.left();      
      Expression right= eq.right();      
      if (isZero(left)) {
        Expression t=right;
        right=left;
        left=t;
      }
      PartialEvalExpression eval(syms);
      OptExp o_ind =  feq.range().indexes().front().exp();
      if (!o_ind) continue;
      if (!is<Range>(o_ind.get())) continue;
      Range range = get<Range>(o_ind.get());
      Expression start_exp = Apply(eval,range.start_ref());
      Expression end_exp = Apply(eval,range.end_ref());
      if (!is<Integer>(start_exp) || !is<Integer>(end_exp)) continue;
      int start = get<Integer>(start_exp);
      int end = get<Integer>(end_exp);
      if (start!=1) continue;
      if (is<Reference>(left) && is<Reference>(right)) { // a = b case
        Reference l = get<Reference>(left);
        Reference r = get<Reference>(right);
        if (l.ref().size()>1) 
          ERROR("antialias must be run on a flat model");
        if (r.ref().size()>1) 
          ERROR("antialias must be run on a flat model");
        if (isVariable(refName(l),syms) && isVariable(refName(r),syms)) {
          if (isState(refName(r),syms) || isState(refName(l),syms))
          continue;
          if (!isArray1(refName(r),syms) || !isArray1(refName(l),syms))
          continue;
          Expression size_l = arraySize(refName(l),syms);
          Expression size_r = arraySize(refName(r),syms);
          size_l = Apply(eval,size_l);
          size_r = Apply(eval,size_r);
          if (!is<Integer>(size_l) || !is<Integer>(size_r)) continue;
          int sz_l = get<Integer>(size_l);
          int sz_r = get<Integer>(size_r);
          if (end!=sz_l || end!=sz_r) continue;
          el.erase(std::find(el.begin(),el.end(),e)); 
          std::cerr << "Aliasing " << l << " with  " << r << "\n";
          alias(l,r);
          aliased=1;
          break;
        }
      } 
    } 
  }
  } while (aliased);

}
void RemoveAlias::alias(Reference a, Expression b) { // Remove a from the model and replace every occurence with b
  VarSymbolTable &syms = _c.syms_ref();
  syms.remove(refName(a));
  
  // Remove variable a
  std::vector<Name> &vars = _c.variables_ref();
  std::vector<Name>::iterator pos = std::find(vars.begin(),vars.end(), refName(a));
  if (pos!=vars.end())
    vars.erase(pos);
  ReplaceEquation req(a,b);
  foreach_ (Equation &eq, _c.equations_ref().equations_ref()) {
    //std::cerr << eq << " is now ";
    eq=Apply(req,eq);
    //std::cerr << eq << "\n";
  }
  foreach_ (Equation &eq, _c.initial_eqs_ref().equations_ref()) 
    eq=Apply(req,eq);
  ReplaceStatement rst(a,b);
  foreach_ (Statement &st, _c.statements_ref().statements_ref()) 
    st=Apply(rst,st);
  foreach_ (Statement &st, _c.initial_sts_ref().statements_ref()) 
    st=Apply(rst,st);

}
};

