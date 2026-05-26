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

#include <boost/variant/get.hpp>
#include <getopt.h>

#include <causalize/sbg_implementation/generate_sbg_input.hpp>
#include <mmo/mmo_class.hpp>
#include <parser/parser.hpp>
#include <util/ast_visitors/state_variables_finder.hpp>
#include <util/debug.hpp>
#include <util/logger.hpp>
#include <util/solve/solve.hpp>
#include <ast/equation.hpp>
#include <util/table.hpp>
#include <causalize/graph_implementation/apply_tarjan.h>
#include <causalize/graph_implementation/graph/graph_definition.h>
#include <fstream>
#include <sstream>

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

  gen_sbg_input.buildFromModel();

  /// Temp hack to test the binaries, hardcoded paths should go on config files.
  const std::string CAUSALIZE = "./3rd-party/sbg/sb-graph-dev/build/eval/sbg-eval ";
  const std::string DEFAULT_CAUSALIZED_JSON = "./output.json";
  const std::string CAUSALIZED_JSON = mmo_class.name() + "_causalized.json";
  std::string ARGS = gen_sbg_input.fileName() + " > " + mmo_class.name() + "_causalized.sbg; mv " + DEFAULT_CAUSALIZED_JSON + " ./" + CAUSALIZED_JSON;
  const std::string CAUSALIZE_CMD = CAUSALIZE + ARGS;

  int res = std::system(CAUSALIZE_CMD.c_str());

  std::cout << "Result: " << res << std::endl;

  // Check if SBG processing succeeded
  if (res != 0) {
    std::cerr << "SBG processing failed with code: " << res << std::endl;
    return res;
  }

  // Validate JSON file exists
  std::ifstream json_file(CAUSALIZED_JSON);
  if (!json_file.good()) {
    std::cerr << "Failed to open SBG output file: " << CAUSALIZED_JSON << std::endl;
    return -1;
  }

  try {
    // Read and parse JSON output from SBG
    std::string json_content = read_json_file(CAUSALIZED_JSON);
    
    // Convert JSON results to EquationList and ExpList
    Modelica::AST::EquationList causalized_eqs = parse_json_to_equations(json_content);
    Modelica::AST::ExpList unknowns = parse_json_to_unknowns(json_content);

    // Apply EquationSolver to generate C code
    std::stringstream output_path;
    output_path << mmo_class.name() << ".c";
    Modelica::AST::EquationList final_eqs = EquationSolver::Solve(
        causalized_eqs, 
        unknowns, 
        mmo_class.syms_ref(), 
        c_code, 
        _cl, 
        output_path.str()
    );

    std::cout << "EquationSolver processing completed successfully." << std::endl;
    std::cout << "Generated C code in: " << output_path.str() << std::endl;

  } catch (const std::exception& e) {
    std::cerr << "Error during EquationSolver integration: " << e.what() << std::endl;
    return -1;
  }

  return res;
}
