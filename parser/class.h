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


#ifndef CLASS_PARSER
#define CLASS_PARSER
#include <boost/spirit/include/qi.hpp>

#include <ast/class.h>
#include <parser/skipper.h>
#include <parser/equation.h>
#include <parser/statement.h>

namespace qi = boost::spirit::qi;
using namespace Modelica::AST;
namespace Modelica {
  namespace parser {

    template <typename Iterator>
    struct class_: qi::grammar<Iterator,skipper<Iterator>,StoredDef()>
    {
      class_(Iterator &it);


      // Rules
      qi::rule<Iterator,skipper<Iterator>,ClassType()> class__;
      modification<Iterator> modification_;
      expression<Iterator> expression_;
      equation<Iterator> equation_;
      statement<Iterator> statement_;
      qi::rule<Iterator, skipper<Iterator>, Import()> import_clause;
      qi::rule<Iterator, skipper<Iterator>, Component()> component_clause;
      qi::rule<Iterator, skipper<Iterator>, Extends()> extends_clause;
      qi::rule<Iterator, skipper<Iterator>, Declaration()> component_declaration;
      qi::rule<Iterator, skipper<Iterator>, Expression()> condition_attribute;
      qi::rule<Iterator, skipper<Iterator>, DeclList()> component_list;
      qi::rule<Iterator, skipper<Iterator>, Element()> element, element2; 
      qi::rule<Iterator, skipper<Iterator>, ElemList()> element_list; 
      qi::rule<Iterator, skipper<Iterator>, CompElement()> comp_element;
      qi::rule<Iterator, skipper<Iterator>, External()> external_function_call;
      qi::rule<Iterator, skipper<Iterator>, DefClass()> def_class;
      qi::rule<Iterator, skipper<Iterator>, EnumClass()> enum_class;
      qi::rule<Iterator, skipper<Iterator>, DerClass()> deriv_class;
      qi::rule<Iterator, skipper<Iterator>, ExtendsClass()> extends_class;
      qi::rule<Iterator, skipper<Iterator>, Class()> class_t;
      qi::rule<Iterator, skipper<Iterator>, ClassList()> class_list; 
      qi::rule<Iterator, skipper<Iterator>, Composition()> composition;
      qi::rule<Iterator, skipper<Iterator>, ClassType()> class_specifier, class_definition, class_def;
      qi::rule<Iterator, skipper<Iterator>, StoredDef()> stored_definition; 
 

      /* Operators tokens */
      qi::rule<Iterator> EQUAL, IMPORT, EXTENDS, DOT, STAR, OBRACE, COMA, CBRACE, IF, SEMICOLON, PUBLIC, PROTECTED, OPAREN, CPAREN,DER, ENUMERATION, EXTERNAL, END, ENCAPSULATED, FINAL, WITHIN, REPLACEABLE, REDECLARE, INNER, OUTER;

      /* Keywords tokens */
      Iterator &it;
    };
  }
}
#endif
