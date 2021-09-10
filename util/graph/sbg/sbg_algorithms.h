/*****************************************************************************
 
    This file is part of the Set-Based Graph library.

******************************************************************************/

#pragma once

#include <iostream>

#include <boost/foreach.hpp>
#include <boost/unordered_set.hpp>

#include <util/graph/sbg/sbg.h>

using namespace SBG;

// Connected components of undirected SBGraphs ---------------------------------------------------

PWLMap connectedComponents(SBGraph g);

// Matching of undirected SBGraphs ---------------------------------------------------------------

struct MatchingStruct{
  MatchingStruct(SBGraph g);

  void debug();

  Set SBGMatching();

  PWLMap recursion(int n, Set VR, PWLMap map_D, PWLMap map_B);
  PWLMap minReachable(Set V, Set E, PWLMap map_D, PWLMap map_B);

  private:
  SBGraph g;
  Set F;
  Set U;
  PWLMap Vmap; // Map from vertices to set-vertices
  PWLMap Emap; // Map from edges to set-edges

  Set allEdges;
  Set allVertices;

  PWLMap mapF; // "Left" maps
  PWLMap mapU; // "Right" maps
  PWLMap mapD; // Forward direction
  PWLMap mapB; // Backward direction

  Set matchedV;
  Set unmatchedV;
  Set matchedE; 
  Set Ed;

  PWLMap mmap;

  PWLMap rmap; // Map of reachable vertices
  PWLMap smap; // Map of successors
};
