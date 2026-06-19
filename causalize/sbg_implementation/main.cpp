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

#include "causalize/sbg_implementation/causalize.hpp"
#include "mmo/mmo_class.hpp"
#include "parser/parser.hpp"
#include "util/ast_visitors/state_variables_finder.hpp"
#include "util/debug.hpp"
#include "util/logger.hpp"

#include "boost/variant/get.hpp"
#include "getopt.h"

using namespace std;
using namespace Modelica;
using namespace Modelica::AST;

void usage()
{
  cout << "Usage causalize [options] <FILE>" << endl;
  cout << "Generates matching graph dot file for a given Modelica model." << endl;
  cout << endl;
  cout << "-h, --help      Display this information and exit" << endl;
  cout << "-o <path>, --output <path> Sets the output path for the generated graph dot file." << endl;
  cout << "-v, --version   Display version information and exit" << endl;
  cout << endl;
  cout << "Modelica C Compiler home page: https://github.com/CIFASIS/modelicacc " << endl;
}

void version()
{
  cout << "Modelica C Compiler 2.0" << endl;
  cout << "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.hpptml>" << endl;
  cout << "This is free software: you are free to change and redistribute it." << endl;
  cout << "There is NO WARRANTY, to the extent permitted by law." << endl;
}

int main(int argc, char** argv)
{
  int opt;
  extern char* optarg;
  string output_path = "";

  while (true) {
    static struct option long_options[] = {
        {"version", no_argument, 0, 'v'}, {"help", no_argument, 0, 'h'}, {"output", required_argument, 0, 'o'}, {0, 0, 0, 0}};
    int option_index = 0;
    opt = getopt_long(argc, argv, "vho:", long_options, &option_index);
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
    case 'o':
      output_path = optarg;
      break;
    case '?':
      usage();
      exit(-1);
      break;
    default:
      abort();
    }
  }

  StoredDef stored_def;
  bool status = false;
  std::string model_file = "";
  if (argv[optind] != nullptr) {
    model_file = argv[optind];
    stored_def = Parser::ParseFile(model_file, status);
  } else {
    std::cout << "No input file provided." << std::endl;
    usage();
    exit(-1);
  }

  if (!status) {
    std::cout << "Error parsing file " << model_file << std::endl;
    exit(-1);
  }

  Modelica::Logger::instance().setFile("SBG");

  Class ast_c = boost::get<Class>(stored_def.classes().front());
  MMO_Class mmo_class(ast_c);
  StateVariablesFinder setup_state_var(mmo_class);
  setup_state_var.findStateVariables();

  Modelica::Causalize::CausalizationResult result
    = Modelica::Causalize::Causalize{}.causalize(mmo_class);
  std::cout << result << "\n";

  return 0;
}
