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

/*! \file test_parse.cpp
*   Current support is for Linux filesystem.
*/

#include <ostream>

#include <boost/filesystem.hpp>

#include <parser/parser.h>

using namespace boost::filesystem;
using namespace std;

int main(){
  path test_path("../mccprograms"); 

  for(directory_entry &x : directory_iterator(test_path)){
    bool ret = false;
    bool &ref = ret;

    Modelica::AST::StoredDef sd = Modelica::Parser::ParseFile(x.path().generic_string(), ref);
    cout << "Parsed class in " << x.path() << "\n";
    cout << sd;
    cout << "\n";
  }

  return 0;
}
