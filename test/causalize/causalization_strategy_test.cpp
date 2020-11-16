#include <boost/test/unit_test.hpp>
#include <boost/test/included/unit_test.hpp>

<<<<<<< HEAD
#include <causalize/graph_implementation/unknowns_collector.h>
#include <causalize/graph_implementation/causalization_strategy.h>
=======
#include <causalize/unknowns_collector.h>
#include <causalize/causalization_strategy.h>
#include <causalize/contains_unknown.h>
>>>>>>> origin/experimental_multidim_intentional_causalization

#include <parser/parser.h>
#include <ast/ast_types.h>
#include <ast/equation.h>
#include <mmo/mmo_class.h>
#include <util/table.h>
#include <util/debug.h>
#include <util/ast_visitors/contains_expression.h>

#include <boost/variant/get.hpp>

using namespace boost::unit_test;
using namespace Modelica::AST;
using namespace Causalize;

<<<<<<< HEAD
void check_causality(MMO_Class& mmoClass, ExpList unknowns)
{
  const EquationList& causalEqs = mmoClass.equations_ref().equations_ref();
=======
void check_causality(MMO_Class &mmo_class, ExpList unknowns) {

  const EquationList &causalEqs = mmo_class.equations_ref().equations_ref();
>>>>>>> origin/experimental_multidim_intentional_causalization

    foreach_(Equation equation, causalEqs) {

<<<<<<< HEAD
  foreach_(Equation equation, causalEqs)
  {
    Equality eqEq = boost::get<Equality>(equation);
    Expression leftExpression = eqEq.left_ref();

    if (is<Reference>(leftExpression)) {
      knowns.push_back(leftExpression);
    } else if (is<Call>(leftExpression)) {
      Call call = boost::get<Call>(leftExpression);
      if (call.name() == "der") knowns.push_back(leftExpression);
    } else if (is<Output>(leftExpression)) {
      Output output = boost::get<Output>(leftExpression);
      foreach_(OptExp oe, output.args())
      {
        if (oe) knowns.push_back(oe.get());
=======
      ContainsUnknown occurrs(unknowns, mmo_class.syms());
      ERROR_UNLESS(is<Equality>(equation),"Must be only equality equations here");
      Equality eqEq = boost::get<Equality>(equation);
      Apply(occurrs, eqEq.right_ref());
      if (occurrs.getUsages().size()!=0) {
        ERROR("Using no causalized unknown");
>>>>>>> origin/experimental_multidim_intentional_causalization
      }

<<<<<<< HEAD
    foreach_(Expression unknown, unknowns)
    {
      Modelica::contains occurrs(unknown);
      if (Apply(occurrs, eqEq.right_ref())) {
        bool isKnown = false;
        foreach_(Expression known, knowns)
        {
          if (known == unknown) {
            isKnown = true;
          }
        }
        BOOST_CHECK(isKnown);
=======
      ExpList::iterator known = std::find(unknowns.begin(), unknowns.end(), eqEq.left_ref());
      if (known!=unknowns.end()) {
        unknowns.erase(known);
      } else {
        ERROR("Causalizing unknown already causalized");
>>>>>>> origin/experimental_multidim_intentional_causalization
      }

    }
<<<<<<< HEAD
  }
=======

    if (unknowns.size()!=0) {
      ERROR("Uncausalized unknowns");
    } else {
      std::cout << "Model causalized correctly\n";
    }

>>>>>>> origin/experimental_multidim_intentional_causalization
}

void rlc_test()
{
  bool r;

<<<<<<< HEAD
  StoredDef sd = parseFile("rlc.mo", r);
=======
  StoredDef sd = Parser::ParseFile("rlc.mo",r);
>>>>>>> origin/experimental_multidim_intentional_causalization

  if (!r) ERROR("Can't parse file\n");

  Class ast_c = boost::get<Class>(sd.classes().front());
  MMO_Class mmo(ast_c);

  UnknownsCollector collector(mmo);
  ExpList unknowns = collector.collectUnknowns();

  Causalize::CausalizationStrategy cStrategy(mmo);
  cStrategy.Causalize();

  check_causality(mmo, unknowns);
}

void rlc_simple_test()
{
  bool r;

<<<<<<< HEAD
  StoredDef sd = parseFile("rlc.mo", r);
=======
  StoredDef sd = Parser::ParseFile("rlc.mo",r);
>>>>>>> origin/experimental_multidim_intentional_causalization

  if (!r) ERROR("Can't parse file\n");

  Class ast_c = boost::get<Class>(sd.classes().front());
  MMO_Class mmo(ast_c);

  UnknownsCollector collector(mmo);
  ExpList unknowns = collector.collectUnknowns();

  Causalize::CausalizationStrategy cStrategy(mmo);
  cStrategy.CausalizeSimple();

  check_causality(mmo, unknowns);
}

void rlc_loop_test()
{
  bool r;

<<<<<<< HEAD
  StoredDef sd = parseFile("rlc_loop.mo", r);
=======
  StoredDef sd = Parser::ParseFile("rlc_loop.mo",r);
>>>>>>> origin/experimental_multidim_intentional_causalization

  if (!r) ERROR("Can't parse file\n");

  Class ast_c = boost::get<Class>(sd.classes().front());
  MMO_Class mmo(ast_c);

  UnknownsCollector collector(mmo);
  ExpList unknowns = collector.collectUnknowns();

  Causalize::CausalizationStrategy cStrategy(mmo);
  cStrategy.Causalize();

  check_causality(mmo, unknowns);
}

void rlc_loop_tarjan_test()
{
  bool r;

<<<<<<< HEAD
  StoredDef sd = parseFile("rlc_loop.mo", r);
=======
  StoredDef sd = Parser::ParseFile("rlc_loop.mo",r);
>>>>>>> origin/experimental_multidim_intentional_causalization

  if (!r) ERROR("Can't parse file\n");

  Class ast_c = boost::get<Class>(sd.classes().front());
  MMO_Class mmo(ast_c);

  UnknownsCollector collector(mmo);
  ExpList unknowns = collector.collectUnknowns();

  Causalize::CausalizationStrategy cStrategy(mmo);
  cStrategy.CausalizeTarjan();

  check_causality(mmo, unknowns);
}

void OneDHeatTransferTI_FD_test()
{
  bool r;

<<<<<<< HEAD
  StoredDef sd = parseFile("OneDHeatTransferTI_FD_100.mo", r);
=======
  StoredDef sd = Parser::ParseFile("OneDHeatTransferTI_FD_100.mo",r);
>>>>>>> origin/experimental_multidim_intentional_causalization

  if (!r) ERROR("Can't parse file\n");

  Class ast_c = boost::get<Class>(sd.classes().front());
  MMO_Class mmo(ast_c);

  UnknownsCollector collector(mmo);
  ExpList unknowns = collector.collectUnknowns();

  Causalize::CausalizationStrategy cStrategy(mmo);
  cStrategy.Causalize();

  check_causality(mmo, unknowns);
}

void OneDHeatTransferTI_FD_simple_test()
{
  bool r;

<<<<<<< HEAD
  StoredDef sd = parseFile("OneDHeatTransferTI_FD.mo", r);
=======
  StoredDef sd = Parser::ParseFile("OneDHeatTransferTI_FD.mo",r);
>>>>>>> origin/experimental_multidim_intentional_causalization

  if (!r) ERROR("Can't parse file\n");

  Class ast_c = boost::get<Class>(sd.classes().front());
  MMO_Class mmo(ast_c);

  UnknownsCollector collector(mmo);
  ExpList unknowns = collector.collectUnknowns();

  Causalize::CausalizationStrategy cStrategy(mmo);
  cStrategy.CausalizeSimple();

  check_causality(mmo, unknowns);
}

// TODO fix the check_causality method
// void OneDHeatTransferTI_FD_loop_test() {

//   bool r;

//   StoredDef sd = parseFile("OneDHeatTransferTI_FD_loop_100.mo",r);

//   if (!r)
//     ERROR("Can't parse file\n");

//   Class ast_c = boost::get<Class>(sd.classes().front());
//   MMO_Class mmo(ast_c);

//   UnknownsCollector collector(mmo);
//   ExpList unknowns = collector.collectUnknowns();

//   CausalizationStrategy cStrategy(mmo);
//   cStrategy.causalize("Anything");

//   check_causality(mmo, unknowns);

// }

test_suite* init_unit_test_suite(int, char*[])
{
  debugInit("p");

  framework::master_test_suite().p_name.value = "Causalization Strategy Test";

  framework::master_test_suite().add(BOOST_TEST_CASE(&rlc_test));
  framework::master_test_suite().add(BOOST_TEST_CASE(&rlc_simple_test));
  framework::master_test_suite().add(BOOST_TEST_CASE(&rlc_loop_test));
  framework::master_test_suite().add(BOOST_TEST_CASE(&rlc_loop_tarjan_test));
  framework::master_test_suite().add(BOOST_TEST_CASE(&OneDHeatTransferTI_FD_test));
  framework::master_test_suite().add(BOOST_TEST_CASE(&OneDHeatTransferTI_FD_simple_test));
  // TODO fix the check_causality method
  // framework::master_test_suite().add( BOOST_TEST_CASE( &OneDHeatTransferTI_FD_loop_test) );

  return 0;
}
