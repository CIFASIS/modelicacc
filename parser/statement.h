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
