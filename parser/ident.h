#ifndef IDENT_PARSER
#define IDENT_PARSER
#include <boost/spirit/include/qi.hpp>
#include <ast/expression.h>
namespace qi = boost::spirit::qi;
using namespace Modelica::AST;
namespace Modelica {
  namespace parser {
    template <typename Iterator>
    struct ident: qi::grammar<Iterator,Name()>
    {
      ident(Iterator &it);
      qi::rule<Iterator,Name()> ident_, keyword_ident;
    };
  }
}
  
 
#endif
