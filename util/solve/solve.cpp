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
#include <util/ast_visitors/contains_variable.hpp>
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
                                   std::vector<Name> &tearing_aux_vars)
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
      if (!Apply(Modelica::ContainsExpression(crs.front()), eq.right_ref())) {
        return eqs;
      }
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
    if (debugIsEnabled('s')) {
      std::cerr << "GiNaC equations " << eqns << " variables " << vars << "\n";
    }
    GiNaC::ex solved = lsolve(eqns, vars, GiNaC::solve_algo::gauss);
    if (solved.nops() == 0) {
      throw std::logic_error("Ginac Solve Error");
    }

    for (unsigned int i = 0; i < solved.nops(); i++) {
      bool for_eq = for_eqs[i];
      std::stringstream s(ios_base::out);
      set_print_func<power, print_dflt>(my_print_power_dflt);
      if (debugIsEnabled('s')) {
        std::cerr << "GiNaC result " << solved.op(i) << "\n";
      }
      Expression lhs = Modelica::ConvertToExp(solved.op(i).op(0));
      Expression rhs = Modelica::ConvertToExp(solved.op(i).op(1));

      if (debugIsEnabled('s')) {
        std::cerr << "Modelica result " << lhs << "=" << rhs << " " << for_eq << "\n";
      }
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
    std::cerr << "EquationSolver: cannot solve equation " << eqns << std::endl;
    std::cerr << "EquationSolver: for variables " << vars << std::endl;
    abort();
  }
  return ret;
}

bool EquationSolver::containsAnyVar(Equation eq, const std::vector<Name> &var_names)
{
  if (is<ForEq>(eq)) {
    foreach_(Equation eq_entry, get<ForEq>(eq).elements()) if (containsAnyVar(eq_entry, var_names)) return true;
    return false;
  }
  if (!is<Equality>(eq)) return false;
  Equality equality = get<Equality>(eq);
  for (const Name &var_name : var_names) {
    Modelica::ContainsVariable contains_var(var_name);
    if (Apply(contains_var, equality.left_ref()) || Apply(contains_var, equality.right_ref())) return true;
  }
  return false;
}

// Equations without a matched variable are paired with a default (Integer 0) expression.
bool EquationSolver::isUnmatchedVar(Expression var) { return is<Modelica::AST::Integer>(var) && get<Modelica::AST::Integer>(var) == 0; }

void EquationSolver::splitAuxVars(const std::vector<Name> &tearing_aux_vars, std::vector<Name> &guess_vars, std::vector<Name> &res_vars)
{
  for (const Name &aux_var : tearing_aux_vars) {
    if (aux_var.compare(0, Modelica::Causalize::kTearingGuessPrefix.size(), Modelica::Causalize::kTearingGuessPrefix) == 0) {
      guess_vars.push_back(aux_var);
    } else if (aux_var.compare(0, Modelica::Causalize::kTearingResPrefix.size(), Modelica::Causalize::kTearingResPrefix) == 0) {
      res_vars.push_back(aux_var);
    }
  }
}

EquationList EquationSolver::Solve(Equation eq, ExpList vars, VarSymbolTable &syms, std::vector<Name> &variables, ClassList &cl,
                                   std::vector<Name> &tearing_aux_vars)
{
  EquationList eq_list;
  if (is<ForEq>(eq)) {
    ForEq for_eq = get<ForEq>(eq);
    for (Equation eq_entry : for_eq.elements()) {
      eq_list.push_back(ForEq(for_eq.range().indexes(), EquationList(1, eq_entry)));
    }
  } else {
    eq_list.push_back(eq);
  }

  ERROR_UNLESS(eq_list.size() == vars.size(), "Causalize: equations and matched variables sizes differ");

  // Equations without a matched variable (e.g. tearing_var_res_{VAR_NAME} = 0) are not solved and are kept unmodified.
  EquationList matched_eqs, unmatched_eqs;
  ExpList matched_vars;
  for (std::size_t i = 0; i < eq_list.size(); ++i) {
    if (isUnmatchedVar(vars[i])) {
      unmatched_eqs.push_back(eq_list[i]);
    } else {
      matched_eqs.push_back(eq_list[i]);
      matched_vars.push_back(vars[i]);
    }
  }

  EquationList solved;
  if (!matched_eqs.empty()) solved = Solve(matched_eqs, matched_vars, syms, variables, cl, tearing_aux_vars);
  ERROR_UNLESS(solved.size() == matched_eqs.size(), "Causalize: solved equations and matched variables sizes differ");

  std::vector<Name> guess_vars, res_vars;
  splitAuxVars(tearing_aux_vars, guess_vars, res_vars);

  // Solved equations follow the order of matched_eqs. Guess equations ({VAR_NAME} = tearing_var_guess_{VAR_NAME}) are kept as in the
  // original list, every other solved equation is kept, and residual definitions go last. Residual equations may also
  // reference guess variables, so they are checked first.
  EquationList guess_eqs, res_eqs, rest_eqs;
  for (std::size_t i = 0; i < matched_eqs.size(); ++i) {
    if (containsAnyVar(matched_eqs[i], res_vars)) {
      res_eqs.push_back(solved[i]);
    } else if (containsAnyVar(matched_eqs[i], guess_vars)) {
      guess_eqs.push_back(matched_eqs[i]);
    } else {
      rest_eqs.push_back(solved[i]);
    }
  }

  EquationList ret;
  ret.insert(ret.end(), guess_eqs.begin(), guess_eqs.end());
  ret.insert(ret.end(), rest_eqs.begin(), rest_eqs.end());
  ret.insert(ret.end(), res_eqs.begin(), res_eqs.end());
  ret.insert(ret.end(), unmatched_eqs.begin(), unmatched_eqs.end());
  return ret;
}

// Merges consecutive for equations with the same range into a single one.
EquationList EquationSolver::groupForEquations(const EquationList &eqs)
{
  EquationList grouped;
  std::string last_range;
  for (const Equation &eq : eqs) {
    if (!is<ForEq>(eq)) {
      grouped.push_back(eq);
      continue;
    }

    const ForEq &for_eq = get<ForEq>(eq);
    std::ostringstream range;
    range << for_eq.range();
    if (!grouped.empty() && is<ForEq>(grouped.back()) && range.str() == last_range) {
      EquationList &elements = get<ForEq>(grouped.back()).elements_ref();
      EquationList new_elements = for_eq.elements();
      elements.insert(elements.end(), new_elements.begin(), new_elements.end());
    } else {
      grouped.push_back(eq);
      last_range = range.str();
    }
  }

  return grouped;
}

EquationList EquationSolver::Solve(const Modelica::Causalize::CausalModel &causal_model, VarSymbolTable &syms, std::vector<Name> &variables,
                                   ClassList &cl, std::vector<Name> &tearing_aux_vars)
{
  EquationList causalized;
  for (const Modelica::Causalize::SortedAlgebraicLoop &loop : causal_model) {
    for (const Modelica::Causalize::EqVarMatch &match : loop.matchs()) {
      EquationList res = Solve(match.equation(), match.variables(), syms, variables, cl, tearing_aux_vars);
      causalized.insert(causalized.end(), res.begin(), res.end());
    }
  }

  return groupForEquations(causalized);
}
