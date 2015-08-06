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

#include <fstream>
#include <streambuf>
#include <util/debug.h>
#include <parser/parser.h>
#include <parser/parser_imp.h>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/boost_tuple.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>

using namespace std;
namespace Modelica {
  typedef std::string::const_iterator iterator_type;
  typedef Modelica::parser::skipper<iterator_type> space_type;

  AST::StoredDef parseFile(std::string name, bool &r) {

    std::string str, res;
    AST::StoredDef sd;

    if (name!="") {
      std::ifstream in(name.c_str());
      if (in.fail()) {
        std::cerr << "Unable to open file " << name << endl;
        exit(-1);
      }
      in.unsetf(std::ios::skipws);
      std::string res((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
      std::string::const_iterator iter = res.begin();
      std::string::const_iterator end = res.end();
      Modelica::parser::parser<iterator_type> p(iter);
      r = phrase_parse(iter, end, p.stored_definition, space_type(), sd);
      if (r && iter == end) 
        return sd;
      r=false;
      return AST::StoredDef();
    } else {
      while (std::getline(std::cin, str)) {
        str += "\n";
        res.append(str);
      }
      std::string::const_iterator iter = res.begin();
      std::string::const_iterator end = res.end();
      Modelica::parser::parser<iterator_type> p(iter);
      r = phrase_parse(iter, end, p.stored_definition, space_type(), sd);
      if (r && iter == end) {
        return sd;
      }
      r=false;
      return AST::StoredDef();
    }
  }

  AST::Expression parseExpression(std::string exp, bool &r) {
    std::string::const_iterator iter = exp.begin();
    std::string::const_iterator end = exp.end();
    Modelica::parser::parser<iterator_type> p(iter);
    AST::Expression e;
    r = phrase_parse(iter, end, p.expression, space_type(), e);
    if (r && iter == end) {
      return e;
    } 
    return 0;
  }

};
