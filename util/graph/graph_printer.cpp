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

#include <util/graph/graph_printer.h>

#include <list>
#include <iostream>
#include <fstream>
#include <sstream>

#include <boost/graph/adjacency_list.hpp>
#include <boost/lexical_cast.hpp>

#include <util/graph/graph_definition.h>

using namespace std;

#define MAKE_SPACE \
  for (int __i = 0; __i < depth; __i++) stri << " ";
#define TAB_SPACE 2
#define INSERT_TAB depth += TAB_SPACE;
#define DELETE_TAB depth -= TAB_SPACE;

namespace SBG {

GraphPrinter::GraphPrinter(const SBGraph &g, const int mod) : graph(g), mode(mod), depth(0){};

void GraphPrinter::printGraph(std::string name)
{
  stringstream stri;
  ofstream out(name.c_str());

  stri << "digraph G{" << endl;
  INSERT_TAB
  MAKE_SPACE
  stri << "  ratio=\"fill\"" << endl;
  MAKE_SPACE
  stri << "  node[shape=\"ellipse\"]" << endl;
  INSERT_TAB
  MAKE_SPACE

  // Vertices printing
  printVertices(stri);
  stri << "\n";

  DELETE_TAB
  DELETE_TAB

  INSERT_TAB
  INSERT_TAB
  stringstream colors;

  DELETE_TAB
  stri << colors.str();
  DELETE_TAB

  INSERT_TAB
  INSERT_TAB

  // Edge printing
  printEdges(stri);

  DELETE_TAB
  DELETE_TAB
  stri << "\n";
  stri << "}" << endl;
  out << stri.str();
  out.close();
}

void GraphPrinter::printVertices(stringstream &stri)
{
  switch (mode) {
  case 1:  // Flatter
    break;
  case 2:  // SetBased
    break;
  case 3:  // Compacted
    break;
  default:
    VertexIt vi, vi_end;
    for (boost::tie(vi, vi_end) = boost::vertices(graph); vi != vi_end; ++vi) {
      stri << vPrinter(graph[*vi]) << " [label=\"" << vLabelPrinter(graph[*vi]) << "\"]";
      stri << "\n";
      MAKE_SPACE
    }
  }
}

void GraphPrinter::printEdges(stringstream &stri)
{
  switch (mode) {
  case 1:  // Flatter
    break;
  case 2:  // SetBased
    break;
  case 3:  // Compacted
    break;
  default:
    EdgeIt ei, ei_end;
    for (boost::tie(ei, ei_end) = boost::edges(graph); ei != ei_end; ++ei) {
      SetVertexDesc v1 = boost::source(*ei, graph);
      SetVertexDesc v2 = boost::target(*ei, graph);
      stri << vPrinter(graph[v1]) << " -> " << vPrinter(graph[v2]);
      stri << " [label=\"" << ePrinter(graph[*ei]) << "\", arrowhead=\"none\"]";
      stri << "\n";
      MAKE_SPACE
    }
  }
}

std::string GraphPrinter::vPrinter(SetVertex v) { return v.name_(); }

std::string GraphPrinter::vLabelPrinter(SetVertex v)
{
  std::stringstream label;
  string nm = v.name_();
  SBG::Set dom = v.vs_();
  label << "{ " << nm << " Dom: " << dom << " }";
  return label.str();
}

std::string GraphPrinter::ePrinter(SetEdge e)
{
  std::stringstream label;
  PWLMap mapF = e.es1_();
  PWLMap mapU = e.es2_();
  label << "{ " << e.name << " mapF: " << mapF << " mapU: " << mapU << " }";
  return label.str();
}

}  // namespace SBG
