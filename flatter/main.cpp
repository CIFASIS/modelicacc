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

#include <boost/type_traits/remove_cv.hpp>
#include <boost/variant/get.hpp>

#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

#include <flatter/class_finder.h>
#include <flatter/connectors.h>
#include <flatter/flatter.h>
#include <mmo/mmo_class.h>
#include <mmo/mmo_tree.h>
#include <parser/parser.h>

#include <util/table.h>

int main(int argc, char** argv){
  using namespace std;
  using namespace Modelica::AST;
  using namespace Modelica;
  using namespace boost;

  bool ret;
  char *className = NULL;
  string filename;
  char opt;
  int debug = 0;
  std::ofstream outputFile;
  cout << "Estoy\n";

  while ((opt = getopt(argc, argv, "i:c:g:d")) != -1) {
    switch (opt) {
    case 'g':
      filename = optarg;
      break;
    case 'd':
      debug = 1;
    case 'c':
      className = optarg;
      break;
    }
  }


  StoredDef sd;
  if (argv[optind] != NULL)
    sd = Parser::ParseFile(argv[optind], ret);
  else
    sd = Parser::ParseFile("", ret);

  if(ret){
    MMO_Tree mt;
    MMO_Class mmo = mt.create(sd);

    Flatter f = Flatter();
    if(className == NULL)
      className = (char*)::className(sd.classes().back()).c_str();
    
    if(className != NULL){
      if (debug) 
        std::cerr << "Searching for class " << (className ? className : "NULL") << std::endl;

      ClassFinder re = ClassFinder();
      OptTypeDefinition m = re.resolveType(mmo, className);
      if(m){
        typeDefinition td = m.get();
        Type::Type t_final = get<1>(td);
        if (is<Type::Class>(t_final)) 
          mmo = *(boost::get<Type::Class>(t_final).clase());
      }
    }

    if(debug){
      std::cerr << "Class to  Flat: " << endl;
      std::cerr << mmo << std::endl;
      std::cerr << " - - - - - - - - - - - - - - - - - - - - - - - - " << std::endl << std::endl;
    }

    f.Flat(mmo, false, true);
    if(debug){
      std::cerr << "First Flatter: " << endl;
      std::cerr << mmo << std::endl;
      std::cerr << " - - - - - - - - - - - - - - - - - - - - - - - - " << std::endl << std::endl;
    }

    Connectors co(mmo);
    co.solve();
    if(debug){
      std::cerr << " - - - - - - - - - - - - - - - - - - - - - - - - " << std::endl;
      if(filename.empty())
        co.debug("prueba.dot");

      co.debug(filename);
      std::cerr << " - - - - - - - - - - - - - - - - - - - - - - - - " << std::endl;
    }

    f.removeConnectorVar(mmo);
  } 

  else
    std::cout << "Error parser" << std::endl;

  return 0;
}
