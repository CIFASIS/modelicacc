
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

#include <string>
#include <map>
#include <assert.h>
#include <stdio.h>
#include <sstream>
#include <ginac/ginac.h>
#include <util/ast_visitors/ginac_interface.h>
#include <util/ast_visitors/eval_expression.h>
#include <boost/tuple/tuple.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/lexical_cast.hpp>
#include <util/debug.h>
#include <ast/queries.h>
#include <parser/parser.h>

using namespace GiNaC;
using namespace std;
REGISTER_FUNCTION(der, dummy())
REGISTER_FUNCTION(pre, dummy())

REGISTER_FUNCTION(user_fun1_1, dummy())
REGISTER_FUNCTION(user_fun1_2, dummy())
REGISTER_FUNCTION(user_fun1_3, dummy())

REGISTER_FUNCTION(user_fun2_1, dummy())
REGISTER_FUNCTION(user_fun2_2, dummy())
REGISTER_FUNCTION(user_fun2_3, dummy())

REGISTER_FUNCTION(user_fun3_1, dummy())
REGISTER_FUNCTION(user_fun3_2, dummy())
REGISTER_FUNCTION(user_fun3_3, dummy())

static ex var_derivative(const ex & x,const ex & y, unsigned diff_param) {
  return der(x);
}
REGISTER_FUNCTION(var, derivative_func(var_derivative))


void my_print_power_dflt(const power & p, const print_dflt & c, unsigned level) ;

/*
void my_print_mul_dflt(const mul & m, const print_dflt & c, unsigned level) {
  // get the precedence of the 'power' class
  unsigned power_prec = m.precedence();
  if (level >= power_prec)
    c.s << '(';
  if (m.op(1).match(pow(wild(),-1))) {
    c.s << "(" << m.op(0) << "/" << m.op(1).op(0) << ")";
  } else if (m.op(0).match(pow(wild(),-1))) {
    c.s << "(" << m.op(1) << "/" << m.op(0).op(0) << ")";
  } else {
    c.s << m.op(0) << "*" << m.op(1);
  }
  if (level >= power_prec)
    c.s << ')';
}
void my_print_add_dflt(const add & s, const print_dflt & c, unsigned level) {
  // get the precedence of the 'power' class
  unsigned power_prec = s.precedence();
  if (level >= power_prec)
    c.s << '(';
  if (s.op(0).match(-wild()) && !s.op(1).match(-wild())) {
    c.s << s.op(1) << "-" << s.op(0).op(0);
  } else {
    c.s << s.op(0) << "+" << s.op(1);
  }
  if (level >= power_prec)
    c.s << ')';
}
*/
namespace Modelica {
  
    using namespace GiNaC;
    std::map<int, Name> ConvertToGiNaC::function3_directory;
    int ConvertToGiNaC::user_func1 = 0;
    int ConvertToGiNaC::user_func2 = 0;
    int ConvertToGiNaC::user_func3 = 0;
    ConvertToGiNaC::ConvertToGiNaC(VarSymbolTable  &var, bool forDerivation): varEnv(var),_forDerivation(forDerivation)   { }

    GiNaC::ex ConvertToGiNaC::operator()(Integer v) const { 
      return ex(v);
    }
    GiNaC::ex ConvertToGiNaC::operator()(Boolean v) const { 
      WARNING("ConvertToGiNaC: conversion not implemented. Returning 0");
      return 0;
    }
    GiNaC::ex ConvertToGiNaC::operator()(String v) const {
      WARNING("ConvertToGiNaC: conversion not implemented. Returning 0");
      return 0;
    }
    GiNaC::ex ConvertToGiNaC::operator()(Name v) const { 
      WARNING("ConvertToGiNaC: conversion not implemented. Returning 0");
      return 0;
    }
    GiNaC::ex ConvertToGiNaC::operator()(Real v) const { 
      return ex(v);
    }
    GiNaC::ex ConvertToGiNaC::operator()(SubEnd v) const { 
      WARNING("ConvertToGiNaC: conversion not implemented. Returning 0");
      return 0;
    }
    GiNaC::ex ConvertToGiNaC::operator()(SubAll v) const { 
      WARNING("ConvertToGiNaC: conversion not implemented. Returning 0");
      return 0;
    }
    GiNaC::ex ConvertToGiNaC::operator()(BinOp v) const { 
      Expression l=v.left(),r=v.right();
      switch (v.op()) {
        case Add:
          return ApplyThis(l)+ApplyThis(r);
        case Sub:
          return ApplyThis(l)-ApplyThis(r);
        case Mult:
          return ApplyThis(l)*ApplyThis(r);
        case Div:
          return ApplyThis(l)/ApplyThis(r);
        case Exp:
          return pow(ApplyThis(l),ApplyThis(r));
        default:
          WARNING("ConvertToGiNaC: BinOp conversion not implemented. Returning 0");
          return 0;
      }
    } 
    GiNaC::ex ConvertToGiNaC::operator()(UnaryOp v) const { 
      Expression e=v.exp();
      if (v.op()==Minus) 
        return -ApplyThis(e);
      WARNING("ConvertToGiNaC: Not conversion not implemented. Returning 0");
      return 0;
    } 
    GiNaC::ex ConvertToGiNaC::operator()(IfExp v) const { 
      WARNING("ConvertToGiNaC: IfExp conversion not implemented. Returning 0");
      return 0;
    }
    GiNaC::ex ConvertToGiNaC::operator()(Range v) const { 
      WARNING("ConvertToGiNaC: conversion not implemented. Returning 0");
      return 0;
    }
    GiNaC::ex ConvertToGiNaC::operator()(Brace v) const { 
      WARNING("ConvertToGiNaC: conversion not implemented. Returning 0");
      return 0;
    }
    GiNaC::ex ConvertToGiNaC::operator()(Bracket v) const { 
      WARNING("ConvertToGiNaC: conversion not implemented. Returning 0");
      return 0;
    }
    GiNaC::ex ConvertToGiNaC::operator()(Call v) const { 
      if ("sin"==v.name()) {
        return sin(ApplyThis(v.args()[0]));
      } 
      if ("cos"==v.name()) {
        return cos(ApplyThis(v.args()[0]));
      } 
      if ("pre"==v.name()) {
        return ApplyThis(v.args()[0]);
      } 
      if ("der"==v.name()) {
        Expression arg = v.args().front();
        ERROR_UNLESS(is<Reference>(arg),"Argument to der operator is not a reference\n");
        Reference r = get<Reference>(arg);
        GiNaC::ex exp = ConvertToGiNaC::operator()(r);
        std::stringstream ss;
        ss << "der!" << exp;
        return getSymbol(ss.str());
      } 
      if ("exp"==v.name()) {
        return exp(ApplyThis(v.args()[0]));
      } 
      if ("sum"==v.name()) {
        // Expand sum over unidimensional arrays
        ERROR_UNLESS(is<Reference>(v.args().front()), "Call to sum with no reference");
        Reference ref = get<Reference>(v.args().front());
        Option<VarInfo> opt_vinfo = varEnv[refName(ref)];
        if (!opt_vinfo)
          ERROR("Variable %s not found", refName(ref));
        ERROR_UNLESS(opt_vinfo.get().indices() && opt_vinfo.get().indices().get().size()==1, "sum operator over matrix not supported");
        Expression size_exp = opt_vinfo.get().indices().get().front();
        Real size = Apply(EvalExpression(varEnv), size_exp);
        GiNaC::ex exp = ConvertToGiNaC::operator()(Reference(refName(ref),1));
        for (int i=2; i<=size;i++) {
          exp = exp + ConvertToGiNaC::operator()(Reference(refName(ref),i));
        }
        return exp;
      } 
      if ("log"==v.name()) {
        return GiNaC::log(ApplyThis(v.args()[0]));
      } 
      if ("log10"==v.name()) {
        return GiNaC::log(ApplyThis(v.args()[0]))/GiNaC::log(10);
      } 
      if (user_func3<3 && v.args().size()==3) {
        Expression arg1 = v.args().front();
        Expression arg2 = v.args().at(1);
        Expression arg3 = v.args().at(2);
        function3_directory.insert(make_pair(user_func3, v.name()));
        //std::cerr << "Converting " << v.name() << " as user_fun3_" << user_func3+1 << "\n";
        switch (user_func3++) {
          case 0:
            return user_fun3_1(ApplyThis(arg1), ApplyThis(arg2), ApplyThis(arg3));
          case 1:
            return user_fun3_2(ApplyThis(arg1), ApplyThis(arg2), ApplyThis(arg3));
          case 2:
            return user_fun3_3(ApplyThis(arg1), ApplyThis(arg2), ApplyThis(arg3));
        }
      }
      WARNING("ConvertToGiNaC: conversion of function %s implemented. Returning 0.\n", v.name().c_str());
      return 0;
    }
    GiNaC::ex ConvertToGiNaC::operator()(FunctionExp v) const { 
      WARNING("ConvertToGiNaC: conversion not implemented. Returning 0");
      return 0;
    }
    GiNaC::ex ConvertToGiNaC::operator()(ForExp v) const {
      WARNING("ConvertToGiNaC: conversion not implemented. Returning 0");
      return 0;
    }
    GiNaC::ex ConvertToGiNaC::operator()(Named v) const {
      WARNING("ConvertToGiNaC: conversion not implemented. Returning 0");
      return 0;
    }
    GiNaC::ex ConvertToGiNaC::operator()(Output v) const {
      OptExpList el = v.args();
      ERROR_UNLESS(el.size()==1,"GiNaC conversion of output expression not implemented");
      if (el[0]) {
        return ApplyThis(el[0].get());
      }
      WARNING("ConvertToGiNaC: conversion of output expression. Returning 0");
      return 0;
    }
    GiNaC::ex ConvertToGiNaC::operator()(Reference v) const {
      Ref r=v.ref();
      ERROR_UNLESS(r.size()==1,"GiNaC conversion of dotted references not implemented");
      Option<ExpList> oel = boost::get<1>(r[0]);
      std::string s=boost::get<0>(r[0]);
      if (!_forDerivation || isConstant(s,varEnv) || isParameter(s,varEnv) || isDiscrete(s,varEnv)) {
        if (oel && oel.get().size()) {
          std::stringstream ss;
          ss << s;
          for(Expression e: oel.get()) {
            ERROR_UNLESS(is<Integer>(e) || is<Reference>(e) || is<BinOp>(e), "Not suppoted conversion");
            if (!is<BinOp>(e))
              ss << "!" << e;
            else {
              BinOp bop = get<BinOp>(e);
              int offset = get<Integer>(bop.right());
              if (bop.op()==Sub)
                offset *=-1;
              ss << "!" << bop.left();
              if (offset>0) 
                ss << "@" << offset;
              else 
                ss << "#" << -offset;
            }
          }
          return getSymbol(ss.str());
        } 
        return getSymbol(s);
      } else if (_forDerivation) {
          return var(getSymbol(s),ConvertToGiNaC::getTime());
      } else {
        ERROR("converting a parameter");
        return 0;
      }
    }

    GiNaC::symbol &ConvertToGiNaC::getSymbol(Name s) const {
          std::map<std::string, GiNaC::symbol>::iterator i = directory.find(s);
          if (i != directory.end())
            return i->second;
          else
            return directory.insert(make_pair(s, symbol(s))).first->second;
    }
    GiNaC::symbol &ConvertToGiNaC::getTime() const {
      std::string s="time";
      std::map<std::string, GiNaC::symbol>::iterator i = directory.find(s);
      if (i != directory.end())
        return i->second;
      return directory.insert(make_pair(s, symbol(s))).first->second;
    }
    
 class GiNaCtoModelica 
 : public visitor,          // this is required
   public add::visitor,     // visit add objects
   public GiNaC::function::visitor,     // visit add objects
   public mul::visitor,     // visit add objects
   public power::visitor,     // visit add objects
   public GiNaC::numeric::visitor, // visit numeric objects
   public symbol::visitor, // visit numeric objects
   public basic::visitor    // visit basic objects
{
    void visit(const power & x) { result_ = Output(BinOp(ConvertToExp(x.op(0)), Exp , ConvertToExp(x.op(1)))); }
    void visit(const add & x) { 
      result_ = Output(BinOp(ConvertToExp(x.op(0)), Add, ConvertToExp(x.op(1)))); 
      for (unsigned int op = 2;op<x.nops();op++)
        result_ = Output(BinOp(result_, Add, ConvertToExp(x.op(op)))); 
    }
    void visit(const GiNaC::function & x) { 
      const std::string name = x.get_name();
      if ("sin"==name) {
        result_ = Call(name, ConvertToExp(x.op(0)));
        return; 
      } 
      if ("user_fun3_1"==name) {
        //std::cerr << ConvertToGiNaC::function3_directory[0];
        result_ = Call(ConvertToGiNaC::function3_directory[0],{ConvertToExp(x.op(0)), ConvertToExp(x.op(1)), ConvertToExp(x.op(2))});
        return ;
      } 
      if ("user_fun3_2"==name) {
        std::cerr << ConvertToGiNaC::function3_directory[1];
        result_ = Call(ConvertToGiNaC::function3_directory[1],{ConvertToExp(x.op(0)), ConvertToExp(x.op(1)), ConvertToExp(x.op(2))});
        return ;
      } 
      if ("user_fun3_3"==name) {
        std::cerr << ConvertToGiNaC::function3_directory[2];
        result_ = Call(ConvertToGiNaC::function3_directory[2],{ConvertToExp(x.op(0)), ConvertToExp(x.op(1)), ConvertToExp(x.op(2))});
        return ;
      } 
      ERROR("Function not supported in GiNaC conversion");

    }
    void visit(const mul & x) { 
      result_ = Output(BinOp(ConvertToExp(x.op(0)), Mult, ConvertToExp(x.op(1)))); 
      for (unsigned int op = 2;op<x.nops();op++)
        result_ = Output(BinOp(result_, Mult, ConvertToExp(x.op(op)))); 
    }
    void visit(const GiNaC::numeric & x) { result_ = x.to_double(); 
      if (x.to_double()<0) 
        result_=Output(result_);
    } 
    void visit(const GiNaC::symbol & x) { 
      std::string name = x.get_name();;
      std::vector< std::string > sp, bop;
      bool is_der=false;
      bool found_name=false;
      boost::algorithm::split(sp, name, boost::is_any_of("!"));
      ExpList el;
      for(std::string s:sp) {
        if ("der"==s) {
          is_der=true;
          continue;
        }
        if (!found_name) {
          name = s;
          found_name = true;
          continue;
        }
        try {
          el.push_back(boost::lexical_cast<int>(s));
        } catch (boost::bad_lexical_cast) { // Unprocess offset  
          if (boost::algorithm::find_first(s, "@")) {
            boost::algorithm::split(bop, s, boost::is_any_of("@"));
            el.push_back(BinOp(Reference(bop.front()), Add, boost::lexical_cast<int>(bop.at(1))));
          } else if (boost::algorithm::find_first(s, "#")) {
            boost::algorithm::split(bop, s, boost::is_any_of("#"));
            el.push_back(BinOp(Reference(bop.front()), Sub, boost::lexical_cast<int>(bop.at(1))));
          } else {  
            el.push_back(Reference(s));
          }
        } 
      }
      if (!is_der) 
        result_ = Reference(name, el);
      else
        result_ = Call("der", Reference(name, el));
    }
    void visit(const basic & x) { std::cerr << x << std::endl; ERROR("Could not convert GiNaC expression to Modelica"); }
    
    Expression result_;
  public:
    Expression exp() { return result_; }
  };

    Expression ConvertToExp(GiNaC::ex e) {
      GiNaCtoModelica v;
      e.accept(v);
      return v.exp();
    }

}
