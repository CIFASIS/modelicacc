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
#include <ast/class.h>
#include <cstdlib>
#include <parser/parser.h>
#include <util/debug.h>
#include <boost/variant/get.hpp>
#include <test_derivate/test_derivate.h>


int main(int argc, char ** argv)
{
  using namespace std;
  using namespace Modelica::AST;
  using namespace Modelica;

  bool ret;
  int opt;
  StoredDef sd;
  while ((opt = getopt(argc, argv, "d:q")) != -1) {
    switch (opt) {
     case 'd':
       if (optarg != NULL && isDebugParam(optarg)) {
         debugInit(optarg);
       } else {
         ERROR("command-line option d has no arguments\n");
       }
       break;
    }
  }
 
  if (argv[optind]!=NULL) 
    sd=parseFile(argv[optind],ret);
  else
    sd=parseFile("",ret);
 
  if (!ret)
    return -1;
  Class ast_c = boost::get<Class>(sd.classes().front());
  MMO_Class mmo(ast_c);
  TestDerivate td(mmo);
  //probamos derivar las equalities
  td.derivateAllEqualities();
  cout << mmo << "\n";
  //probamos reemplazar der(a) por der_a
  Reference r = Reference("a");
  td.findReplaceDer(r);  
  cout << mmo << "\n";
  return 0;
}
