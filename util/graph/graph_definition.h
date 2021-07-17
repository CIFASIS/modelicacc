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

/*! \file graph_definition.h
 *   General library to work with set based graphs.
 *   The main goal of this module was to abstract
 *   classes that describe mathematical entities,
 *   such as sets, graphs, etc.
 *   If implementation has to change, a new concrete
 *   class can be defined, but the client modules
 *   of graph_definition will call abstract classes,
 *   so no change is needed in the rest of the compiler.
 *   Through typename template the abstract class uses
 *   the new concrete implementation, therefore, requiring
 *   little change appart from concrete class implementation.
 *
 *   An example will be commented for intervals.
 */

#ifndef GRAPH_DEFINITION_
#define GRAPH_DEFINITION_

#include <iostream>
#include <list>
#include <map>
#include <math.h>
#include <utility>

#include <boost/config.hpp>
#include <boost/foreach.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/unordered_set.hpp>

#include <util/graph/sbg/defs.h>
#include <util/graph/sbg/interval.h>
#include <util/graph/sbg/multi_interval.h>
#include <util/graph/sbg/atomic_set.h>
#include <util/graph/sbg/set.h>
#include <util/graph/sbg/lmap.h>
#include <util/graph/sbg/atom_pw_map.h>
#include <util/graph/sbg/pw_map.h>

namespace SBG {

// Function declarations ---------------------------------------------------------------------------

bool equivalentPW(PWLMap pw1, PWLMap pw2);

PWLMap minAtomPW(AtomSet &dom, LMap &lm1, LMap &lm2);
PWLMap minPW(Set &dom, LMap &lm1, LMap &lm2);
PWLMap minMap(PWLMap &pw1, PWLMap &pw2);
PWLMap minInv(PWLMap &pw, Set &s);

PWLMap reduceMapN(PWLMap pw, int dim);

PWLMap mapInf(PWLMap pw);

PWLMap minAdjCompMap(PWLMap pw3, PWLMap pw2, PWLMap pw1);
PWLMap minAdjCompMap(PWLMap pw2, PWLMap pw1);

PWLMap minAdjMap(PWLMap pw3, PWLMap pw2, PWLMap pw1);
PWLMap minAdjMap(PWLMap pw2, PWLMap pw1);

/*-----------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
// Graph definition
/*-----------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/

struct SetVertex {
  SetVertex()
  {
    name = "";
    id = -1;
    Set aux;
    vs = aux;
    index = 0;
  };
  SetVertex(std::string n, Set v)
  {
    name = n;
    id = -1;
    vs = v;
    index = 0;
  };
  SetVertex(std::string n, int i, Set v, int ind)
  {
    name = n;
    id = i;
    vs = v;
    index = ind;
  };

  Set vs_() { return vs; }

  std::string name_() { return name; }

  bool operator==(const SetVertex &other) const { return id == other.id; }

  size_t hash() { return id; }

  // For pretty-printing
  std::string name;

  private:
  // Unique identifier
  int id;
  // Set vertex
  Set vs;
  // For debugging
  int index;
};

size_t hash_value(SetVertex v);

std::ostream &operator<<(std::ostream &out, SetVertex &V);

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

typedef boost::adjacency_list<boost::listS, boost::listS, boost::undirectedS, SetVertex, SetEdge> SBGraph;
typedef SBGraph::vertex_descriptor SetVertexDesc;
typedef boost::graph_traits<SBGraph>::vertex_iterator VertexIt;
typedef SBGraph::edge_descriptor SetEdgeDesc;
typedef boost::graph_traits<SBGraph>::edge_iterator EdgeIt;
typedef boost::graph_traits<SBGraph>::out_edge_iterator OutEdgeIt;

}  // namespace SBG

#endif
