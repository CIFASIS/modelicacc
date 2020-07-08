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

#include <iostream>
#include <fstream>
#include <stdio.h>

#include <mmo/mmo_class.h>
#include <mmo/mmo_tree.h>
#include <parser/parser.h>

int main(int argc, char** argv){
  using namespace std;
  using namespace Modelica::AST;
  using namespace Modelica;
  using namespace boost;

  bool ret;
  char *out = NULL;
  char opt;
  std::ofstream outputFile;

  while ((opt = getopt(argc, argv, "o:")) != -1){
    switch (opt){
      case 'o':
        out = optarg;
        break;
    }
  }

  StoredDef sd;
  if (argv[optind] != NULL)
    sd = Parser::ParseFile(argv[optind], ret);
  else
    sd = Parser::ParseFile("", ret);

  if(ret){
    if(out) std::cout << sd << std::endl;

    else std::cout << sd << std::endl;
  }

  else std::cout << "Error parser" << std::endl;
 
  if(out) return 0;

  return 0;
}
