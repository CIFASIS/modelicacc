#ifndef VECTOR_GRAPH_DEFINITION_
#define VECTOR_GRAPH_DEFINITION_

#include <utility>
#include <boost/config.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/icl/interval_set.hpp>

#include <ast/equation.h>
#include <mmo/mmo_class.h>
#include <causalize/graph/graph_definition.h>

namespace Causalize {
  /// @brief This is the property for a vertex in the incidence graph. Nodes can be of two types: Equation or Unknow.
  struct VectorVertexProperties: VertexProperties {
  /// @brief The number of equations or unknowns left to causalize in this node
    int count;
  };
  
  /*
  * genericIndex: if the edge represents an occurrence of the form 
  * a[i-1] in some equation, then this list contains the expression: i-1.
  * indexes: it contains the number of indexes of a variable (in case it is
  * an array) that are used in the equation connected by the edge.
  */
  struct VectorEdgeProperties{
	  //std::pair<int, int> genericIndex;
	  boost::icl::interval_set<int> p_e;
	  boost::icl::interval_set<int> p_v;
    bool operator<  (const VectorEdgeProperties & rhs) const {
      if (p_e <rhs.p_e) return true;
      if (p_v < rhs.p_v ) return true; 
      return false;
    }
    bool isBalanced() {
      return p_e.size()==p_v.size();
    }
    friend std::ostream & operator << (std::ostream &os, const VectorEdgeProperties &ep) {
      os << ep.p_e << "->" << ep.p_v;
      return os;
    }
  };

  /// @brief This is the definition of the Incidence graph for the vector case.
  typedef boost::adjacency_list<boost::listS, boost::listS, boost::undirectedS, VectorVertexProperties, VectorEdgeProperties> VectorCausalizationGraph;
  /// @brief This a node from the vectorized indicdence graph
  typedef Causalize::VectorCausalizationGraph::vertex_descriptor VectorVertex;
  /// @brief An equation vertex is the same as a regular vertex
  typedef VectorVertex VectorEquationVertex;
  /// @brief An unknown vertex is the same as a regular vertex
  typedef VectorVertex VectorUnknownVertex;
  /// @brief This is an edge of the vectorized causalization graph
  typedef VectorCausalizationGraph::edge_descriptor VectorEdge;
  /// @brief This struct represents a set of causalized vars for the vector algorithm
  struct CausalizedVar{
    VectorVertexProperties unknown;
    VectorVertexProperties equation;
    VectorEdgeProperties edge;
  };
}
#endif
