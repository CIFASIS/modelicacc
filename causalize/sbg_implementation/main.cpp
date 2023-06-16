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

#include <causalize/sbg_implementation/matching_graph_builder.h>
#include <mmo/mmo_class.h>
#include <parser/parser.h>
#include <util/ast_visitors/state_variables_finder.h>
#include <util/debug.h>
#include <util/logger.h>
//#include <sbg/graph_builders/matching_graph_builder.hpp>
#include <sbg/descs.hpp>
#include <sbg/graph_builders/order_graph_builder.hpp>
#include <sbg/graph_builders/scc_graph_builder.hpp>
#include <sbg/sbg.hpp>
#include <sbg/sbg_algorithms.hpp>
#include <sbg/sbg_printer.hpp>

using namespace std;
using namespace Modelica;
using namespace Modelica::AST;
using namespace Causalize;

void usage()
{
  cout << "Usage causalize [options] file" << endl;
  cout << "Generates matching graph dot file for a given Modelica model file." << endl;
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
  cout << "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>" << endl;
  cout << "This is free software: you are free to change and redistribute it." << endl;
  cout << "There is NO WARRANTY, to the extent permitted by law." << endl;
}

void print_result(MatchingStruct m, SCCStruct scc, OrderStruct o, VertexOrder vo)
{
  SBG::IO::UndirectedConverter uc(m.g());
  SBG::IO::GraphIO g_io = uc.convert_graph();
  BOOST_FOREACH (Set s, vo) {
    BOOST_FOREACH (MultiInterval mi, s.asets()) {
      SBG::Set represented = scc.rmap().preImage(Set(mi));
      
      SBG::IO::MatchingIO s_io = uc.convert_matching(represented);
      std::cout << s_io << "\n";
    }
  }

  return;
}

int main(int argc, char **argv)
{
  int opt;
  extern char* optarg;
  string output_path = "";

  while (true) {
    static struct option long_options[] = {{"version", no_argument, 0, 'v'},
                                           {"help", no_argument, 0, 'h'},
                                           {"output", required_argument, 0, 'o'},
                                           {0, 0, 0, 0}};
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
  if (argv[optind] != nullptr) {
    stored_def = Parser::ParseFile(argv[optind], status);
  } 

  if (!status) {
    return -1;
  }

  Modelica::Logger::instance().setFile("SBG");

  Class ast_c = boost::get<Class>(stored_def.classes().front());
  MMO_Class mmo_class(ast_c);
  StateVariablesFinder setup_state_var(mmo_class);
  setup_state_var.findStateVariables();

  MatchingGraphBuilder matching_graph_builder(mmo_class);

  SBG::SBGraph matching_graph = matching_graph_builder.makeGraph();
  SBG::GraphPrinter printer(matching_graph, 0);

  printer.printGraph(output_path+mmo_class.name()+".dot");
  
  MatchingStruct match(matching_graph);
  pair<SBG::Set, bool> res = match.SBGMatching();

  LOG << "\n" << matching_graph << "\n";
  LOG << res.first << "\n";

  SCCGraphBuilder scc_builder(match);
  scc_builder.build();
  DSBGraph scc_graph = scc_builder.result();
  SCCStruct scc_struct(scc_graph);
  PWLMap scc = scc_struct.SBGSCC();

  LOG << scc_graph << "\n";
  LOG << scc << "\n";

  OrderGraphBuilder order_builder(scc_struct);
  order_builder.build();
  DSBGraph order_graph = order_builder.result();
  OrderStruct order_struct(order_graph);
  VertexOrder order = order_struct.order();

  LOG << order_graph << "\n";
  LOG << order << "\n";

  SBG::IO::DirectedConverter dc(order_graph);
  SBG::IO::GraphIO g_io = dc.convert_graph();
  SBG::IO::VertexOrderIO order_io = dc.convert_vertex_order(order);

  BOOST_FOREACH (SetVertexDesc vd, vertices(matching_graph)) {
    SBG::SetVertex v = matching_graph[vd];
    std::cout << v.id() << ": " << v.desc().text() << "\n";
  }
  std::cout << "\n";
  print_result(match, scc_struct, order_struct, order);
  return 0;
}
