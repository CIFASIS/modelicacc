#ifndef VECTOR_GRAPH_DEFINITION_
#define VECTOR_GRAPH_DEFINITION_

#include <utility>
#include <set>
#include <boost/config.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/icl/interval_set.hpp>

#include <ast/equation.h>
#include <mmo/mmo_class.h>
#include <causalize/graph/graph_definition.h>

namespace Causalize {
  /// @brief This is the property for a vertex in the incidence graph. Nodes can be of two types: Equation or Unknow.
  struct VectorVertexProperty: VertexProperty {
  /// @brief The number of equations or unknowns left to causalize in this node
    int count;
  };
  
  /// @brief A pair representing a usage of a variable in an equation
  typedef std::pair<int,int> IndexPair;
  typedef std::set<IndexPair> IndexPairSet;

  struct VectorEdgeProperty {
	  //TODO: remove old fields
	  boost::icl::interval_set<int> p_e;
	  boost::icl::interval_set<int> p_v;
    bool operator<  (const VectorEdgeProperty & rhs) const {
      if (p_e <rhs.p_e) return true;
      if (p_v < rhs.p_v ) return true; 
      return false;
    }
    bool isBalanced() {
      return p_e.size()==p_v.size();
    }

    friend std::ostream & operator << (std::ostream &os, const VectorEdgeProperty &ep) {
      os << "{";
      foreach_(IndexPair ip, ep.labels) 
        os << "(" << ip.first << "," << ip.second << ") ";
      os << "}";
      return os;
    }
    
    /* This is the new version */
    IndexPairSet labels;
/// @brief This function removes a set of pairs from this Edge
///
/// @param ips set of pairs to remove
    void RemovePairs(IndexPairSet ips) {
      foreach_(IndexPair ip, ips) 
        labels.erase(ip);
    } 
    void RemoveUnknonws(IndexPairSet ips_remove) {
      IndexPairSet new_labels;
      foreach_(IndexPair ip, labels)  {
        bool found=false;
        foreach_(IndexPair ip_remove, ips_remove)  {
          if (ip_remove.second == ip.second) {
            found=true;
            break;
          }
        }
        if (!found)
          new_labels.insert(ip);
      }
      labels = new_labels;
    } ;
    void RemoveEquations(IndexPairSet ips_remove) {
      IndexPairSet new_labels;
      foreach_(IndexPair ip, labels)  {
        bool found=false;
        foreach_(IndexPair ip_remove, ips_remove)  {
          if (ip_remove.first == ip.first) {
            found=true;
            break;
          }
        }
        if (!found)
          new_labels.insert(ip);
      }
      labels = new_labels;
    }
    bool isEmpty() {
      return labels.size()==0;
    }
  };

  /// @brief This is the definition of the Incidence graph for the vector case.
  typedef boost::adjacency_list<boost::listS, boost::listS, boost::undirectedS, VectorVertexProperty, VectorEdgeProperty> VectorCausalizationGraph;
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
    VectorVertexProperty unknown;
    VectorVertexProperty equation;
    //TODO: Remove this prop
    VectorEdgeProperty edge;
    IndexPairSet pairs;
  };
}
#endif
