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

// Connected components --------------------------------------------------------------------------

PWLMap connectedComponents(SBGraph g);

// Matching --------------------------------------------------------------------------------------

typedef OrdCT<Set> SetPath;

//typedef VertexMap property_map<SBGraph, > // For access to set-vertices

struct MatchingStruct{
  MatchingStruct(SBGraph g);

  Set SBGMatching();

  UnordCT<Set> split(Set ftilde);

  SetPath waspf(Set ftilde, bool rec);
  SetPath waspu(Set utilde);

  SBGraph g;

  PWLMap mapF; // "Left" maps
  PWLMap mapU; // "Right" maps

  int wmax; // Maximum augmenting path width found

  UnordCT<Set> auxF; // Auxiliary set of left set-vertices

  Set matchedF;
  Set matchedU;

  UnordCT<Set> visitedU;

  UnordCT<Set> currentF; // Set of set-vertices in current path

  Set matchedE; 

  SetPath Pmax;
};
