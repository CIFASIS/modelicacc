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

#include <test_derivate/test_derivate.h>
#include <ast/equation.h>
#include <ast/queries.h>
#include <boost/variant/get.hpp>
#include <util/ast_visitors/part_evalexp.h>
#include <util/ast_visitors/replace_eq.h>
#include <util/ast_visitors/replace_st.h>
#include <causalize/state_variables_finder.h>
#include <algorithm>
#include <vector>
#include <util/derivate_equality.h>


namespace Modelica {
TestDerivate::TestDerivate(MMO_Class &c): _c(c) {
  StateVariablesFinder svf(c); 
  svf.findStateVariables();
}
void TestDerivate::derivateAllEqualities() {
  EquationList &el = _c.equations_ref().equations_ref();
  VarSymbolTable &syms = _c.syms_ref();
  foreach_(Equation &e, el) {
    if (is<Equality>(e)) {
      Equality &eq = boost::get<Equality>(e);
      PartEvalExp eval(syms, true);
      Equality d;
      d=derivate_equality(eq, syms);
      std::cerr << "e: " << e << "  der: " << d << " eq: " << eq <<"\n";
      }
    }
}
void TestDerivate::alias(Reference a, Expression b) { // Remove a from the model and replace every occurence with b
  VarSymbolTable &syms = _c.syms_ref();
  syms.remove(refName(a));
  
  // Remove variable a
  std::vector<Name> &vars = _c.variables_ref();
  std::vector<Name>::iterator pos = std::find(vars.begin(),vars.end(), refName(a));
  if (pos!=vars.end())
    vars.erase(pos);
  replace_eq req(a,b);
  foreach_ (Equation &eq, _c.equations_ref().equations_ref()) {
    //std::cerr << eq << " is now ";
    eq=boost::apply_visitor(req,eq);
    //std::cerr << eq << "\n";
  }
  foreach_ (Equation &eq, _c.initial_eqs_ref().equations_ref()) 
    eq=boost::apply_visitor(req,eq);
  replace_st rst(a,b);
  foreach_ (Statement &st, _c.statements_ref().statements_ref()) 
    st=boost::apply_visitor(rst,st);
  foreach_ (Statement &st, _c.initial_sts_ref().statements_ref()) 
    st=boost::apply_visitor(rst,st);

}
};

