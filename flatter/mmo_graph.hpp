
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

#include <boost/config.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <ast/expression.h>
#include <util/table.h>

#ifndef MMOGRAPH_DEFINITION_
#define MMOGRAPH_DEFINITION_

template <typename T>
std::vector<T> toVector(std::set<T> v)
{
  std::vector<T> output;
  std::copy(v.begin(), v.end(), std::back_inserter(output));
  return output;
}

using namespace Modelica;
using namespace Modelica::AST;

struct VertexProperty {
  public:
  OptExp name;
  bool hasIndex;
  OptExp range;
  OptExpList free;
};

struct EdgeProperty {
  public:
  OptExp index;
  bool visited;
};

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, VertexProperty, EdgeProperty> MMOGraph;

typedef MMOGraph::vertex_descriptor Vertex;
typedef MMOGraph::edge_descriptor Edge;
typedef Option<Vertex> OptVertex;
typedef Option<Edge> OptEdge;
typedef boost::graph_traits<MMOGraph>::vertex_iterator MMOGraphVertexIteretor;
typedef boost::graph_traits<MMOGraph>::edge_iterator MMOGraphEdgeIteretor;
typedef boost::graph_traits<MMOGraph>::out_edge_iterator MMOGraphOutputEdgeIteretor;

#endif
