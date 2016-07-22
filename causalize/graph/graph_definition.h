#ifndef GRAPH_DEFINITION_
#define GRAPH_DEFINITION_

#include <iostream>
#include <utility>
#include <boost/config.hpp>
#include <boost/graph/adjacency_list.hpp>

#include <ast/equation.h>

namespace Causalize {
  /// @brief Vertex in the incidence graph can be either Equations or Unknowns. This type is used for distinguish between them
  enum VertexType {E, U};

  /// @brief This is the property for a vertex in the incidence graph. Nodes can be of two types: Equation or Unknow.
  struct VertexProperty {
	  VertexType type;
  /// @brief This is used for debugin purposes
	  int  index; 
  /// @brief This list holds the equations in the case of a Equation node. At the beginign the list contains only one equation. In the case a loop is found some nodes are collapsed
	  Modelica::AST::EquationList  eqs;
  /// @brief This list holds the unknows in the case of a Unknown node. At the beginign the list contains only one unknown. In the case a loop is found some nodes are collapsed
	  Modelica::AST::ExpList unknowns;

    bool visited;
  };

  /// @brief Empty edge properties for incidence graph 
  struct EdgeProperty {
    friend std::ostream & operator << (std::ostream &os, const EdgeProperty &ep) {
      os << "";
      return os;
    }
  };
  
  /// @brief This is the definition of the Incidence graph for the scalar case.
  typedef boost::adjacency_list<boost::listS, boost::listS, boost::undirectedS, VertexProperty,EdgeProperty> CausalizationGraph;
  /// @brief A vertex of the Incidence graph
  typedef Causalize::CausalizationGraph::vertex_descriptor Vertex;
  /// @brief An equation vertex is the same as a regular vertex
  typedef Vertex EquationVertex;
  /// @brief An unknown vertex is the same as a regular vertex
  typedef Vertex UnknownVertex;
  /// @brief This is an edge of the sacalar causalization graph
  typedef CausalizationGraph::edge_descriptor Edge;
}
#endif
