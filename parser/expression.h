#ifndef EXPRESSION_PARSER
#define EXPRESSION_PARSER
#include <boost/spirit/include/qi.hpp>

#include <ast/expression.h>
#include <parser/skipper.h>
#include <parser/ident.h>

namespace qi = boost::spirit::qi;
using namespace Modelica::AST;
namespace Modelica {
  namespace parser {

   /* Modelica rules for floating point values */
   template <typename T>
   struct modelica_number_real_policies : qi::strict_ureal_policies<T>
   {
     static bool const expect_dot = true;
     static bool const allow_leading_dot = false;
     static bool const allow_trailing_dot = true;

     template <typename Iter, typename Attribute>
     static bool
     parse_nan(Iter& first, Iter const& last, Attribute& attr) {
       return false;
     }

     template <typename Iter, typename Attribute>
     static bool
     parse_inf(Iter& first, Iter const& last, Attribute& attr) {
       return false;
     }
   };


    template <typename Iterator>
    struct expression: qi::grammar<Iterator,skipper<Iterator>,Expression()>
    {
      expression(Iterator &it);


      // Rules
      qi::rule<Iterator,skipper<Iterator>,Expression()> expression_;
      qi::rule<Iterator,skipper<Iterator>,Expression()> primary;
      qi::rule<Iterator,skipper<Iterator>,Expression()> subscript;
      qi::rule<Iterator,skipper<Iterator>,Expression()> function_argument;
      qi::rule<Iterator, skipper<Iterator>, Reference()> component_reference;
      qi::rule<Iterator, skipper<Iterator>, Call()> call_exp;
      qi::rule<Iterator, skipper<Iterator>, FunctionExp()> function_exp;
      qi::rule<Iterator, skipper<Iterator>, Named()> named_argument;
      qi::rule<Iterator, skipper<Iterator>, Name()> name;
 
      qi::rule<Iterator, skipper<Iterator>, ExpList()> 
          function_call_args, function_arguments, named_arguments, 
          expression_list, array_subscripts;
                  ;


      /* Rules with no skip */
      qi::rule<Iterator, String()> string_;
      ident<Iterator> ident_;

      /* Modelica rules for floating point and integers values */
      qi::real_parser<double,modelica_number_real_policies<double> > modelica_double;
      qi::uint_parser<int> modelica_int;

      /* Operators tokens */
      qi::rule<Iterator> OPAREN, CPAREN, COLON, SEMICOLON, QUOTE, STAR,
                  OBRACKET, CBRACKET, COMA, OBRACE, CBRACE, EQUAL, PLUS,
                  ASSIGN;

      /* Keywords with value */
      qi::rule<Iterator, Name()> DOT, DER, INITIAL;

      /* Keywords tokens */
      qi::rule<Iterator> FUNCTION1;
      Iterator &it;
    };
  }
}
#endif
