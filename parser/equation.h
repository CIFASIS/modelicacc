#ifndef EQUATION_PARSER
#define EQUATION_PARSER
#include <boost/spirit/include/qi.hpp>

#include <ast/equation.h>
#include <parser/skipper.h>
#include <parser/modification.h>

namespace qi = boost::spirit::qi;
using namespace Modelica::AST;
namespace Modelica {
  namespace parser {

    template <typename Iterator>
    struct equation: qi::grammar<Iterator,skipper<Iterator>,Equation()>
    {
      equation(Iterator &it);


      // Rules
      qi::rule<Iterator,skipper<Iterator>,Equation()> equation_;
      qi::rule<Iterator, skipper<Iterator>, Equality()> equality_equation;
      qi::rule<Iterator, skipper<Iterator>, Connect()> connect_equation;
      qi::rule<Iterator, skipper<Iterator>, IfEq()> if_equation;
      qi::rule<Iterator, skipper<Iterator>, ForEq()> for_equation;
      qi::rule<Iterator, skipper<Iterator>, CallEq()> call_equation;
      qi::rule<Iterator, skipper<Iterator>, WhenEq()> when_equation;
      qi::rule<Iterator, skipper<Iterator>, EquationList() > equation_list;
      qi::rule<Iterator, skipper<Iterator>, EquationSection()> equation_section;
      modification<Iterator> modification_;
      expression<Iterator> expression_;

      /* Operators tokens */
      qi::rule<Iterator> EQUAL, CONNECT, OPAREN, COMA, CPAREN, WHEN, THEN, ELSEWHEN, END, SEMICOLON, IF, ELSEIF, ELSE, FOR, LOOP, EQUATION, INITIAL;

      /* Keywords tokens */
      Iterator &it;
    };
  }
}
#endif
