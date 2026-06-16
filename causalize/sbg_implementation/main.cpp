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

#include "boost/variant/get.hpp"
#include <getopt.h>

#include "causalize/sbg_implementation/algebraic_loops_detection.hpp"
#include "causalize/sbg_implementation/generate_sbg_input.hpp"
#include "causalize/sbg_implementation/horizontal_sorting.hpp"
#include "causalize/sbg_implementation/tearing.hpp"
#include "mmo/mmo_class.hpp"
#include "parser/parser.hpp"
#include "util/ast_visitors/state_variables_finder.hpp"
#include "util/debug.hpp"
#include "util/logger.hpp"
#include <util/solve/solve.hpp>
#include <ast/equation.hpp>
#include <util/table.hpp>
// #include <causalize/graph_implementation/apply_tarjan.h>
// #include <causalize/graph_implementation/graph/graph_definition.h>

using namespace std;
using namespace Modelica;
using namespace Modelica::AST;

// Data structures for EquationSolver integration
std::list<std::string> c_code;
Modelica::AST::ClassList _cl;

// JSON parsing functions
std::string read_json_file(const std::string& filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open JSON file: " + filename);
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

Modelica::AST::EquationList parse_json_to_equations(const std::string& json_content) {
  // TODO: Implement JSON to EquationList conversion
  // This is a placeholder - you need to implement the actual parsing
  Modelica::AST::EquationList eqs;
  // Parse JSON and convert to Modelica equations
  return eqs;
}

Modelica::AST::ExpList parse_json_to_unknowns(const std::string& json_content) {
  // TODO: Implement JSON to ExpList conversion  
  // This is a placeholder - you need to implement the actual parsing
  Modelica::AST::ExpList unknowns;
  // Parse JSON and convert to variable expressions
  return unknowns;
}

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

  Modelica::Causalize::GenerateSBGInput gen_sbg_input(mmo_class);
  Modelica::Causalize::SBGGenerationInfo sbg_info
    = gen_sbg_input.buildFromModel();
  Modelica::Causalize::HorizontalSorting horizontal_sorter(sbg_info);
  Modelica::Causalize::HorizontalSortingInfo hs_info = horizontal_sorter.sort();
  std::cout << "Matching:\n" << hs_info.horizontal_sorting() << "\n"; 
  Modelica::Causalize::AlgebraicLoopsDetector loops_detector(hs_info);
  Modelica::Causalize::AlgebraicLoopsInfo loops_info = loops_detector.detect();
  std::cout << "Algebraic loops:\n" << loops_info.loops() << "\n";
  Modelica::Causalize::TearingDetector tearing_detector(loops_info);
  Modelica::Causalize::TearingVariables tearing_vars
    = tearing_detector.detect();
  std::cout << "Tearing variables:\n" << tearing_vars << "\n";

  debugInit("s");
  std::list<std::string> c_code;
  Modelica::AST::ClassList classes = stored_def.classes();
  std::string path = "test.c";
  EquationList causalized;
  for (const Modelica::Causalize::AlgebraicLoop& l : loops_info.loops()) {
    EquationList res = EquationSolver::Solve(l.equations(), l.variables(), mmo_class.syms_ref(), c_code, classes, path);
    causalized.insert(causalized.end(), res.begin(), res.end());
  }
  
  mmo_class.equations_ref().equations_ref() = causalized;  
  std::cout << mmo_class << std::endl;

  return 0;
}
