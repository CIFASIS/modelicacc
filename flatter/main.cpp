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
#include <time.h>
#include <unistd.h>

#include <flatter/class_finder.h>
#include <flatter/connectors.h>
#include <flatter/flatter.h>
#include <mmo/mmo_class.h>
#include <mmo/mmo_tree.h>
#include <parser/parser.h>
#include <util/logger.h>
#include <util/table.h>

int main(int argc, char** argv)
{
  using namespace std;
  using namespace Modelica::AST;
  using namespace Modelica;
  using namespace boost;

  bool ret;
  char* className = NULL;
  string filename;
  char opt;
  int debug = 0;
  std::ofstream outputFile;

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

  if (ret) {
    MMO_Tree mt;
    MMO_Class mmo = mt.create(sd);

    Flatter f = Flatter();
    if (className == NULL) className = (char*)::className(sd.classes().back()).c_str();

    if (className != NULL) {
      Logger::instance().setFile(className);

      if (debug) LOG << "Searching for class " << (className ? className : "NULL") << std::endl;

      ClassFinder re = ClassFinder();
      OptTypeDefinition m = re.resolveType(mmo, className);
      if (m) {
        typeDefinition td = m.get();
        Type::Type t_final = get<1>(td);
        if (is<Type::Class>(t_final)) mmo = *(boost::get<Type::Class>(t_final).clase());
      }
    } else {
      Logger::instance().setFile("flatter");
    }

    if (debug) {
      LOG << "Class to  Flat: " << endl;
      LOG << mmo << std::endl;
      LOG << " - - - - - - - - - - - - - - - - - - - - - - - - " << std::endl << std::endl;
    }

    f.Flat(mmo, false, true);
    f.removeConnectorVar(mmo);
    if (debug) {
      LOG << "First Flatter: " << endl;
      LOG << mmo << std::endl;
      LOG << " - - - - - - - - - - - - - - - - - - - - - - - - " << std::endl << std::endl;
    }

    Connectors co(mmo);

    clock_t start, end;
    start = clock();
    
    co.solve();

    end = clock();
    double exect = double(end - start) / double(CLOCKS_PER_SEC);

    if (debug) {
      LOG << " - - - - - - - - - - - - - - - - - - - - - - - - " << std::endl;
      if (filename.empty()) {
        if (className != NULL) {
          std::string str_class_name = className; 
          co.debug(str_class_name+".dot");
        } else {
          co.debug("flatter.dot");
        }      
      } else {
        co.debug(filename);
      }
      LOG << "Execution time: " << exect << "\n";
      LOG << " - - - - - - - - - - - - - - - - - - - - - - - - " << std::endl;
    }
  }

  else
    LOG << "Error parser" << std::endl;

  return 0;
}
