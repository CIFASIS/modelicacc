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

#ifndef VECTOR_GRAPH_DEFINITION_
#define VECTOR_GRAPH_DEFINITION_

#include <utility>
#include <set>
#include <boost/config.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/icl/interval_set.hpp>

#include <ast/equation.h>
#include <causalize/graph/graph_definition.h>

#include <ast/ast_types.h>
#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <sstream>
#include <string>

namespace ICL = boost::icl;
namespace Causalize {
  /// @brief This is the property for a vertex in the incidence graph. Nodes can be of two types: Equation or Unknow.

  struct VectorUnknown: Unknown {
    int dimension;
    std::vector<int> dimensionList;
    VectorUnknown(){};
    VectorUnknown(VarInfo varInfo, Modelica::AST::Reference var);
    void SetIndex(Modelica::AST::ExpList index);
  };

  struct VectorVertexProperty: VertexProperty {
  /// @brief The number of equations or unknowns left to causalize in this node
    int count;
    VectorUnknown unknown;
  };

  /// @brief A pair representing a usage of a variable in an equation
  typedef ICL::interval_set<int> Interval;
  inline Interval CreateInterval(int a, int b) {
    Interval ret;
    return ret.insert(ICL::discrete_interval<int>(a,b, ICL::interval_bounds::closed()));
  }
  typedef std::list<Interval> IntervalList;
  typedef std::pair<IntervalList, std::vector<int> > IntervalListUsage;
  typedef std::pair<IntervalListUsage, IntervalListUsage> IndexPair;
  inline IndexPair CreateIndexPair(Interval a, Interval b, std::vector<int> ua, std::vector<int> ub) {
    return make_pair(make_pair(std::list<Interval>(1,a),ua) , make_pair(std::list<Interval>(1,b),ub));
  }
 // typedef std::pair<std::list<int>, std::list<int> > IndexPair;
  std::ostream& operator<<(std::ostream &os, const IndexPair &ip);
  typedef std::set<IndexPair> IndexPairSet;
  std::ostream& operator<<(std::ostream &os, const IndexPairSet &ips);

  struct VectorEdgeProperty {

    friend std::ostream& operator<<(std::ostream &os, const VectorEdgeProperty &ep);
    IndexPairSet labels;

    std::set<std::list<int> > getDom() const;
    std::set<std::list<int> > getRan() const;
/// @brief This function removes a set of pairs from this Edge
/// @param ips set of pairs to remove
    void RemovePairs(IndexPairSet ips);
    void RemoveUnknowns(IndexPairSet ips_remove);
    void RemoveEquations(IndexPairSet ips_remove);
    inline bool IsEmpty() { return labels.size()==0; }

  };

  /// @brief This is the definition of the Incidence graph for the vector case.
  typedef boost::adjacency_list<boost::listS, boost::listS, boost::undirectedS, VectorVertexProperty, VectorEdgeProperty> VectorCausalizationGraph;
  /// @brief This a node from the vectorized incidence graph
  typedef Causalize::VectorCausalizationGraph::vertex_descriptor VectorVertex;
  /// @brief An equation vertex is the same as a regular vertex
  typedef VectorVertex VectorEquationVertex;
  /// @brief An unknown vertex is the same as a regular vertex
  typedef VectorVertex VectorUnknownVertex;
  /// @brief This is an edge of the vectorized causalization graph
  typedef VectorCausalizationGraph::edge_descriptor VectorEdge;
  /// @brief This struct represents a set of causalized vars for the vector algorithm

  struct CausalizedVar{
    VectorUnknown unknown;
    Equation equation;
    IndexPairSet pairs;
  };
}
#endif
