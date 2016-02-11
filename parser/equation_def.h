#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/boost_tuple.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::EquationSection,
    (bool, initial_)
    (Modelica::AST::EquationList, equations_)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::Equality,
    (Modelica::AST::Expression, left_)
    (Modelica::AST::Expression, right_)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::Connect,
    (Modelica::AST::Expression, left_)
    (Modelica::AST::Expression, right_)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::CallEq,
    (Modelica::AST::Name, name_)
    (Modelica::AST::ExpList , args_)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::IfEq,
    (Modelica::AST::Expression, c)
    (Modelica::AST::EquationList, els)
    (Modelica::AST::IfEq::ElseList, elsesif)
    (Modelica::AST::EquationList, elses)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::ForEq,
    (Modelica::AST::Indexes, r)
    (Modelica::AST::EquationList, els)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::WhenEq,
    (Modelica::AST::Expression, c)
    (Modelica::AST::EquationList, els)
    (Modelica::AST::WhenEq::ElseList , elsew)
)



namespace Modelica
{
  namespace parser {

    std::string at(std::string::const_iterator where, std::string::const_iterator start) ;
    template <typename Iterator>
    equation<Iterator>::equation(Iterator &it) : equation::base_type(equation_), it(it), modification_(it), expression_(it),
                                                OPAREN("("),CPAREN(")"), COMA(","), CONNECT("connect"), SEMICOLON(";"), 
                                                WHEN("when"), THEN("then"), ELSEWHEN("elsewhen"), END("end"), EQUAL("="),
                                                FOR("for"), IF("if"), ELSEIF("elseif"), ELSE("else"), LOOP("loop"), 
                                                EQUATION("equation"), INITIAL("initial") {
      using qi::_1;
      using qi::_2;
      using qi::_3;
      using qi::_4;
      using qi::_val;
      using phoenix::bind;
      using phoenix::val;
      using qi::fail;
      using qi::on_error;
      using qi::matches;

      equation_section =
                          matches[INITIAL] >> EQUATION >> equation_list
                       ;

 
      equation_ = (
                    equality_equation
                  | for_equation
                  | if_equation
                  | connect_equation
                  | when_equation
                  | call_equation
                  ) > modification_.comment
                  ;

      equality_equation = 
                            (expression_.simple_expression >> EQUAL > expression_.expression_)
                        ;

      connect_equation = 
                         CONNECT > OPAREN > expression_.component_reference > COMA > expression_.component_reference > CPAREN
                       ;

      when_equation = 
                      WHEN > expression_.expression_ > THEN > equation_list > *(ELSEWHEN > expression_.expression_ > THEN > equation_list) > END > WHEN 
                    ;

      if_equation = 
                    IF > modification_.expression_ > THEN > equation_list > *(ELSEIF > modification_.expression_ > THEN > equation_list) > -(ELSE > equation_list) > END > IF
                  ;
 
      for_equation  = 
                      FOR > modification_.expression_.for_indices > LOOP > equation_list > END > FOR
                    ;
   
      equation_list =
                      *(equation_ > SEMICOLON)
                     ;

      call_equation = 
                       modification_.expression_.name >> modification_.expression_.function_call_args
                    ;

 
      /* Error and debug */
      on_error<fail>
        (
            equation_
          , std::cerr
                << val("Parser error. Expecting ")
                << _4                               // what failed?
                << bind(&at,_3,it)
                << std::endl
        );
 
      equation_list.name("equation_list");

      equation_.name("equation");
      equation_section.name("equation_section");

      for_equation.name("for equation");

      if_equation.name("if equation");
      when_equation.name("when_equation");
    }
  }
}
