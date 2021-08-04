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


typedef OrdCT<Set> SetPath;

//typedef VertexMap property_map<SBGraph, > // For access to set-vertices

struct MatchingStruct{
  MatchingStruct(SBGraph g);

  void debug();

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
