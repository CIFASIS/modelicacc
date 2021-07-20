/*****************************************************************************
 
    This file is part of the Set-Based Graph library.

******************************************************************************/

#pragma once

#include <boost/graph/adjacency_list.hpp>

#include <util/graph/sbg/defs.h>
#include <util/graph/sbg/interval.h>
#include <util/graph/sbg/multi_interval.h>
#include <util/graph/sbg/atomic_set.h>
#include <util/graph/sbg/set.h>
#include <util/graph/sbg/lmap.h>
#include <util/graph/sbg/atom_pw_map.h>
#include <util/graph/sbg/pw_map.h>

namespace SBG {

// Set-vertex --------------------------------------------------------------------------------------

#define SET_VERTEX_TEMPLATE       \
  template<typename DESCRIPTION>

#define SETV_TEMP_TYPE \
  SetVertexImp<DESCRIPTION>

SET_VERTEX_TEMPLATE
struct SetVertexImp {
  member_class(std::string, name); // For pretty-printing
  member_class(int, id); // Unique identifier
  member_class(Set, range); // Vertices of the set-vertex
  member_class(int, index); // For debugging
  member_class(DESCRIPTION, desc); // Description

  SetVertexImp();
  SetVertexImp(std::string name, Set range);
  SetVertexImp(std::string name, int id, Set range, int index, DESCRIPTION desc);

  eq_class(SetVertexImp);

  //size_t hash() { return id; }
};

printable_temp(SET_VERTEX_TEMPLATE, SETV_TEMP_TYPE);

typedef SetVertexImp<std::string> SetVertex;

//size_t hash_value(SetVertex v);
// Set-edge ----------------------------------------------------------------------------------------

struct SetEdge {
  SetEdge()
  {
    name = "";
    id = -1;
    PWLMap aux;
    es1 = aux;
    es2 = aux;
    index = 0;
  };
  SetEdge(std::string nm, PWLMap e1, PWLMap e2)
  {
    name = nm;
    id = -1;
    es1 = e1;
    es2 = e2;
    index = 0;
  };
  SetEdge(std::string nm, int i, PWLMap e1, PWLMap e2, int ind)
  {
    name = nm;
    id = i;
    es1 = e1;
    es2 = e2;
    index = ind;
  };

  PWLMap es1_() const { return es1; }

  PWLMap es2_() const { return es2; }

  int id_() { return id; }

  std::string name_() { return name; }

  std::string name;

  private:
  int id;
  PWLMap es1;
  PWLMap es2;
  int index;
};

std::ostream &operator<<(std::ostream &out, SetEdge &E);

// Set graph ---------------------------------------------------------------------------------------

typedef boost::adjacency_list<boost::listS, boost::listS, boost::undirectedS, SetVertex, SetEdge> SBGraph;
typedef SBGraph::vertex_descriptor SetVertexDesc;
typedef boost::graph_traits<SBGraph>::vertex_iterator VertexIt;
typedef SBGraph::edge_descriptor SetEdgeDesc;
typedef boost::graph_traits<SBGraph>::edge_iterator EdgeIt;
typedef boost::graph_traits<SBGraph>::out_edge_iterator OutEdgeIt;

}  // namespace SBG

