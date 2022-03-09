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

#include <chrono>
#include <string>
#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/max_cardinality_matching.hpp>
#include <boost/variant/get.hpp>
#include <cassert>
#include <getopt.h>


#include <causalize/sbg_implementation/matching_graph_builder.h>
#include <mmo/mmo_class.h>
#include <parser/parser.h>
#include <util/ast_visitors/state_variables_finder.h>
#include <util/debug.h>
#include <sbg/sbg.hpp>
#include <sbg/sbg_algorithms.hpp>
#include <sbg/sbg_printer.hpp>
#include <sbg/graph_builders/ordinary_graph_builder.hpp>

using namespace boost;
using namespace std;

using namespace Modelica;
using namespace Modelica::AST;
using namespace Causalize;

void usage()
{
  cout << "Usage: boost-cpm [options] file" << endl;
  cout << "Computes matching for a given Modelica model file using SBG and boost matching algorithms and shows the execution time of both." << endl;
  cout << endl;
  cout << "-d, --debug      Print debug information about graph generation." << endl;
  cout << "-h, --help       Display this information and exit" << endl;
  cout << "-o <path>, --output <path> Sets the output path for the generated graph dot file." << endl;
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

//===============================================================================
// Based on the example code from Aaron Windsor: 
//
// https://www.boost.org/doc/libs/1_46_0/libs/graph/example/matching_example.cpp
//
//===============================================================================

void computeMaxCardinalityMatching(SBG::SBGraph sb_graph, std::string dot_file, bool debug)
{

  OG::OrdinaryGraphBuilder ordinary_graph_builder(sb_graph);

  OG::Graph graph = ordinary_graph_builder.build();

  if (debug) {
    OG::GraphPrinter printer(graph);
    printer.printGraph(dot_file+"_boost.dot");
  }

  int n_vertices = num_vertices(graph);

  std::vector<graph_traits<OG::Graph>::vertex_descriptor> mate(n_vertices);
  auto begin = std::chrono::high_resolution_clock::now();
  checked_edmonds_maximum_cardinality_matching(graph, &mate[0]);
  auto end = std::chrono::high_resolution_clock::now();
  auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);

  if (debug) {
    std::cout << std::endl << "Boost: Found a matching of size " << matching_size(graph, &mate[0]) << std::endl;

    std::cout << "The matching is:" << std::endl;
    
    graph_traits<OG::Graph>::vertex_iterator vi, vi_end;
    for(tie(vi,vi_end) = vertices(graph); vi != vi_end; ++vi)
      if (mate[*vi] != graph_traits<OG::Graph>::null_vertex() && *vi < mate[*vi])
        std::cout << "{" << *vi << ", " << mate[*vi] << "}" << std::endl;

    std::cout << std::endl;
  }

  std::cout << "Boost Edmonds Maximum cardinality matching time: " << milliseconds.count() << " " << std::endl;
}

int main(int argc, char **argv)
{
  int opt;
  extern char* optarg;
  string output_path = "";
  bool debug = false;

  while (true) {
    static struct option long_options[] = {{"version", no_argument, 0, 'v'},
                                           {"help", no_argument, 0, 'h'},
                                           {"debug", no_argument, 0, 'd'},
                                           {"output", required_argument, 0, 'o'},
                                           {0, 0, 0, 0}};
    int option_index = 0;
    opt = getopt_long(argc, argv, "vhdo:", long_options, &option_index);
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
    case 'd':
      debug = true;
      break;
    case 'o':
      output_path = optarg;
      break;
    default:
      abort();
    }
  }

  StoredDef stored_def;
  bool status = false;
  if (argv[optind] != nullptr) {
    stored_def = Parser::ParseFile(argv[optind], status);
  } 

  if (!status) {
    return -1;
  }

  Class ast_c = boost::get<Class>(stored_def.classes().front());
  MMO_Class mmo_class(ast_c);
  StateVariablesFinder setup_state_var(mmo_class);
  setup_state_var.findStateVariables();

  MatchingGraphBuilder matching_graph_builder(mmo_class);

  SBG::SBGraph matching_graph = matching_graph_builder.makeGraph();
  SBG::GraphPrinter printer(matching_graph, 0);

  std::string dot_file = output_path+mmo_class.name();

  printer.printGraph(dot_file+".dot");

  MatchingStruct match(matching_graph);
  auto begin = std::chrono::high_resolution_clock::now();
  pair<SBG::Set, bool> res = match.SBGMatching();
  auto end = std::chrono::high_resolution_clock::now();
  auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);

  cout << "Generated matching:\n";
  cout << get<0>(res) << "\n\n";
  if (get<1>(res))
     cout << ">>> Matched all unknowns\n";
  std::cout << "SBG Algorithn matching time: " << milliseconds.count() << std::endl;

  computeMaxCardinalityMatching(matching_graph, dot_file, debug);

  return 0;
}
