#include <boost/spirit/include/qi.hpp>
#include <iostream>
#include <parser/modification.h>
#include <parser/skipper.h>

/*namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

template <typename Iterator>
struct ident2_parser: qi::grammar<Iterator,skipper<Iterator>,Name()>
{
  ident2_parser(Iterator &it) : ident2_parser::base_type(ident2), ident(it) {
    using qi::char_;
    using qi::alpha;
    using qi::alnum;
    using qi::lexeme;

    ident2 = ident >> "+" >> ident;
  }

  qi::rule<Iterator,skipper<Iterator>,Name()> ident2;
  Modelica::parser::ident<Iterator> ident;
};*/


////////////////////////////////////////////////////////////////////////////
//  Main program
////////////////////////////////////////////////////////////////////////////
int
main()
{
    std::cout << "/////////////////////////////////////////////////////////\n\n";
    std::cout << "\t\tA complex number micro parser for Spirit...\n\n";
    std::cout << "/////////////////////////////////////////////////////////\n\n";

    std::cout << "Give me a complex number of the form r or (r) or (r,i) \n";
    std::cout << "Type [q or Q] to quit\n\n";

    std::string str;
    Modelica::AST::Modification res;
    typedef std::string::const_iterator iterator_type;
    typedef skipper<iterator_type> space_type;

    while (getline(std::cin, str))
    {
        if (str.empty() || str[0] == 'q' || str[0] == 'Q')
            break;

        std::string::const_iterator iter = str.begin();
        std::string::const_iterator end = str.end();
        Modelica::parser::modification<iterator_type> p(iter);

        phrase_parse(iter, end, p.modification_, space_type(),res);
        if (iter==end)
        {
            std::cout << "-------------------------\n";
            std::cout << "Parsing succeeded\n";
            std::cout << "got: " << res << std::endl;
            std::cout << "\n-------------------------\n";
        }
        else
        {
            std::cout << "-------------------------\n";
            std::cout << "Parsing failed\n";
            std::cout << "-------------------------\n";
        }
    }

    std::cout << "Bye... :-) \n\n";
    return 0;
}

