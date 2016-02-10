#ifndef SKIPPER_PARSER
#define SKIPPER_PARSER

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
template <typename Iterator>
struct skipper : qi::grammar<Iterator> {
  skipper() : skipper::base_type(start) {
    using qi::char_;
    ascii::space_type space;

    start = space                               // tab/space/cr/lf
          |   "//" >> *(char_ - "\n") >> "\n"     // C++-style comments
          |   "/*" >> *(char_ - "*/") >> "*/"     // C-style comments
          ;
    }
    qi::rule<Iterator> start;
};
 template <typename T>
  std::vector<T> consume_one(T t) {
    std::vector<T> l(1,t);
    return l;
  }
 

  template <typename T>
  std::vector<T> append_list(std::vector<T> lt, std::vector<T> lt2) {
    lt.insert(lt.end(),lt2.begin(), lt2.end());
    return lt;
  }


#endif
