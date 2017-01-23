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

/*
 * Contains_unknown.cpp
 *
 *  Created on: 17 oct. 2016
 *      Author: D.Hollmann
 */

#include <causalize/contains_unknown.h>
#include <util/debug.h>
#include <boost/variant/get.hpp>
#include <util/ast_visitors/eval_expression.h>
#include <util/ast_visitors/partial_eval_expression.h>
#include <ast/queries.h>

using namespace boost;
using namespace Modelica;
using namespace Modelica::AST ;

namespace Causalize {
  ContainsUnknown::ContainsUnknown(std::vector<Expression> unks, const VarSymbolTable &s): syms(s){ 
    int i=0;
    for (Expression e: unks) {
      std::stringstream ss;
      ss << e;
      definedUnks.insert(make_pair(ss.str(),i++));
    }
    
  }
  void ContainsUnknown::clear() {
    usedUnks.clear(); 
  }

  bool ContainsUnknown::operator()(Modelica::AST::Integer v) const { return false; }

  bool ContainsUnknown::operator()(Boolean v) const { return false; }

  bool ContainsUnknown::operator()(String v) const { return false; }

  bool ContainsUnknown::operator()(Name v) const { return false; }

  bool ContainsUnknown::operator()(Real v) const { return false; }

  bool ContainsUnknown::operator()(SubEnd v) const { return false; }

  bool ContainsUnknown::operator()(SubAll v) const { return false; }

  bool ContainsUnknown::operator()(BinOp v) const {
    Expression l=v.left(), r=v.right();
    bool rl = ApplyThis(l);
    bool rr = ApplyThis(r);
    return rr || rl;
  }

  bool ContainsUnknown::operator()(UnaryOp v) const {
    Expression e=v.exp();
    return ApplyThis(e);
  }

  bool ContainsUnknown::operator()(IfExp v) const {
    Expression cond=v.cond(), then=v.then(), elseexp=v.elseexp();
    const bool rc = ApplyThis(cond);
    const bool rt = ApplyThis(then);
    const bool re = ApplyThis(elseexp);
    return rc || rt || re;
  }

  bool ContainsUnknown::operator()(Range v) const {
    Expression start=v.start(), end=v.end();
    bool rs = ApplyThis(start);
    bool re = ApplyThis(end);
    return rs || re;
  }

  bool ContainsUnknown::operator()(Brace v) const {
    bool ret = false;
    for (Expression exp: v.args()) {
      bool b = ApplyThis(exp);
      ret |= b;
    }
    return ret;
  }

  bool ContainsUnknown::operator()(Bracket v) const {
    bool ret = false;
    for (Expression exp: v.args()) {
      bool b = ApplyThis(exp);
      ret |= b;
    }
    return ret;
  }

  bool ContainsUnknown::operator()(Call call) const {
    if ("der"!=call.name())
      return false;
    int usage = IsUsed(call);
    if (usage!=-1) {
      usedUnks.insert(usage);
      return true;
    } else {
      return false;
    }
  }

  bool ContainsUnknown::operator()(FunctionExp v) const {
    bool ret = false;
    for (Expression exp: v.args()) {
      bool b = ApplyThis(exp);
      ret |= b;
    }
    return ret;
  }

  bool ContainsUnknown::operator()(ForExp v) const {
    Expression exp = v.exp();
    bool ret = ApplyThis(exp);
    return ret;
  }

  bool ContainsUnknown::operator()(Named v) const {
    Expression exp = v.exp();
    bool ret = ApplyThis(exp);
    return ret;
  }

  bool ContainsUnknown::operator()(Output v) const {
    bool ret = false;
    for (Option<Expression> exp: v.args()) {
      if (exp) {
        bool b = ApplyThis(exp.get());
        ret |= b;
      }
    }
    return ret;
  }


  bool ContainsUnknown::operator()(Reference ref) const {
    int usage = IsUsed(ref);
    if (usage!=-1) {
      usedUnks.insert(usage);
      return true;
    } else {
      return false;
    }
  }

  std::set<int> ContainsUnknown::getUsages() const {
    return usedUnks;
  }

  int ContainsUnknown::IsUsed(Expression e) const {
    std::stringstream ss;
    ss << e;
    try {
      return definedUnks.at(ss.str());
    } catch (const std::out_of_range& oor) {
      return -1;
    }
  }

}




