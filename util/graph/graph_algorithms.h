
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

#include <iostream>
#include <list>
#include <map>
#include <math.h>
#include <utility>

#include <boost/config.hpp>
#include <boost/foreach.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/unordered_set.hpp>

#include <util/debug.h>
#include <util/graph/graph_definition.h>
#include <util/table.h>

using namespace SBG;

// Connected components of undirected SBGraphs ---------------------------------------------------

PWLMap connectedComponents(SBGraph g);

// Matching of undirected SBGraphs ---------------------------------------------------------------

typedef OrdCT<Set> SetPath;

//typedef VertexMap property_map<SBGraph, > // For access to set-vertices

struct MatchingStruct{
  MatchingStruct(SBGraph g);

  Set SBGMatching();

  Set recursion(PWLMap smap, int iters, Set E);
  void minReachable(PWLMap m_map, PWLMap map_D, PWLMap map_B, Set E);
  //void recursion();

  private:
  SBGraph g;
  Set F;
  Set U;

  Set allEdges;
  Set allVertices;

  PWLMap mapF; // "Left" maps
  PWLMap mapU; // "Right" maps
  PWLMap mapD; // Forward direction
  PWLMap mapB; // Backward direction

  Set matchedV;
  Set matchedE; 
  Set Ed;

  PWLMap mmap;

  PWLMap rmap; // Map of reachable vertices
  PWLMap smap; // Map of successors
};
