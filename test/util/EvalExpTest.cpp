#include <iostream>

#include <boost/test/unit_test.hpp>
#include <boost/test/included/unit_test.hpp>

#include <ast/ast_builder.h>
#include <ast/expression.h>
#include <util/ast_util.h>

using namespace boost::unit_test;

//____________________________________________________________________________//

void eval_binary_exp_add_test()
{
  AST_Expression_Integer intExp = (AST_Expression_Integer)newAST_Expression_Integer(1);
  AST_Expression_Integer intExp2 = (AST_Expression_Integer)newAST_Expression_Integer(1);
  AST_Expression_BinOp binOp = (AST_Expression_BinOp)newAST_Expression_BinOp(intExp, intExp2, BINOPADD);

  EvalExp *evalExp = new EvalExp(NULL);
  AST_Expression result = evalExp->eval(binOp);

  BOOST_CHECK(result->expressionType() == EXPINTEGER);
  BOOST_CHECK(result->getAsInteger()->val() == 2);
}

void eval_binary_exp_sub_test()
{
  AST_Expression_Integer intExp = (AST_Expression_Integer)newAST_Expression_Integer(2);
  AST_Expression_Integer intExp2 = (AST_Expression_Integer)newAST_Expression_Integer(1);
  AST_Expression_BinOp binOp = (AST_Expression_BinOp)newAST_Expression_BinOp(intExp, intExp2, BINOPSUB);

  EvalExp *evalExp = new EvalExp(NULL);
  AST_Expression result = evalExp->eval(binOp);

  BOOST_CHECK(result->expressionType() == EXPINTEGER);
  BOOST_CHECK(result->getAsInteger()->val() == 1);
}

void eval_binary_exp_mult_test()
{
  AST_Expression_Integer intExp = (AST_Expression_Integer)newAST_Expression_Integer(2);
  AST_Expression_Integer intExp2 = (AST_Expression_Integer)newAST_Expression_Integer(2);
  AST_Expression_BinOp binOp = (AST_Expression_BinOp)newAST_Expression_BinOp(intExp, intExp2, BINOPMULT);

  EvalExp *evalExp = new EvalExp(NULL);
  AST_Expression result = evalExp->eval(binOp);

  BOOST_CHECK(result->expressionType() == EXPINTEGER);
  BOOST_CHECK(result->getAsInteger()->val() == 4);
}

void eval_binary_exp_div_test()
{
  AST_Expression_Integer intExp = (AST_Expression_Integer)newAST_Expression_Integer(4);
  AST_Expression_Integer intExp2 = (AST_Expression_Integer)newAST_Expression_Integer(2);
  AST_Expression_BinOp binOp = (AST_Expression_BinOp)newAST_Expression_BinOp(intExp, intExp2, BINOPDIV);

  EvalExp *evalExp = new EvalExp(NULL);
  AST_Expression result = evalExp->eval(binOp);

  BOOST_CHECK(result->expressionType() == EXPINTEGER);
  BOOST_CHECK(result->getAsInteger()->val() == 2);
}

void eval_binary_exp_exp_test()
{
  AST_Expression_Integer intExp = (AST_Expression_Integer)newAST_Expression_Integer(2);
  AST_Expression_Integer intExp2 = (AST_Expression_Integer)newAST_Expression_Integer(3);
  AST_Expression_BinOp binOp = (AST_Expression_BinOp)newAST_Expression_BinOp(intExp, intExp2, BINOPEXP);

  EvalExp *evalExp = new EvalExp(NULL);
  AST_Expression result = evalExp->eval(binOp);

  BOOST_CHECK(result->expressionType() == EXPINTEGER);
  BOOST_CHECK(result->getAsInteger()->val() == 8);
}

void eval_binary_exp_combi_test()
{
  // 24 / ((2 + 1 + (-1)) ^ 3)
  AST_Expression_Integer intExp = (AST_Expression_Integer)newAST_Expression_Integer(24);
  AST_Expression_Integer intExp2 = (AST_Expression_Integer)newAST_Expression_Real(2);
  AST_Expression_Integer intExp3 = (AST_Expression_Integer)newAST_Expression_Integer(1);
  AST_Expression_Integer intExp4 = (AST_Expression_Integer)newAST_Expression_Integer(1);
  AST_Expression_Integer intExp5 = (AST_Expression_Integer)newAST_Expression_Integer(3);
  AST_Expression_BinOp binOp = (AST_Expression_BinOp)newAST_Expression_BinOp(intExp2, intExp3, BINOPADD);
  AST_Expression_UMinus uMinus = (AST_Expression_UMinus)newAST_Expression_UnaryMinus(intExp4);
  AST_Expression_BinOp binOp1 = (AST_Expression_BinOp)newAST_Expression_BinOp(binOp, uMinus, BINOPADD);
  AST_Expression_BinOp binOp2 = (AST_Expression_BinOp)newAST_Expression_BinOp(binOp1, intExp5, BINOPEXP);
  AST_Expression_BinOp binOp3 = (AST_Expression_BinOp)newAST_Expression_BinOp(intExp, binOp2, BINOPDIV);

  EvalExp *evalExp = new EvalExp(NULL);
  AST_Expression result = evalExp->eval(binOp3);

  BOOST_CHECK(result->expressionType() == EXPREAL);
  BOOST_CHECK(result->getAsReal()->val() == 3);
}

//____________________________________________________________________________//

test_suite *init_unit_test_suite(int, char *[])
{
  framework::master_test_suite().p_name.value = "Expression Evaluation";

  framework::master_test_suite().add(BOOST_TEST_CASE(&eval_binary_exp_add_test));
  framework::master_test_suite().add(BOOST_TEST_CASE(&eval_binary_exp_sub_test));
  framework::master_test_suite().add(BOOST_TEST_CASE(&eval_binary_exp_div_test));
  framework::master_test_suite().add(BOOST_TEST_CASE(&eval_binary_exp_mult_test));
  framework::master_test_suite().add(BOOST_TEST_CASE(&eval_binary_exp_exp_test));
  framework::master_test_suite().add(BOOST_TEST_CASE(&eval_binary_exp_combi_test));

  return 0;
}

//____________________________________________________________________________//

// EOF
