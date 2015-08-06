/*
 * findstate.cpp
 *
 *  Created on: 28/04/2013
 *      Author: Moya
 */

#include <iostream>
#include <boost/variant/get.hpp>

#include <causalize/state_variables_finder.h>
#include <ast/expression.h>
#include <util/table.h>
#include <util/debug.h>
#include <cassert>
#define apply(X) boost::apply_visitor(*this,X)

StateVariablesFinder::StateVariablesFinder(MMO_Class &c): _c(c) { }

void StateVariablesFinder::findStateVariables() {
  foreach_(Equation e, _c.equations().equations()) {
    if (is<Equality>(e)) {
      Equality eq=boost::get<Equality>(e);
      Expression left=eq.left(),right=eq.right();
      apply(left);
      apply(right);
    } else if (is<ForEq>(e)) {
      ForEq fe = boost::get<ForEq>(e);    
      foreach_(Equation feq,fe.elements()) {
        if (is<Equality>(feq)) {
          Equality eq=boost::get<Equality>(feq);
          Expression left=eq.left(),right=eq.right();
          apply(left);
          apply(right);
        } else {
          ERROR("StateVariablesFinder::findStateVariablesInEquations:\n" "Equation type not supported.\n");
        } 
      } 
    } else {
      ERROR("StateVariablesFinder::findStateVariablesInEquations:\n" "Equation type not supported.\n");
    }

  } 
}

    void StateVariablesFinder::operator()(Integer v) const { return ; } 
    void StateVariablesFinder::operator()(Boolean v) const { return ; } 
    void StateVariablesFinder::operator()(String v) const { return ; } 
    void StateVariablesFinder::operator()(Name v) const { return ; }
    void StateVariablesFinder::operator()(Real v) const { return ; } 
    void StateVariablesFinder::operator()(SubEnd v) const { return ; } 
    void StateVariablesFinder::operator()(SubAll v) const { return ; } 
    void StateVariablesFinder::operator()(BinOp v) const { 
      Expression l=v.left(), r=v.right();
      apply(l);
      apply(r);
    } 
    void StateVariablesFinder::operator()(UnaryOp v) const { 
      Expression e=v.exp();
      apply(e);
    } 
    void StateVariablesFinder::operator()(IfExp v) const { 
      Expression cond=v.cond(), then=v.then(), elseexp=v.elseexp();
      apply(cond);
      apply(then);
      apply(elseexp);
    }
    void StateVariablesFinder::operator()(Range v) const { 
      Expression start=v.start(), end=v.end();
      apply(start);
      apply(end);
    }
    void StateVariablesFinder::operator()(Brace v) const { 
      abort();
    }
    void StateVariablesFinder::operator()(Bracket v) const { 
      abort();
    }
    void StateVariablesFinder::operator()(Call v) const { 
      if (v.name()=="der") {
        ERROR_UNLESS(v.args().size()==1, "Call to der() with zero or more than one argument!");
        Expression e = v.args().front();
        ERROR_UNLESS(is<Reference>(e), "Argument to der is not a reference!");
        Reference r = boost::get<Reference>(e);
        ERROR_UNLESS(r.ref().size()==1, "StateVariablesFinder: dotted expression not supported!");
        RefTuple rt = r.ref().front();
        Name name= boost::get<0>(rt);
        VarSymbolTable &vars = _c.syms_ref();
        Option<VarInfo> opt_vinfo = vars[name];
        ERROR_UNLESS(opt_vinfo, "StateVariablesFinder: Variable %s not found", name.c_str());
        VarInfo vinfo = opt_vinfo.get();
        vinfo.state_ref()=true;
        vars.insert(name,vinfo);
      } else {
        foreach_ (Expression e, v.args()) 
          apply(e);
      }
    }
    void StateVariablesFinder::operator()(FunctionExp v) const { 
      return;
    }
    void StateVariablesFinder::operator()(ForExp v) const {
      return;
    }
    void StateVariablesFinder::operator()(Named v) const {
      return;
    }
    void StateVariablesFinder::operator()(Output v) const {
      foreach_(OptExp oe, v.args()) {
        if (oe)
          apply(oe.get());
      }
    }
    void StateVariablesFinder::operator()(Reference v) const {
      return;
    }
