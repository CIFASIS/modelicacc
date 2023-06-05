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
#include <getopt.h>
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

void usage()
{
  cout << "Usage flatter [options] file" << endl;
  cout << "Generates flattened model for a given Modelica file." << endl;
  cout << endl;
  cout << "-c <class_name>, --class <class_name> Flatter class <class_name> defined in a Modelica model." << endl;
  cout << "-d, --debug     Generate a detailed log file with debug information about the " << endl;
  cout << "                the generated graph, including a dot file for it and the" << endl; 
  cout << "                differents stages of the flatter algorithm." << endl;
  cout << "-g <path>, --graph <path> Sets the output path for the generated graph dot file," << endl;
  cout << "                this flag only has effect if the debug flag is enabled." << endl;
  cout << "-h, --help      Display this information and exit" << endl;
  
  cout << "-v, --version   Display version information and exit" << endl;
  cout << endl;
  cout << "Modelica C Compiler home page: https://github.com/CIFASIS/modelicacc " << endl;
}

void version()
{
  cout << "Modelica C Compiler 2.0" << endl;
  cout << "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>" << endl;
  cout << "This is free software: you are free to change and redistribute it." << endl;
  cout << "There is NO WARRANTY, to the extent permitted by law." << endl;
}


int main(int argc, char** argv)
{
  using namespace std;
  using namespace Modelica::AST;
  using namespace Modelica;
  using namespace boost;

  bool ret;
  char* className = NULL;
  string filename;
  int debug = 0;
  std::ofstream outputFile;
  int opt;
  extern char* optarg;
  string output_path = "";

  while (true) {
    static struct option long_options[] = {{"version", no_argument, 0, 'v'},
                                           {"help", no_argument, 0, 'h'},
                                           {"class", required_argument, 0, 'c'},
                                           {"graph", required_argument, 0, 'g'},
                                           {"debug", no_argument, 0, 'd'},
                                           {0, 0, 0, 0}};
    int option_index = 0;
    opt = getopt_long(argc, argv, "vhdc:g:", long_options, &option_index);
    if (opt == EOF) {
      break;
    }
    switch (opt) {
    case 'v':
      version();
      exit(0);
    case 'h':
      usage();
      exit(0);
    case 'g':
      filename = optarg;
      break;
    case 'd':
      debug = 1;
      break;
    case 'c':
      className = optarg;
      break;
    case '?':
      usage();
      exit(-1);
      break;
    default:
      abort();
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
      Modelica::Logger::instance().setFile(className);

      if (debug) LOG << "Searching for class " << (className ? className : "NULL") << std::endl;

      ClassFinder re = ClassFinder();
      OptTypeDefinition m = re.resolveType(mmo, className);
      if (m) {
        typeDefinition td = m.get();
        Type::Type t_final = get<1>(td);
        if (is<Type::Class>(t_final)) mmo = *(boost::get<Type::Class>(t_final).clase());
      }
    } else {
      Modelica::Logger::instance().setFile("flatter");
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
