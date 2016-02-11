#ifndef STATEMENT_PARSER
#define STATEMENT_PARSER
#include <boost/spirit/include/qi.hpp>

#include <ast/statement.h>
#include <parser/skipper.h>
#include <parser/modification.h>

namespace qi = boost::spirit::qi;
using namespace Modelica::AST;
namespace Modelica {
  namespace parser {

    template <typename Iterator>
    struct statement: qi::grammar<Iterator,skipper<Iterator>,Statement()>
    {
      statement(Iterator &it);


      // Rules
      qi::rule<Iterator,skipper<Iterator>,Statement()> statement_;
      modification<Iterator> modification_;
      expression<Iterator> expression_;
      qi::rule<Iterator, skipper<Iterator>, Assign() > assign_statement;
      qi::rule<Iterator, skipper<Iterator>, CallSt() > call_statement;
      qi::rule<Iterator, skipper<Iterator>, ForSt() > for_statement;
      qi::rule<Iterator, skipper<Iterator>, WhenSt() > when_statement;
      qi::rule<Iterator, skipper<Iterator>, WhileSt() > while_statement;
      qi::rule<Iterator, skipper<Iterator>, IfSt() > if_statement;
      qi::rule<Iterator, skipper<Iterator>, StatementList() > statement_list;
      qi::rule<Iterator, skipper<Iterator>, StatementSection() > algorithm_section;
 

      /* Operators tokens */
      qi::rule<Iterator> ASSIGN, CONNECT, OPAREN, COMA, CPAREN, WHEN, THEN, ELSEWHEN, END, SEMICOLON, IF, ELSEIF, ELSE, FOR, LOOP, ALGORITHM, INITIAL, WHILE;

      /* Keywords tokens */
      Iterator &it;
    };
  }
}
#endif
