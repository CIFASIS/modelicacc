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

#include <string.h>
#include <stdio.h>
#include <sstream>
#include <cassert>
#include <fstream>
#include <set>
#include <algorithm>
#include <ginac/ginac.h>

#include <ast/queries.hpp>
#include <ast/equation.hpp>
#include <ast/expression.hpp>
#include <ast/modification.hpp>
#include <util/ast_visitors/ginac_interface.hpp>
#include <util/ast_visitors/contains_expression.hpp>
#include <util/ast_visitors/partial_eval_expression.hpp>
#include <util/ast_visitors/all_expressions.hpp>
#include <util/ast_visitors/eval_expression.hpp>
#include <util/ast_visitors/replace_equation.hpp>
#include <util/solve/solve.hpp>
#include <util/debug.hpp>
#include <parser/parser.hpp>
#include <boost/variant/get.hpp>

using namespace GiNaC;
void my_print_power_dflt(const power &p, const print_dflt &c, unsigned level)
{
  // get the precedence of the 'power' class
  unsigned power_prec = p.precedence();

  // if the parent operator has the same or a higher precedence
  // we need parentheses around the power
  if (level >= power_prec) c.s << '(';
  if (p.op(1).match(-1)) {
    c.s << "(1/(" << p.op(0) << "))";
  } else {
    c.s << p.op(0) << "^" << p.op(1);
  }
  // don't forget the closing parenthesis
  if (level >= power_prec) c.s << ')';
}

ForEq updateForEquationRange(ForEq for_eq)
{
  Modelica::AST::IndexList for_idxs = for_eq.range().indexes();
  Modelica::AST::Index idx = for_idxs.front();
  Modelica::AST::Range range_exp = get<Modelica::AST::Range>(idx.exp().get());

  Modelica::AST::Integer step_val = get<Modelica::AST::Integer>(range_exp.step().get());
  if (step_val < 0) {
    Modelica::AST::Integer start_val = get<Modelica::AST::Integer>(range_exp.start());
    Modelica::AST::Integer end_val = get<Modelica::AST::Integer>(range_exp.end());

    IndexList new_idxs;
    new_idxs.push_back(Index(idx.name(), OptExp(Range(range_exp.end(), range_exp.start()))));

    Expression replace_idx = Expression(
        BinOp(Expression(UnaryOp(Reference(idx.name()), Minus)), Modelica::AST::BinOpType::Add, Expression(start_val + end_val)));
    Modelica::ReplaceEquation rep_eq(Expression(Reference(idx.name())), replace_idx);

    Modelica::AST::Equation eq = for_eq.elements().front();
    Modelica::AST::Equation new_eq = Apply(rep_eq, eq);

    return ForEq{new_idxs, EquationList{1, new_eq}};
  }
  return for_eq;
}

EquationList EquationSolver::Solve(EquationList eqs, ExpList crs, VarSymbolTable &syms, std::vector<Name> &variables, ClassList &funs,
                                   Modelica::Causalize::TearingVariables tearing_vars)
{
  using namespace std;

  Modelica::ConvertToGiNaC tog(syms);
  Modelica::PartialEvalExpression peval(syms, false);
  Modelica::EvalExpression eval(syms);

  const int size = eqs.size();
  if (size == 1 && is<Equality>(eqs.front())) {  // Trivial solve
    Equality eq = get<Equality>(eqs.front());
    Expression l = Apply(peval, eq.left_ref());
    Expression r = Apply(peval, eq.right_ref());
    if (l == crs.front()) {
      if (!Apply(Modelica::ContainsExpression(crs.front()), eq.right_ref())) return eqs;
    } else if (r == crs.front()) {
      if (!Apply(Modelica::ContainsExpression(crs.front()), eq.left_ref())) {
        return EquationList(1, Equality(r, l));
      }
    }
  }
  GiNaC::lst eqns, vars;
  foreach_(Expression exp, crs)
  {
    if (debugIsEnabled('s')) {
      std::cerr << "Solving variables " << exp << ": GiNaC " << Apply(tog, exp) << "\n";
    }
    vars.append(Apply(tog, exp));
  }

  std::vector<bool> for_eqs;
  std::vector<Modelica::AST::IndexList> for_idxs;

  foreach_(Equation e, eqs)
  {
    if (debugIsEnabled('s')) {
      std::cerr << "Using equation " << e << "\n";
    }
    if (is<ForEq>(e)) {
      ForEq feq = get<ForEq>(e);
      ERROR_UNLESS(is<Equality>(feq.elements().front()), "Trying to solve a for loop with a non suported equation inside");
      for_eqs.emplace_back(true);
      for_idxs.emplace_back(feq.range().indexes());
      Equality eq = get<Equality>(feq.elements().front());
      // Expression l = Apply(peval, eq.left_ref());
      // Expression r = Apply(peval, eq.right_ref());
      Expression l = eq.left_ref();
      Expression r = eq.right_ref();
      GiNaC::ex left = Apply(tog, l);
      GiNaC::ex right = Apply(tog, r);
      if (debugIsEnabled('s')) std::cerr << "GiNaC equation " << left << "=" << right << "\n";
      eqns.append(left == right);
    } else {
      for_eqs.emplace_back(false);
      for_idxs.emplace_back(Modelica::AST::IndexList());
      ERROR_UNLESS(is<Equality>(e), "Solve: Only equality equations are supported\n");
      Equality eq = get<Equality>(e);
      // Expression l = Apply(peval, eq.left_ref());
      // Expression r = Apply(peval, eq.right_ref());
      Expression l = eq.left_ref();
      Expression r = eq.right_ref();
      GiNaC::ex left = Apply(tog, l);
      GiNaC::ex right = Apply(tog, r);
      if (debugIsEnabled('s')) std::cerr << "GiNaC equation " << left << "=" << right << "\n";
      eqns.append(left == right);
    }
  }

  EquationList ret;
  EquationList tearing_res_eqs;
  try {
    if (debugIsEnabled('s')) std::cerr << "GiNaC equations " << eqns << " variables " << vars << "\n";
    GiNaC::ex solved = lsolve(eqns, vars, GiNaC::solve_algo::gauss);
    if (solved.nops() == 0) {
      throw std::logic_error("Ginac Solve Error");
    }
    if (!tearing_vars.empty()) {
      int tearing_var_index = 1;
      for (const auto &tearing_var : tearing_vars) {
        for (unsigned int i = 0; i < solved.nops(); i++) {
          set_print_func<power, print_dflt>(my_print_power_dflt);
          Expression lhs = Modelica::ConvertToExp(solved.op(i).op(0));
          if (is<Reference>(lhs)) {
            Reference l = get<Reference>(lhs);
            Modelica::AST::Bracket first_bracket = *tearing_var.second.begin();
            Modelica::AST::Brace brace_exp = first_bracket.args().front();
            Modelica::AST::Range range_exp = get<Modelica::AST::Range>(brace_exp.args().front());
            if (lhs == Expression(Reference(tearing_var.first, Expression(range_exp.start())))) {
              std::stringstream var_name;
              var_name << "tearing_var_guess_" << tearing_var_index;
              Modelica::AST::Name tearing_var_guess_name = var_name.str();
              var_name.str("");
              var_name << "tearing_var_res_" << tearing_var_index;
              Modelica::AST::Name tearing_var_res_name = var_name.str();
              tearing_var_index++;
              Modelica::AST::Name start_mod = "start";
              ClassModification class_mod;
              class_mod.push_back(ElMod(start_mod, ModEq(Expression(1))));
              syms.insert(tearing_var_guess_name, VarInfo(TypePrefixes(), "Real", Option<Comment>(), Modification(ModClass(class_mod))));
              syms.insert(tearing_var_res_name, VarInfo(TypePrefixes(), "Real", Option<Comment>(), Modification(ModClass(class_mod))));
              variables.push_back(tearing_var_guess_name);
              variables.push_back(tearing_var_res_name);

              Expression rhs = Modelica::ConvertToExp(solved.op(i).op(1));
              Expression guess_exp = Expression(Reference(tearing_var_guess_name));
              Expression res_exp = Expression(Reference(tearing_var_res_name));
              Expression zero_exp = Expression(0);
              Expression res_tearing_var_exp = BinOp(lhs, Modelica::AST::BinOpType::Sub, rhs);
              tearing_res_eqs.push_back(Equality(res_exp, res_tearing_var_exp));
              tearing_res_eqs.push_back(Equality(res_exp, zero_exp));
              ret.push_back(Equality(lhs, guess_exp));
            }
          }
        }
      }
    }

    std::vector<Modelica::AST::Name> tearing_var_names = tearing_vars.variables();
    for (unsigned int i = 0; i < solved.nops(); i++) {
      bool for_eq = for_eqs[i];
      std::stringstream s(ios_base::out);
      set_print_func<power, print_dflt>(my_print_power_dflt);
      if (debugIsEnabled('s')) std::cerr << "GiNaC result " << solved.op(i) << "\n";
      Expression lhs = Modelica::ConvertToExp(solved.op(i).op(0));
      if (is<Reference>(lhs)) {
        Reference l = get<Reference>(lhs);
        if (std::find(tearing_var_names.begin(), tearing_var_names.end(), Modelica::refName(l)) != tearing_var_names.end()) {
          Modelica::AST::Bracket first_bracket = *tearing_vars[Modelica::refName(l)].begin();
          Modelica::AST::Brace brace_exp = first_bracket.args().front();
          Modelica::AST::Range range_exp = get<Modelica::AST::Range>(brace_exp.args().front());
          if (lhs == Expression(Reference(Modelica::refName(l), Expression(range_exp.start())))) {
            continue;
          }
        }
      }
      Expression rhs = Modelica::ConvertToExp(solved.op(i).op(1));

      if (debugIsEnabled('s')) std::cerr << "Modelica result " << lhs << "=" << rhs << " " << for_eq << "\n";
      if (for_eq) {
        ForEq feq = ForEq{for_idxs[i], EquationList{1, Equality(lhs, rhs)}};
        feq = updateForEquationRange(feq);
        ret.push_back(feq);
      } else {
        ret.push_back(Equality(lhs, rhs));
      }
    }
    for (const auto &tearing_eq : tearing_res_eqs) {
      ret.push_back(tearing_eq);
    }
  } catch (std::logic_error &) {
    std::cerr << "EquationSolver: cannot solve equation" << eqns << std::endl;
    std::cerr << "EquationSolver: for variables " << vars << std::endl;
    abort();
  }
  return ret;
}

Equation EquationSolver::Solve(Equation eq, Expression exp, VarSymbolTable &syms, std::vector<Name> &variables, ClassList &cl,
                               Modelica::Causalize::TearingVariables tearing_vars)
{
  return Solve(EquationList(1, eq), ExpList(1, exp), syms, variables, cl, tearing_vars).front();
}
