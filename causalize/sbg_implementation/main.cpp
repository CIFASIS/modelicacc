
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

#include <causalize/sbg_implementation/matching_graph_builder.h>
#include <mmo/mmo_class.h>
#include <parser/parser.h>
#include <util/debug.h>
#include <util/graph/graph_definition.h>
#include <util/graph/graph_printer.h>

using namespace std;
using namespace Modelica;
using namespace Modelica::AST;
using namespace Causalize;

int main(int argc, char **argv)
{
  bool status;
  int opt;
  while ((opt = getopt(argc, argv, "d")) != -1) {
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

  StoredDef stored_def;
  if (argv[optind] != NULL) {
    stored_def = Parser::ParseFile(argv[optind], status);
  } else {
    stored_def = Parser::ParseFile("", status);
  }

  if (!status) {
    return -1;
  }

  Class ast_c = boost::get<Class>(stored_def.classes().front());
  MMO_Class mmo_class(ast_c);
  
  MatchingGraphBuilder matching_graph_builder(mmo_class);

  SBG::SBGraph matching_graph = matching_graph_builder.makeGraph();
  SBG::GraphPrinter printer(matching_graph, 0);

  printer.printGraph(mmo_class.name()+".dot");
  return 0;
}
