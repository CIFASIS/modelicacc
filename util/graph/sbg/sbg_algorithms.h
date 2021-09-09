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

  Set recursion(PWLMap s_map, int iters, Set E);
  Set findPathEdges(int lD, int lB, PWLMap smapD, PWLMap smapB, Set v);
  Set longestPath(PWLMap rmapD, PWLMap rmapB, PWLMap smapD, PWLMap smapB);
  void minReachable(PWLMap m_map, Set E);
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
  PWLMap map_D; // Auxiliary map
  PWLMap mapB; // Backward direction
  PWLMap map_B; // Auxiliary map

  Set matchedV;
  Set unmatchedV;
  Set matchedE; 
  Set Ed;

  PWLMap mmap;

  PWLMap rmap; // Map of reachable vertices
  PWLMap smap; // Map of successors
  PWLMap lmap; // Map of length of paths
  PWLMap lmapD; // Auxiliary map
  PWLMap lmapB; // Auxiliary map
};
