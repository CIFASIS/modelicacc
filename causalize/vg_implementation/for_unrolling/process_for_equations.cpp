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

#include <math.h>
#include <boost/variant/variant.hpp>
#include <boost/variant/get.hpp>

#include <util/debug.h>
#include <util/table.h>
#include <util/ast_visitors/partial_eval_expression.h>

#include <causalize/vg_implementation/for_unrolling/process_for_equations.h>
#include <causalize/common/for_unrolling/for_index_iterator.h>

namespace Causalize {

Equation instantiate_equation(Equation innerEq, std::list<Name> variables, std::list<int> indexes, VarSymbolTable &symbolTable) {
  VarSymbolTable v=symbolTable;
  ERROR_UNLESS(variables.size()==indexes.size(), "Mismatch size of variables and indexes size");
  std::list<Name>::iterator varsIter = variables.begin();
  foreach_(int i, indexes) {
    VarInfo vinfo = VarInfo(TypePrefixes(1,parameter), "Integer", Option<Comment>(), Modification(ModEq(Expression(i))));
    v.insert(*varsIter,vinfo);
    varsIter++;
  }
  if (is<Equality>(innerEq)) {
    Equality eqeq = boost::get<Equality>(innerEq);
    Expression l=eqeq.left(), r=eqeq.right();
    //std::cout << "Left= " << l << " right " << r << std::endl;
    return Equality(Apply(Modelica::PartialEvalExpression(v),l),Apply(Modelica::PartialEvalExpression(v),r));
  } else {
      ERROR("process_for_equations - instantiate_equation:\n"
            "Incorrect equation type or not supported yet.\n");
  }
  return Equation();
}

void process_for_equations(Modelica::MMO_Class &mmo_class) {
  EquationList &equations = mmo_class.equations_ref().equations_ref();
  EquationList new_equations;
  foreach_ (Equation &e, equations) {
    if (is<ForEq>(e)) {
      ForEq feq = boost::get<ForEq>(e);
      IndexList il = feq.range().indexes();
      ERROR_UNLESS(il.size() <= 3,
          "process_for_equations:\n"
          "forIndexList with more than 3 forIndex are not supported yet\n");
      EquationList processedEqs;
      switch (il.size()) {
      case 1:
        processedEqs = process_for_eq_1d(feq, mmo_class.syms_ref());
        break;
      case 2:
        processedEqs = process_for_eq_2d(feq, mmo_class.syms_ref());
        break;
      case 3:
        processedEqs = process_for_eq_3d(feq, mmo_class.syms_ref());
        break;
      }
      new_equations.insert(new_equations.end(), processedEqs.begin(), processedEqs.end());
    } else {
      // Not a for eq
      new_equations.push_back(e);
    }

  }
  mmo_class.equations_ref().equations_ref()=new_equations;
}

EquationList process_for_eq_1d(ForEq feq, VarSymbolTable &symbolTable) {
  IndexList il = feq.range().indexes();
  Index in = il.front();
  Name variable  = in.name();
  OptExp ind = in.exp();
  if (!ind)
    ERROR("for-equation's index with implicit range not supported yet\n");
  Expression exp = ind.get();
  ForIndexIterator *forIndexIter = NULL;
  if (is<Range>(exp)) {
    forIndexIter = new RangeIterator(get<Range>(exp), symbolTable);
  } else if (is<Brace>(exp)) {
    forIndexIter = new BraceIterator(get<Brace>(exp), symbolTable);
  } else {
    ERROR("For Iterator not supported");
  }
  EquationList processed;
  while (forIndexIter->hasNext()) {
    Real index_val = forIndexIter->next();
    foreach_ (Equation eq, feq.elements())
      processed.push_back(instantiate_equation(eq, std::list<Name>(1,variable), std::list<int>(1,index_val), symbolTable));
  }
  delete forIndexIter;
  return processed;
}

EquationList process_for_eq_2d(ForEq feq, VarSymbolTable &symbolTable) {
  IndexList il = feq.range().indexes();
  Index in1 = il[0];
  Index in2 = il[1];
  Name variable1  = in1.name();
  Name variable2  = in2.name();
  OptExp ind1 = in1.exp();
  OptExp ind2 = in2.exp();
  if (!ind1||!ind2)
    ERROR("for-equation's index with implicit range not supported yet\n");
  Expression exp1 = ind1.get();
  Expression exp2 = ind2.get();
  ForIndexIterator *forIndexIter1 = NULL;
  ForIndexIterator *forIndexIter2 = NULL;
  if (is<Range>(exp1)) {
    forIndexIter1 = new RangeIterator(get<Range>(exp1), symbolTable);
  } else if (is<Brace>(exp1)) {
    forIndexIter1 = new BraceIterator(get<Brace>(exp1), symbolTable);
  } else {
    ERROR("For Iterator not supported");
  }
  EquationList processed;
  while (forIndexIter1->hasNext()) {
    int index_val1 = forIndexIter1->next();
    if (is<Range>(exp2)) {
      forIndexIter2 = new RangeIterator(get<Range>(exp2), symbolTable);
    } else if (is<Brace>(exp2)) {
      forIndexIter2 = new BraceIterator(get<Brace>(exp2), symbolTable);
    } else {
      ERROR("For Iterator not supported");
    }
    while (forIndexIter2->hasNext()) {
      int index_val2 = forIndexIter2->next();
      foreach_ (Equation eq, feq.elements())
        processed.push_back(instantiate_equation(eq, {variable1, variable2}, {index_val1, index_val2}, symbolTable));
    }
  }
  delete forIndexIter1;
  delete forIndexIter2;
  return processed;
}

EquationList process_for_eq_3d(ForEq feq, VarSymbolTable &symbolTable) {
  IndexList il = feq.range().indexes();
  Index in1 = il[0];
  Index in2 = il[1];
  Index in3 = il[2];
  Name variable1  = in1.name();
  Name variable2  = in2.name();
  Name variable3  = in3.name();
  OptExp ind1 = in1.exp();
  OptExp ind2 = in2.exp();
  OptExp ind3 = in3.exp();
  if (!ind1||!ind2||!ind3)
    ERROR("for-equation's index with implicit range not supported yet\n");
  Expression exp1 = ind1.get();
  Expression exp2 = ind2.get();
  Expression exp3 = ind3.get();
  ForIndexIterator *forIndexIter1 = NULL;
  ForIndexIterator *forIndexIter2 = NULL;
  ForIndexIterator *forIndexIter3 = NULL;
  if (is<Range>(exp1)) {
    forIndexIter1 = new RangeIterator(get<Range>(exp1), symbolTable);
  } else if (is<Brace>(exp1)) {
    forIndexIter1 = new BraceIterator(get<Brace>(exp1), symbolTable);
  } else {
    ERROR("For Iterator not supported");
  }
  EquationList processed;
  while (forIndexIter1->hasNext()) {
    int index_val1 = forIndexIter1->next();
    if (is<Range>(exp2)) {
      forIndexIter2 = new RangeIterator(get<Range>(exp2), symbolTable);
    } else if (is<Brace>(exp2)) {
      forIndexIter2 = new BraceIterator(get<Brace>(exp2), symbolTable);
    } else {
      ERROR("For Iterator not supported");
    }
    while (forIndexIter2->hasNext()) {
      int index_val2 = forIndexIter2->next();
      if (is<Range>(exp3)) {
        forIndexIter3 = new RangeIterator(get<Range>(exp3), symbolTable);
      } else if (is<Brace>(exp3)) {
        forIndexIter3 = new BraceIterator(get<Brace>(exp3), symbolTable);
      } else {
        ERROR("For Iterator not supported");
      }
      while (forIndexIter3->hasNext()) {
        int index_val3 = forIndexIter3->next();
        foreach_ (Equation eq, feq.elements())
        processed.push_back(instantiate_equation(eq, {variable1, variable2, variable3}, {index_val1, index_val2, index_val3}, symbolTable));
      }
    }
  }
  delete forIndexIter1;
  delete forIndexIter2;
  delete forIndexIter3;
  return processed;
}

}




