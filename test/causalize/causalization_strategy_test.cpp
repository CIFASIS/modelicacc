#include <boost/test/unit_test.hpp>
#include <boost/test/included/unit_test.hpp>

#include <causalize/unknowns_collector.h>
#include <causalize/causalization_strategy.h>

#include <parser/parser.h>
#include <ast/ast_types.h>
#include <ast/equation.h>
#include <mmo/mmo_class.h>
#include <util/table.h>
#include <util/debug.h>
#include <util/ast_visitors/contains.h>

#include <boost/variant/get.hpp>

using namespace boost::unit_test;
using namespace Modelica::AST;

void check_causality(MMO_Class &mmoClass, ExpList unknowns) {

  const EquationList &causalEqs = mmoClass.equations_ref().equations_ref();

  /*
   * Here we collect the left side of each
   * equation.
   */
  ExpList knowns;

  foreach_(Equation equation, causalEqs) {

    Equality eqEq = boost::get<Equality>(equation);
    Expression leftExpression = eqEq.left_ref();

    if (is<Reference>(leftExpression)) {
      knowns.push_back(leftExpression);
    } else if (is<Call>(leftExpression)) {
      Call call = boost::get<Call>(leftExpression);
      if (call.name()=="der")
        knowns.push_back(leftExpression);
    } else if (is<Output>(leftExpression)) {
      Output output = boost::get<Output>(leftExpression);
      foreach_(OptExp oe, output.args()) {
        if (oe)
          knowns.push_back(oe.get());
      }
    } else {
      ERROR("Unexpected type for equation's left expression\n");
    }

    foreach_(Expression unknown, unknowns) {
      Modelica::contains occurrs(unknown);
      if (boost::apply_visitor(occurrs, eqEq.right_ref())) {
        bool isKnown = false;
        foreach_(Expression known, knowns) {
          if(known == unknown) {
            isKnown = true;
          }
        }
        BOOST_CHECK(isKnown);
      }
    }

  }

}

void rlc_test() {

  bool r;

  StoredDef sd = parseFile("rlc.mo",r);

  if (!r)
    ERROR("Can't parse file\n");

  Class ast_c = boost::get<Class>(sd.classes().front());
  MMO_Class mmo(ast_c);

  UnknownsCollector collector(mmo);
  ExpList unknowns = collector.collectUnknowns();

  CausalizationStrategy cStrategy(mmo);
  cStrategy.causalize("Anything");

  //std::cout << mmo << std::endl;

  check_causality(mmo, unknowns);

}

void rlc_loop_test() {

  bool r;

  StoredDef sd = parseFile("rlc_loop.mo",r);

  if (!r)
    ERROR("Can't parse file\n");

  Class ast_c = boost::get<Class>(sd.classes().front());
  MMO_Class mmo(ast_c);

  UnknownsCollector collector(mmo);
  ExpList unknowns = collector.collectUnknowns();

  CausalizationStrategy cStrategy(mmo);
  cStrategy.causalize("Anything");

  //std::cout << mmo << std::endl;

  check_causality(mmo, unknowns);

}

void rlc_loop_tarjan_test() {

  bool r;

  StoredDef sd = parseFile("rlc_loop.mo",r);

  if (!r)
    ERROR("Can't parse file\n");

  Class ast_c = boost::get<Class>(sd.classes().front());
  MMO_Class mmo(ast_c);

  UnknownsCollector collector(mmo);
  ExpList unknowns = collector.collectUnknowns();

  CausalizationStrategy cStrategy(mmo);
  cStrategy.causalize_tarjan("Anything");

  //std::cout << mmo << std::endl;

  check_causality(mmo, unknowns);

}

void OneDHeatTransferTI_FD_test() {

  bool r;

  StoredDef sd = parseFile("OneDHeatTransferTI_FD.mo",r);

  if (!r)
    ERROR("Can't parse file\n");

  Class ast_c = boost::get<Class>(sd.classes().front());
  MMO_Class mmo(ast_c);

  UnknownsCollector collector(mmo);
  ExpList unknowns = collector.collectUnknowns();

  CausalizationStrategy cStrategy(mmo);
  cStrategy.causalize("Anything");

  check_causality(mmo, unknowns);

}

void OneDHeatTransferTI_FD_simple_test() {

  bool r;

  StoredDef sd = parseFile("OneDHeatTransferTI_FD.mo",r);

  if (!r)
    ERROR("Can't parse file\n");

  Class ast_c = boost::get<Class>(sd.classes().front());
  MMO_Class mmo(ast_c);

  UnknownsCollector collector(mmo);
  ExpList unknowns = collector.collectUnknowns();

  CausalizationStrategy cStrategy(mmo);
  cStrategy.causalize_simple("Anything");

  check_causality(mmo, unknowns);

}

test_suite*
init_unit_test_suite( int, char* []) {

  //debugInit("c");

  framework::master_test_suite().p_name.value = "Causalization Strategy Test";

  framework::master_test_suite().add( BOOST_TEST_CASE( &rlc_test) );
  framework::master_test_suite().add( BOOST_TEST_CASE( &rlc_loop_test) );
  framework::master_test_suite().add( BOOST_TEST_CASE( &rlc_loop_tarjan_test) );
  framework::master_test_suite().add( BOOST_TEST_CASE( &OneDHeatTransferTI_FD_test) );
  framework::master_test_suite().add( BOOST_TEST_CASE( &OneDHeatTransferTI_FD_simple_test) );

  return 0;
}
