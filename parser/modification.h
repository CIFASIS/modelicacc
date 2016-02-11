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


#ifndef MODIFICATION_PARSER
#define MODIFICATION_PARSER
#include <boost/spirit/include/qi.hpp>

#include <ast/modification.h>
#include <parser/skipper.h>
#include <parser/expression.h>

namespace qi = boost::spirit::qi;
using namespace Modelica::AST;
namespace Modelica {
  namespace parser {

    template <typename Iterator>
    struct modification: qi::grammar<Iterator,skipper<Iterator>,Modification()>
    {
      modification(Iterator &it);


      // Rules
      qi::rule<Iterator, skipper<Iterator>, Modification()> modification_;
      qi::rule<Iterator, skipper<Iterator>, ModEq()> equal_mod;
      qi::rule<Iterator, skipper<Iterator>, Declaration()> declaration, component_declaration1;
      qi::rule<Iterator, skipper<Iterator>, ElRepl()> element_replaceable; 
      qi::rule<Iterator, skipper<Iterator>, ElMod()> element_modification; 
      qi::rule<Iterator, skipper<Iterator>, Component1()> component_clause1;
      qi::rule<Iterator, skipper<Iterator>, ElRedecl()> element_redeclaration;
      qi::rule<Iterator, skipper<Iterator>, ModAssign()> assign_mod;
      qi::rule<Iterator, skipper<Iterator>, StringComment()> string_comment;
      qi::rule<Iterator, skipper<Iterator>, Annotation()> annotation;
      qi::rule<Iterator, skipper<Iterator>, ModClass()> class_mod;
      qi::rule<Iterator, skipper<Iterator>, Comment()> comment;
      qi::rule<Iterator, skipper<Iterator>, ClassPrefixes() > class_prefixes; 
      qi::rule<Iterator, skipper<Iterator>, TypePrefixes()> type_prefix; 
      qi::rule<Iterator, skipper<Iterator>, Enum()> enumeration_literal;
      qi::rule<Iterator, skipper<Iterator>, EnumList()> enum_list;
      qi::rule<Iterator, skipper<Iterator>, EnumSpec()> enum_spec;
      qi::rule<Iterator, skipper<Iterator>, ClassModification()> class_modification;
      qi::rule<Iterator, skipper<Iterator>, ShortClass()> short_class_definition;
      qi::rule<Iterator, skipper<Iterator>, Argument()> argument;
      qi::rule<Iterator, skipper<Iterator>, Constrained()> constraining_clause; 
      expression<Iterator> expression_;

      /* Operators tokens */
      qi::rule<Iterator> EQUAL, ASSIGN, PLUS, ANNOTATION, COMA, COLON, ENUMERATION, OPAREN, CPAREN, REPLACEABLE, CONSTRAINEDBY,
                         EACH, REDECLARE, FINAL;

      Iterator &it;
    };
  }
}
#endif
