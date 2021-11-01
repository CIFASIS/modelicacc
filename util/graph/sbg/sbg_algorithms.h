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

// Minimum reachable -----------------------------------------------------------------------------

std::pair<PWLMap, PWLMap> recursion(int n, Set VR, Set E, PWLMap Emap, PWLMap map_D, PWLMap map_B, PWLMap currentSmap, PWLMap currentRmap);
PWLMap minReach1(Set V, PWLMap map_D, PWLMap map_B, PWLMap currentSmap, PWLMap currentRmap);
std::pair<PWLMap, PWLMap> minReachable(int nmax, Set V, Set E, PWLMap Vmap, PWLMap Emap, PWLMap map_D, PWLMap map_B, PWLMap currentSmap, PWLMap currentRmap);

// Matching of undirected SBGraphs ---------------------------------------------------------------

struct MatchingStruct{
  MatchingStruct(SBGraph g);


  Set SBGMatching();

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

  PWLMap mmap; // Offset map

  PWLMap rmap; // Map of reachable vertices
  PWLMap smap; // Map of successors

  void debugInit();
  void debugStep();

  Set filterSuccsEdges(PWLMap map_D, PWLMap map_B, PWLMap s_map);
};
