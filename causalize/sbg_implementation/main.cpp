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
#include <iostream>
#include "boost/variant/get.hpp"
#include <getopt.h>

#include "causalize/sbg_implementation/causalize.hpp"
#include "mmo/mmo_class.hpp"
#include "parser/parser.hpp"
#include "util/ast_visitors/state_variables_finder.hpp"
#include "util/debug.hpp"
#include "util/logger.hpp"
#include <util/solve/solve.hpp>
#include <ast/equation.hpp>
#include <util/table.hpp>
#include <sbg/pwmap_impl.hpp>
#include <sbg/set_impl.hpp>
#include <util/time_profiler.hpp>

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
  cout << "-t, --tearing Use tearing variables." << endl;
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
  bool tearing = false;

  while (true) {
    static struct option long_options[] = {{"version", no_argument, 0, 'v'},
                                           {"help", no_argument, 0, 'h'},
                                           {"output", required_argument, 0, 'o'},
                                           {"tearing", no_argument, 0, 't'},
                                           {0, 0, 0, 0}};
    int option_index = 0;
    opt = getopt_long(argc, argv, "vhot:", long_options, &option_index);
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
    case 't':
      tearing = true;
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

  SBG::LIB::SET_IMPL.set_set_fact(SBG::LIB::SetKind::kOrdUnidimDense);
  SBG::LIB::PWMAP_IMPL.set_pwmap_fact(SBG::LIB::PWMapKind::kUnordered);
  Modelica::Causalize::CausalizationResult result;
  EquationList causalized;
  {
    SBG::Util::Internal::TimeProfiler profiler{"Causalization"};
    result = Modelica::Causalize::Causalize{}.causalize(mmo_class);

    //debugInit("s");
    //{
    //  SBG::Util::Internal::TimeProfiler solve_profiler{"GiNaC solve"};
    //  Modelica::AST::ClassList classes = stored_def.classes();
    //  Modelica::Causalize::CausalModel causal_model = result.vertical_sort();
    //  Modelica::Causalize::TearingVariables tearing_vars;
    //  if (tearing) {
    //    tearing_vars = result.tearing();
    //  }
    //  for (const Modelica::Causalize::SortedAlgebraicLoop& s : causal_model) {
    //    EquationList res =
    //        EquationSolver::Solve(s.equations(), s.variables(), mmo_class.syms_ref(), mmo_class.variables_ref(), classes, tearing_vars);
    //    causalized.insert(causalized.end(), res.begin(), res.end());
    //  }
    //}
  }
  mmo_class.equations_ref().equations_ref() = causalized;

  std::cout << result << "\n";
  std::cout << mmo_class << std::endl;

  std::string causalized_file_name = mmo_class.name();

  causalized_file_name.append("_causalized.mo");

  std::ofstream out_stream(causalized_file_name);

  static constexpr std::string_view annotation_string = R"(annotation(
  experiment(
    MMO_Description="",
    MMO_Solver=DASSL,
    Jacobian=Dense,
    MMO_BDF_PDepth=1,
    MMO_BDF_Max_Step=0,
    StartTime=0.0,
    StopTime=20,
    Tolerance={1e-3},
    AbsTolerance={1e-3}
  ));)";

  std::stringstream buffer;
  buffer << mmo_class;
  std::string content = buffer.str();

  size_t last_pos = content.rfind("end");

  if (last_pos != std::string::npos) {
    content.insert(last_pos, std::string(annotation_string) + "\n");
  }

  if (out_stream.is_open()) {
    out_stream << content << std::endl;
  } else {
    std::cerr << "Error: Could not open file " << causalized_file_name << " for writing." << std::endl;
  }

  std::cout << "\n";
  SBG::Util::Internal::TimeProfiler::print_execution_time("Horizontal sorting SBG builder");
  SBG::Util::Internal::TimeProfiler::print_execution_time("Horizontal sorting");
  SBG::Util::Internal::TimeProfiler::print_execution_time("Algebraic loops SBG builder");
  SBG::Util::Internal::TimeProfiler::print_execution_time("Algebraic loops detection");
  SBG::Util::Internal::TimeProfiler::print_execution_time("Tearing SBG builder");
  SBG::Util::Internal::TimeProfiler::print_execution_time("Tearing");
  SBG::Util::Internal::TimeProfiler::print_execution_time("Vertical sorting SBG builder");
  SBG::Util::Internal::TimeProfiler::print_execution_time("Vertical sorting");
  SBG::Util::Internal::TimeProfiler::print_execution_time("GiNaC solve");
  SBG::Util::Internal::TimeProfiler::print_execution_time("Causalization");
  std::cout << "\n\n";

  return 0;
}
