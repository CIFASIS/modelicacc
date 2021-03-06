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

#include <ast/queries.h>
#include <ast/equation.h>
#include <util/ast_visitors/ginac_interface.h>
#include <util/ast_visitors/contains_expression.h>
#include <util/ast_visitors/partial_eval_expression.h>
#include <util/ast_visitors/all_expressions.h>
#include <util/ast_visitors/eval_expression.h>
#include <util/ast_visitors/replace_expression.h>
#include <util/solve/solve.h>
#include <util/debug.h>
#include <parser/parser.h>
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

EquationList EquationSolver::Solve(EquationList eqs, ExpList crs, VarSymbolTable &syms, std::list<std::string> &c_code, ClassList &funs,
                                   const std::string path)
{
  using namespace std;
  static int fsolve = 1;
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
    if (debugIsEnabled('s')) std::cerr << "Solving variables " << exp << ": GiNaC " << Apply(tog, exp) << "\n";
    vars.append(Apply(tog, exp));
  }
  bool for_eq = false;
  foreach_(Equation e, eqs)
  {
    if (debugIsEnabled('s')) std::cerr << "Using equation " << e << "\n";
    if (is<ForEq>(e)) {
      ForEq feq = get<ForEq>(e);
      ERROR_UNLESS(is<Equality>(feq.elements().front()), "Trying to solve a for loop with a non suported equation inside");
      for_eq = true;
      Equality eq = get<Equality>(feq.elements().front());
      Expression l = Apply(peval, eq.left_ref());
      Expression r = Apply(peval, eq.right_ref());
      GiNaC::ex left = Apply(tog, l);
      GiNaC::ex right = Apply(tog, r);
      if (debugIsEnabled('s')) std::cerr << "GiNaC equation " << left << "=" << right << "\n";
      eqns.append(left == right);
    } else {
      ERROR_UNLESS(is<Equality>(e), "Solve: Only equality equations are supported\n");
      Equality eq = get<Equality>(e);
      Expression l = Apply(peval, eq.left_ref());
      Expression r = Apply(peval, eq.right_ref());
      /*Expression l=eq.left_ref();
      Expression r=eq.right_ref();*/
      GiNaC::ex left = Apply(tog, l);
      GiNaC::ex right = Apply(tog, r);
      if (debugIsEnabled('s')) std::cerr << "GiNaC equation " << left << "=" << right << "\n";
      eqns.append(left == right);
    }
  }

  EquationList ret;
  try {
    /*if (size>1)
      throw std::logic_error("Blahh");*/
    if (debugIsEnabled('s')) std::cerr << "GiNaC equations " << eqns << " variables " << vars << "\n";
    GiNaC::ex solved = lsolve(eqns, vars, GiNaC::solve_algo::gauss);
    if (solved.nops() == 0) {
      std::cerr << "EquationSolver: cannot solve equation" << eqns << std::endl;
      std::cerr << "EquationSolver: for variables " << vars << std::endl;
      abort();
    }
    for (unsigned int i = 0; i < solved.nops(); i++) {
      std::stringstream s(ios_base::out);
      set_print_func<power, print_dflt>(my_print_power_dflt);
      if (debugIsEnabled('s')) std::cerr << "GiNaC result " << solved.op(i) << "\n";
      Expression lhs = Modelica::ConvertToExp(solved.op(i).op(0));
      Expression rhs = Modelica::ConvertToExp(solved.op(i).op(1));
      if (debugIsEnabled('s')) std::cerr << "Modelica result " << lhs << "=" << rhs << "\n";
      if (for_eq) {
        ForEq feq = get<ForEq>(eqs.front());
        feq.elements_ref().front() = Equality(lhs, rhs);
        ret.push_back(feq);
      } else {
        ret.push_back(Equality(lhs, rhs));
      }
    }
  } catch (std::logic_error &) {
    //~ ERROR_UNLESS(!for_eq, "Non linear solving of for loops not suported yet");
    OptExpList ol, crs_copy;
    std::vector<Reference> args;
    foreach_(Expression exp, crs)  // TODO (tener cuidado en el crs)
        ol.push_back(exp),
        crs_copy.push_back(exp);
    std::stringstream fun_name;
    fun_name << "fsolve" << fsolve++;
    EquationList loop;
    foreach_(VarSymbolTable::table_type::value_type val, syms)
    {
      if (val.second.builtin()) continue;
      if (Modelica::isParameter(val.first, syms) || Modelica::isConstant(val.first, syms)) continue;
      if (crs.end() != std::find(crs.begin(), crs.end(), Expression(Reference(val.first)))) continue;
      VarInfo vinfo = val.second;
      //~ if (vinfo.indices() && vinfo.indices().get().size() > 1 ) {
      //~ ERROR("Multidimensional arrays not supported yet");
      //~ }
      if (vinfo.indices()) {
        //~ Apply(eval,vinfo.indices().get().front());

        //~ ExpList expi = vinfo.indices().get();
        //~ for (auto exp1 : expi)
        //~ Apply(eval, exp1);

        Reference var(val.first, vinfo.indices().get().front());
        Modelica::AllExpressions all(var);  // Este es el visitor que hay que modificar para que me devuelva todas las expresiones.
        foreach_(Equation & e, eqs)
        {
          Equality eq;
          if (is<ForEq>(e)) {
            ForEq feq = get<ForEq>(e);
            ERROR_UNLESS(is<Equality>(feq.elements().front()), "Algebraic loop including non-equality equations not supported");
            eq = get<Equality>(feq.elements().front());
          } else {
            ERROR_UNLESS(is<Equality>(e), "Algebraic loop including non-equality equations not supported");
            eq = get<Equality>(e);
          }
          Expression pev = Apply(peval, eq.left_ref());
          auto rta = Apply(all, pev);
          pev = Apply(peval, eq.right_ref());
          auto aux = Apply(all, pev);
          rta.insert(rta.end(), aux.begin(), aux.end());

          for (Expression exp : rta) {
            if (crs_copy.end() == std::find(crs_copy.begin(), crs_copy.end(), Expression(exp))) {
              crs_copy.push_back(Expression(exp));
              if (is<Reference>(exp)) {
                Reference ref = get<Reference>(exp);
                args.push_back(ref);
              }
            }
          }
        }
      } else {
        Modelica::AllExpressions all(Reference(val.first));
        foreach_(Equation & e, eqs)
        {
          Equality eq;
          if (is<ForEq>(e)) {
            ForEq feq = get<ForEq>(e);
            ERROR_UNLESS(is<Equality>(feq.elements().front()), "Algebraic loop including non-equality equations not supported");
            eq = get<Equality>(feq.elements().front());
          } else {
            ERROR_UNLESS(is<Equality>(e), "Algebraic loop including non-equality equations not supported");
            eq = get<Equality>(e);
          }
          Expression pev = Apply(peval, eq.left_ref());
          auto rta = Apply(all, pev);
          pev = Apply(peval, eq.right_ref());
          auto aux = Apply(all, pev);
          rta.insert(rta.end(), aux.begin(), aux.end());
          for (Expression exp : rta) {
            if (crs_copy.end() == std::find(crs_copy.begin(), crs_copy.end(), Expression(exp))) {
              crs_copy.push_back(Expression(exp));
              if (is<Reference>(exp)) {
                Reference ref = get<Reference>(exp);
                args.push_back(ref);
              }
            }
          }
        }
      }
    }
    int i;
    Class c;
    c.name_ref() = fun_name.str();
    Composition com;
    External ext;
    std::vector<Reference> c_args;
    std::vector<Expression> c_crs;

    i = 0;
    foreach_(Reference n, args)
    {
      std::stringstream arg_name;
      arg_name << "u_" << i;
      com.elements_ref().push_back(Component(TypePrefixes(1, input), "Real", Option<ExpList>(), DeclList(1, Declaration(arg_name.str()))));
      ext.args_ref().push_back(Expression(Reference(arg_name.str())));
      c_args.push_back(Reference(arg_name.str()));
      i++;
    }
    i = 0;
    foreach_(Expression e, crs)
    {
      std::stringstream arg_name;
      arg_name << "y_" << i;
      com.elements_ref().push_back(Component(TypePrefixes(1, output), "Real", Option<ExpList>(), DeclList(1, Declaration(arg_name.str()))));
      if (crs.size() > 1) {
        ext.args_ref().push_back(Expression(Reference(arg_name.str())));
      }
      c_crs.push_back(Expression(Reference(arg_name.str())));
      i++;
    }
    std::ostringstream code;
    setCFlag(code, 1);
    code << "int " << fun_name.str() << "_eval(const gsl_vector * __x, void * __p, gsl_vector * __f) {\n";
    code << "  double *args=(double*)__p;\n";
    i = 0;
    foreach_(Expression e, c_crs) { code << "  const double " << e << " = gsl_vector_get(__x," << i++ << ");\n"; }
    i = 0;
    foreach_(Reference n, c_args) { code << "  const double " << n << " = args[" << i++ << "];\n"; }
    i = 0;
    foreach_(Equation & e, eqs)
    {
      Equality eq;
      if (is<ForEq>(e)) {
        ForEq feq = get<ForEq>(e);
        ERROR_UNLESS(is<Equality>(feq.elements().front()), "Algebraic loop including non-equality equations not supported");
        eq = get<Equality>(feq.elements().front());
      } else {
        ERROR_UNLESS(is<Equality>(e), "Algebraic loop including non-equality equations not supported");
        eq = get<Equality>(e);
        // loop.push_back(Equality(Apply(peval,eq.left_ref()), Apply(peval,eq.right_ref())));
      }
      Expression l = Apply(peval, eq.left_ref());
      Expression r = Apply(peval, eq.right_ref());
      int j = 0;
      for (Expression e : crs) {
        Modelica::ReplaceExpression repl(e, c_crs[j++]);
        l = Apply(repl, l);
        r = Apply(repl, r);
      }
      j = 0;
      for (Reference ref : args) {
        Modelica::ReplaceExpression repl(Expression(ref), Expression(c_args[j++]));
        l = Apply(repl, l);
        r = Apply(repl, r);
      }

      code << "  gsl_vector_set (__f," << i++ << ", (" << l << ") - (" << r << "));\n";
    }
    code << "  return GSL_SUCCESS;\n";
    code << "}\n";
    if (c_crs.size() > 1)
      code << "void " << fun_name.str() << "(";
    else
      code << "double " << fun_name.str() << "(";
    i = 0;
    foreach_(Reference n, c_args) { code << (++i > 1 ? "," : "") << "double " << n; }
    i = 0;
    if (c_crs.size() > 1) {
      if (c_args.size()) code << ",";
      foreach_(Expression e, c_crs)
      {
        code << "double *" << e;
        if (++i < (int)c_crs.size()) code << ",";
      }
    }
    code << ") { \n";
    code << "  size_t __iter = 0;\n  int __status,i;\n  const gsl_multiroot_fsolver_type *__T = gsl_multiroot_fsolver_hybrid;\n";
    code << "  gsl_multiroot_fsolver *__s = gsl_multiroot_fsolver_alloc (__T, " << eqs.size() << ");\n  gsl_multiroot_function __F;\n";
    code << "  static gsl_vector *__x = NULL;\n";
    code << "  if (__x==NULL) {\n";
    code << "    __x=gsl_vector_alloc(" << eqs.size() << ");\n";
    code << "    for (i=0;i<" << eqs.size() << ";i++)\n";
    code << "      gsl_vector_set (__x, i,0);\n";
    i = 0;
    foreach_(Expression e, crs)
    {  // Check if the variable has a start value
      if (is<Reference>(e)) {
        Reference ref = get<Reference>(e);
        if (ref.ref().size() > 1) {
          ERROR("Solving variables with dot notation not supported");
        }
        RefTuple rt = ref.ref().front();
        if (get<1>(rt).size() > 0) {
          WARNING("Looking for initial value of indexes expression not supported");
          continue;
        }
        Name name = get<0>(rt);
        Option<VarInfo> opt_vinfo = syms[name];
        if (opt_vinfo && opt_vinfo.get().modification() && is<ModClass>(opt_vinfo.get().modification().get())) {
          ClassModification mod = get<ModClass>(opt_vinfo.get().modification().get()).modification();
          foreach_(Argument a, mod)
          {
            if (is<ElMod>(a) && get<ElMod>(a).name() == "start" && get<ElMod>(a).modification() &&
                is<ModEq>(get<ElMod>(a).modification().get())) {
              Expression exp_mod = get<ModEq>(get<ElMod>(a).modification().get()).exp();
              Real r = Apply(eval, exp_mod);
              code << "    gsl_vector_set (__x, " << i << "," << r << ");\n";
            }
          }
        } else if (!opt_vinfo) {
          ERROR("No information for variable %s", name.c_str());
        }
      }
      i++;
    }
    code << "  }\n";
    code << "  __F.n = " << eqs.size() << ";\n";
    code << "  __F.f = " << fun_name.str() << "_eval;\n";
    code << "  double __args[" << args.size() << "];\n";
    i = 0;
    foreach_(Reference n, c_args) { code << "  __args[" << i++ << "] = " << n << ";\n"; }
    code << "   __F.params  = __args;\n";
    code << "  gsl_vector *__f = gsl_vector_alloc(" << eqs.size() << ");\n";
    code << "   // Try if we are already in the solution from the start (useful for discrete dependendt loops) \n";
    code << "   " << fun_name.str() << "_eval(__x, (void*)__args, __f) ;\n";
    code << "   if (gsl_multiroot_test_residual(__f, 1e-7)==GSL_SUCCESS) {\n";
    code << "       gsl_vector_free(__f);\n";
    code << "       gsl_multiroot_fsolver_free (__s);\n";
    if (c_crs.size() == 1) {
      code << "       return gsl_vector_get (__x, 0 );\n";
    } else {
      i = 0;
      foreach_(Expression e, c_crs)
      {
        code << "       " << e << "[0] = "
             << "gsl_vector_get(__x," << i << ");\n";
        i++;
      }
    }
    code << "   }\n";
    code << "   gsl_vector_free(__f);\n";
    code << "   gsl_multiroot_fsolver_set (__s, &__F,__x);\n";
    code << "   do {\n";
    code << "     __iter++;\n";
    code << "     __status = gsl_multiroot_fsolver_iterate (__s);\n";
    code << "     if (__status)   /* check if solver is stuck */\n";
    code << "       break;\n";
    code << "     __status = gsl_multiroot_test_residual (__s->f, 1e-7);\n";
    code << "   } while (__status == GSL_CONTINUE && __iter < 100);\n";
    code << "   if (__iter == 100) printf(\"Warning: GSL could not solve an algebraic loop after %d iterations\\n\",(int) __iter); \n";
    i = 0;
    if (c_crs.size() > 1) {
      foreach_(Expression e, c_crs)
      {
        code << "  " << e << "[0] = "
             << "gsl_vector_get(__s->x," << i << ");\n";
        code << "  gsl_vector_set (__x, " << i << " , gsl_vector_get(__s->x, " << i << "));\n";
        i++;
      }
      code << "   gsl_multiroot_fsolver_free (__s);\n";
    }
    if (c_crs.size() == 1) {
      code << "  "
           << "double ret = "
           << "gsl_vector_get(__s->x,0);\n";
      code << "  gsl_vector_set (__x, 0 , ret);\n";
      code << "  gsl_multiroot_fsolver_free (__s);\n";
      code << "  return ret;\n";
    }
    code << "}\n";
    c_code.push_back(code.str());

    ExpList exp_args(args.begin(), args.end());
    i = 0;
    if (for_eq) {
      ForEq feq = get<ForEq>(eqs.front());
      if (crs.size() > 1)
        feq.elements_ref().front() = (Equality(Output(ol), Call(fun_name.str(), exp_args)));
      else
        feq.elements_ref().front() = (Equality(crs.front(), Call(fun_name.str(), exp_args)));
      ret.push_back(feq);
    } else {
      if (crs.size() > 1)
        ret.push_back(Equality(Output(ol), Call(fun_name.str(), exp_args)));
      else
        ret.push_back(Equality(crs.front(), Call(fun_name.str(), exp_args)));
    }
    if (crs.size() == 1) ext.comp_ref_ref() = Expression(Reference("y_0"));
    c.prefixes_ref() = ClassPrefixes(1, Modelica::function);
    com.language_ref() = String("C");
    ext.fun_ref() = fun_name.str();
    com.call_ref() = ext;
    com.external_ref() = true;
    ExpList el;
    el.push_back(String("m"));
    el.push_back(String("gsl"));
    el.push_back(String("blas"));
    Annotation ext_anot(ClassModification(Argument(ElMod("Library", ModEq(Brace(el)))),
                                          Argument(ElMod("Include", ModEq(String("#include \\\"" + path + "\\\""))))));
    com.ext_annot_ref() = ext_anot;
    c.composition_ref() = com;
    funs.push_back(c);
  }
  return ret;
}

Equation EquationSolver::Solve(Equation eq, Expression exp, VarSymbolTable &syms, std::list<std::string> &c_code, ClassList &cl,
                               const std::string path)
{
  return Solve(EquationList(1, eq), ExpList(1, exp), syms, c_code, cl, path).front();
}
