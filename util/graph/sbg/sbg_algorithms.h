/*****************************************************************************
 
    This file is part of the Set-Based Graph library.

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
#include <util/graph/sbg/defs.h>
#include <util/graph/sbg/interval.h>
#include <util/graph/sbg/multi_interval.h>
#include <util/graph/sbg/atomic_set.h>
#include <util/graph/sbg/set.h>
#include <util/graph/sbg/lmap.h>
#include <util/graph/sbg/atom_pw_map.h>
#include <util/graph/sbg/pw_map.h>
#include <util/graph/sbg/sbg.h>
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

  UnordCT<Set> split(Set ftilde);

  SetPath waspf(Set ftilde);
  SetPath waspu(Set utilde);

  Set wholeVertex(Set matched_subset);

  Set wholeEdge(Set matched_subset);

  Set matchedUVar(Set var);

  bool checkRecursion(Set candidate_u);

  bool matchingLookAhead(Set matched_f, Set matched_u);

  VertexIt findSetVertex(Set matched);

  int pathWidth();

  SetPath pathTo(Set var);

  void updatePath(SetPath path);

  SBGraph g;

  PWLMap mapF; // "Left" maps
  PWLMap mapU; // "Right" maps

  int wmax; // Maximum augmenting path width found

  UnordCT<Set> auxF; // Auxiliary set of left not matched set-vertices

  Set matchedF;
  Set matchedU;

  UnordCT<Set> visitedU;

  UnordCT<Set> currentF; // Set of set-vertices in current path

  Set matchedE; 

  SetPath Pmax;
};
