#include <iostream>

#include <boost/test/unit_test.hpp>
#include <boost/test/included/unit_test.hpp>

#include <parser/parser.h>
#include <ast/expression.h>
#include <ast/class.h>
#include <mmo/mmo_class.h>
#include <util/table.h>
#include <causalize/graph_implementation/for_unrolling/process_for_equations.h>
#include <boost/variant/variant.hpp>
#include <boost/variant/get.hpp>

using namespace boost::unit_test;
using namespace boost;
using namespace std;
using namespace Modelica;
using namespace Modelica::AST;

#define DEBUG true

//____________________________________________________________________________//

void unrolling_test_1()
{
  bool r;
  StoredDef sd = parseFile("for_example_1.mo", r);

  if (!r) {
    cout << "Couldn't open for_example.mo file" << endl;
    return;
  }

  Class ast_c = boost::get<Class>(sd.classes().front());

  // TypeSymbolTable tyEnv = newTypeSymbolTable();
  MMO_Class c(ast_c);

  int equationsBefore = c.equations().equations().size();

  if (DEBUG) {
    cout << "Numero de ecuaciones antes: " << equationsBefore << endl;
    cout << c << endl;
  }

  Causalize::process_for_equations(c);

  int equationsAfter = c.equations().equations().size();

  if (DEBUG) {
    cout << "Numero de ecuaciones despues: " << equationsAfter << endl;
    cout << c << endl;
  }

  BOOST_CHECK(equationsAfter == 10);
  int i = 1;
  foreach_(Equation eq, c.equations().equations())
  {
    BOOST_CHECK(is<Equality>(eq));
    Equality eqEq = get<Equality>(eq);
    Expression expLeft = eqEq.left();
    BOOST_CHECK(is<Reference>(expLeft));
    Reference array = get<Reference>(expLeft);
    BOOST_CHECK(array.ref().size() == 1);
    ExpList indexes = get<1>(array.ref().front());
    BOOST_CHECK(indexes.size() == 1);
    Expression index = indexes.front();
    BOOST_CHECK(get<Modelica::AST::Real>(index) || get<Modelica::AST::Integer>(index));
    if (is<Real>(index)) {
      BOOST_CHECK((int)get<Modelica::AST::Real>(index) == i);
    } else {
      BOOST_CHECK((int)get<Modelica::AST::Integer>(index) == i);
    }
    // Falta hacer lo mismo para la exp de la derecha y para las otras funcs
    /*
    AST_Equation_Equality eqEq = eq->getAsEquality();
    AST_Expression expLeft = eqEq->left();
    BOOST_CHECK(expLeft->expressionType() == EXPCOMPREF);
    AST_Expression_ComponentReference array = expLeft->getAsComponentReference();
    BOOST_CHECK(array->indexes()->size() == 1);
    AST_ExpressionList indexes = array->indexes()->front();
    BOOST_CHECK(indexes->size() == 1);
    AST_Expression index = indexes->front();
    BOOST_CHECK(index->expressionType() == EXPREAL || index->expressionType() == EXPINTEGER);
    if (index->expressionType() == EXPREAL) {
      BOOST_CHECK((int)index->getAsReal()->val() == i);
    } else {
      BOOST_CHECK(index->getAsInteger()->val() == i);
    }
    AST_Expression expRight = eqEq->right();
    BOOST_CHECK(expRight->expressionType() == EXPREAL || expRight->expressionType() == EXPINTEGER);
    if (expRight->expressionType() == EXPREAL) {
      BOOST_CHECK(expRight->getAsReal()->val() == i);
    } else {
      BOOST_CHECK(expRight->getAsInteger()->val() == i);
    }
    i++;
  */
  }
}

void unrolling_test_2()
{
  /*
  int r;
  AST_Class ast_c = parseClass("for_example_2.mo",&r);
  if (r!=0) {
    cout << "Couldn't open for_example_2.mo file" << endl;
    return;
  }

  TypeSymbolTable tyEnv = newTypeSymbolTable();
  MMO_Class c = newMMO_Class(ast_c, tyEnv);

  int equationsBefore = c->getEquations()->size();

  if (DEBUG) {
    cout << "Numero de ecuaciones antes: " << equationsBefore << endl;
    cout << c << endl;
  }

  process_for_equations(c);

  int equationsAfter = c->getEquations()->size();

  if (DEBUG) {
    cout << "Numero de ecuaciones despues: " << equationsAfter << endl;
    cout << c << endl;
  }

  BOOST_CHECK(equationsAfter == 10);
  MMO_EquationList equations = c->getEquations();
  MMO_EquationListIterator eqsIter = equations->begin();
  int i=11;
  foreach(eqsIter, equations) {
    MMO_Equation eq = current_element(eqsIter);
    BOOST_CHECK(eq->equationType() == EQEQUALITY);
    AST_Equation_Equality eqEq = eq->getAsEquality();
    AST_Expression expLeft = eqEq->left();
    BOOST_CHECK(expLeft->expressionType() == EXPCOMPREF);
    AST_Expression_ComponentReference array = expLeft->getAsComponentReference();
    BOOST_CHECK(array->indexes()->size() == 1);
    AST_ExpressionList indexes = array->indexes()->front();
    BOOST_CHECK(indexes->size() == 1);
    AST_Expression index = indexes->front();
    BOOST_CHECK(index->expressionType() == EXPREAL || index->expressionType() == EXPINTEGER);
    if (index->expressionType() == EXPREAL) {
      BOOST_CHECK((int)index->getAsReal()->val() == i);
    } else {
      BOOST_CHECK(index->getAsInteger()->val() == i);
    }
    AST_Expression expRight = eqEq->right();
    BOOST_CHECK(expRight->expressionType() == EXPREAL || expRight->expressionType() == EXPINTEGER);
    if (expRight->expressionType() == EXPREAL) {
      BOOST_CHECK(expRight->getAsReal()->val() == i);
    } else {
      BOOST_CHECK(expRight->getAsInteger()->val() == i);
    }
    i++;
  }
  */
}

void unrolling_test_3()
{
  /*
  int r;
  AST_Class ast_c = parseClass("for_example_3.mo",&r);
  if (r!=0) {
    cout << "Couldn't open for_example_3.mo file" << endl;
    return;
  }

  TypeSymbolTable tyEnv = newTypeSymbolTable();
  MMO_Class c = newMMO_Class(ast_c, tyEnv);

  int equationsBefore = c->getEquations()->size();

  if (DEBUG) {
    cout << "Numero de ecuaciones antes: " << equationsBefore << endl;
    cout << c << endl;
  }

  process_for_equations(c);

  int equationsAfter = c->getEquations()->size();

  if (DEBUG) {
    cout << "Numero de ecuaciones despues: " << equationsAfter << endl;
    cout << c << endl;
  }

  BOOST_CHECK(equationsAfter == 10);
  MMO_EquationList equations = c->getEquations();
  MMO_EquationListIterator eqsIter = equations->begin();
  int i=1;
  foreach(eqsIter, equations) {
    MMO_Equation eq = current_element(eqsIter);
    BOOST_CHECK(eq->equationType() == EQEQUALITY);
    AST_Equation_Equality eqEq = eq->getAsEquality();
    AST_Expression expLeft = eqEq->left();
    BOOST_CHECK(expLeft->expressionType() == EXPCOMPREF);
    AST_Expression_ComponentReference array = expLeft->getAsComponentReference();
    BOOST_CHECK(array->indexes()->size() == 1);
    AST_ExpressionList indexes = array->indexes()->front();
    BOOST_CHECK(indexes->size() == 1);
    AST_Expression index = indexes->front();
    BOOST_CHECK(index->expressionType() == EXPREAL || index->expressionType() == EXPINTEGER);
    if (index->expressionType() == EXPREAL) {
      BOOST_CHECK((int)index->getAsReal()->val() == i+1);
    } else {
      BOOST_CHECK(index->getAsInteger()->val() == i+1);
    }
    AST_Expression expRight = eqEq->right();
    BOOST_CHECK(expRight->expressionType() == EXPREAL || expRight->expressionType() == EXPINTEGER);
    if (expRight->expressionType() == EXPREAL) {
      BOOST_CHECK(expRight->getAsReal()->val() == i+1);
    } else {
      BOOST_CHECK(expRight->getAsInteger()->val() == i+1);
    }
    i++;
  }
  */
}

//____________________________________________________________________________//

test_suite* init_unit_test_suite(int, char*[])
{
  framework::master_test_suite().p_name.value = "For unrolling";

  framework::master_test_suite().add(BOOST_TEST_CASE(&unrolling_test_1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&unrolling_test_2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&unrolling_test_3));

  return 0;
}

//____________________________________________________________________________//

// EOF
