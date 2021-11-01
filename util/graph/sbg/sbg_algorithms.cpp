/*****************************************************************************
 
    This file is part of the Set-Based Graph library.

******************************************************************************/

#include <util/graph/sbg/sbg_algorithms.h>

/*-----------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
// SB-Graphs algorithms
/*-----------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/

// Connected components ---------------------------------------------------------------------------

PWLMap connectedComponents(SBGraph g)
{
  PWLMap res;

  VertexIt vi_start, vi_end;
  boost::tie(vi_start, vi_end) = vertices(g);
  EdgeIt ei_start, ei_end;
  boost::tie(ei_start, ei_end) = edges(g);

  if (vi_start != vi_end /*&& ei_start != ei_end*/) {
    Set vss;
    while (vi_start != vi_end) {
      Set aux = (g[*vi_start]).range();
      vss = vss.cup(aux);

      ++vi_start;
    }

    PWLMap auxres(vss);
    res = auxres;

    if (ei_start == ei_end) return res;

    PWLMap emap1 = (g[*ei_start]).map_f();
    PWLMap emap2 = (g[*ei_start]).map_u();
    ++ei_start;

    while (ei_start != ei_end) {
      emap1 = (g[*ei_start]).map_f_ref().combine(emap1);
      emap2 = (g[*ei_start]).map_u_ref().combine(emap2);

      ++ei_start;
    }

    PWLMap oldres;

    while (!oldres.equivalentPW(res)) {
      PWLMap ermap1 = res.compPW(emap1);
      PWLMap ermap2 = res.compPW(emap2);

      PWLMap rmap1 = ermap1.minAdjMap(ermap2);
      PWLMap rmap2 = ermap2.minAdjMap(ermap1);
      rmap1 = rmap1.combine(res);
      rmap2 = rmap2.combine(res);

      PWLMap newRes = rmap1.minMap(rmap2);
      oldres = res;
      res = newRes.minMap(res);

      if (!oldres.equivalentPW(res)) {
        res = newRes;
        res = res.mapInf();
      }
    }
  }

  return res;
}

// Minimum reachable ------------------------------------------------------------------------------

std::pair<PWLMap, PWLMap> recursion(int n, Set VR, Set E, PWLMap Emap, PWLMap map_D, PWLMap map_B, PWLMap currentSmap, PWLMap currentRmap)
{
  // *** Initialization

  PWLMap newSmap;
  PWLMap newRmap; // Minimum reachable map

  Interval i(0, 1, 0);
  MultiInterval mi;
  for (int j = 0; j < map_D.ndim(); j++)
    mi.addInter(i);
  Set zero = createSet(mi);

  // *** Traverse graph

  Set ES = currentSmap.compPW(map_B).diffMap(map_D).preImage(zero); // Edges that connect vertices with successors

  Set currentVertex = VR;
  PWLMap smapNth = currentSmap; // Map of nth successor
  Set ER; // Edges in recursive paths
  PWLMap originalSmap = currentSmap;
  for (int i = 0; i < n; i++) {
    ER = ER.cup(map_D.preImage(smapNth.image(VR)).cap(map_B.preImage(currentVertex)));
    currentVertex = smapNth.image(VR);
    smapNth = smapNth.compPW(originalSmap); 
  }
  ER = ER.cap(ES);

  Set ERplus = Emap.preImage(Emap.image(ER)); // Edges that share sub-set-edges with those in ER
  ERplus = ERplus.cap(E);
  
  PWLMap tildeD = map_D.restrictMap(ERplus);
  PWLMap tildeB = map_B.restrictMap(ERplus);
  PWLMap smapPlus = tildeD.compPW(tildeB.minInv(tildeB.image())); // Successor map through ERplus

  PWLMap finalsRec = smapNth.restrictMap(smapNth.image(VR));
  smapPlus = finalsRec.combine(smapPlus); // Preserve the successors of the final vertices
  smapPlus = smapPlus.combine(currentSmap);

  PWLMap rmapPlus = currentRmap.compPW(smapPlus.mapInf()); // Minimum reachable through ERplus
  PWLMap tildeRmap = currentRmap.minMap(rmapPlus); // New minimum reachable map
  newRmap = tildeRmap.combine(currentRmap);

  newSmap = smapPlus.minMap(currentSmap, newRmap);

  return std::pair<PWLMap, PWLMap>(newSmap, newRmap);
}

bool notEqId(Set dom, LMap lm) 
{
  ORD_REALS::iterator itg = lm.gain_ref().begin();
  ORD_REALS::iterator ito = lm.offset_ref().begin();

  for (int i = 0; i < lm.ndim(); i++) 
    if (*itg != 1 || *ito != 0)
      return true;

  return false;
}

// Find min reachable in 1 step
PWLMap minReach1(Set V, PWLMap map_D, PWLMap map_B, PWLMap currentSmap, PWLMap currentRmap)
{
  PWLMap finalSmap;

  Interval i(0, 1, 0);
  MultiInterval mi;
  for (int j = 0; j < map_D.ndim(); j++)
    mi.addInter(i);
  Set zero = createSet(mi);

  PWLMap Vid(V);

  PWLMap auxSmap = map_B.minAdjMap(map_D, currentRmap); // Get minimum adjacent vertex with minimum representative
  auxSmap = auxSmap.combine(Vid);
  PWLMap newSmap = currentSmap.minMap(auxSmap, currentRmap); // Old successor vs new successor  

  PWLMap oldSuccRep = currentRmap.compPW(currentSmap);
  PWLMap newSuccRep = currentRmap.compPW(newSmap);

  PWLMap deltaSmap = newSuccRep.diffMap(oldSuccRep); // Difference of representatives of old and new succesor (always non positive)
  Set updateVerts = V.diff(deltaSmap.preImage(zero)); // Vertices "v" for which rmap(smap_new(v)) < rmap(smap_old(v)) 

  finalSmap = newSmap.restrictMap(updateVerts).combine(currentSmap);

  return finalSmap;
}

// Minimum reachable in several steps
std::pair<PWLMap, PWLMap> minReachable(int nmax, Set V, Set E, PWLMap Vmap, PWLMap Emap, PWLMap map_D, PWLMap map_B, PWLMap currentSmap, PWLMap currentRmap)
{
  // *** Initialization

  PWLMap newSmap; // Successors map
  PWLMap newRmap; // Minimum reachable map

  Interval i(0, 1, 0);
  MultiInterval mi;
  for (int j = 0; j < map_D.ndim(); j++)
    mi.addInter(i);
  Set zero = createSet(mi);

  map_D = map_D.restrictMap(E);
  map_B = map_B.restrictMap(E);

  PWLMap Vid(V);
  newRmap = Vid;
  newSmap = Vid;

  PWLMap oldsmap = currentSmap;
  PWLMap oldrmap = currentRmap;

  Set Vc; // Vertices that changed its successor

  // *** Traverse graph
  do {
    oldsmap = newSmap;
    newSmap = minReach1(V, map_D, map_B, newSmap, newRmap); // Get successor
    newRmap = newSmap.mapInf(); // Get representative

    PWLMap deltaSmap = newSmap.diffMap(oldsmap); 
    Vc = V.diff(deltaSmap.preImage(zero)); // Vertices that changed its successor

    if (!Vc.empty()) {
      int n = 0;
      Set VR; // Recursive vertices that changed its successor 
      PWLMap smapNth = newSmap;
      do {
        PWLMap deltaVmap = Vmap.compPW(smapNth).diffMap(Vmap);
        VR = deltaVmap.preImage(zero).cap(Vc);
        n++;
        smapNth = smapNth.compPW(newSmap);
      }
      while(VR.empty() && n < nmax);

      // *** Handle recursion
      if (!VR.empty()) { 
        std::pair<PWLMap, PWLMap> p = recursion(n, VR, E, Emap, map_D, map_B, newSmap, newRmap);
        newRmap = std::get<1>(p);
      }
    }
  }
  while (!Vc.empty());
 

  return std::pair<PWLMap, PWLMap> (newSmap, newRmap);
}

// Matching ---------------------------------------------------------------------------------------

MatchingStruct::MatchingStruct(SBGraph garg)
{
  g = garg; 

  BOOST_FOREACH (SetEdgeDesc ei, edges(g)) {
    PWLMap fmap = (g[ei]).map_f();
    PWLMap umap = (g[ei]).map_u();

    mapF = mapF.concat(fmap);
    mapU = mapU.concat(umap);
  }

  mapD = mapF;
  mapB = mapU;

  allEdges = mapF.wholeDom();
  F = mapF.image(allEdges);
  U = mapU.image(allEdges);
  allVertices = F.cup(U);  
  int dims = F.ndim();

  PWLMap vmap;
  BOOST_FOREACH (SetVertexDesc vdi, vertices(g)) {
    SetVertex vi = g[vdi];

    LMap lm;
    for (int i = 0; i < dims; i++) 
      lm.addGO(0, vi.id());
    vmap.addSetLM(vi.range(), lm);
  }
  Vmap = vmap;

  int eCount = 1;
  PWLMap emap;
  BOOST_FOREACH (SetEdgeDesc edi, edges(g)) {
    SetEdge ei = g[edi];

    BOOST_FOREACH (Set sf, ei.map_f_ref().dom()) {
      BOOST_FOREACH (Set su, ei.map_u_ref().dom()) {
        Set uf = sf.cap(su);
 
        if (!uf.empty()) {
          LMap lm;
          for (int i = 0; i < dims; i++) 
            lm.addGO(0, eCount);
          emap.addSetLM(uf, lm);
  
          ++eCount;
        }
      }
    }
  }
  Emap = emap;

  Set emptySet;
  matchedV = emptySet;
  unmatchedV = emptySet;
  matchedE = emptySet;

  PWLMap idMap(allVertices);
  mmap = idMap;

  PWLMap emptyMap;
  rmap = emptyMap;
  smap = emptyMap;
}

// Get edges used to get to successors
Set MatchingStruct::filterSuccsEdges(PWLMap map_D, PWLMap map_B, PWLMap s_map)
{
  Set resEdges;

  PWLMap auxSmap = s_map.filterMap(notEqId);

  Set edgesB = map_B.preImage(auxSmap.wholeDom());
  Set edgesD = map_D.preImage(auxSmap.image());
  Set edgesBD = edgesB.cap(edgesD);

  BOOST_FOREACH (AtomSet as, edgesBD.asets()) 
    if (auxSmap.image(map_B.image(as)) == map_D.image(as))
      resEdges.addAtomSet(as);

  return resEdges;
}

Set MatchingStruct::SBGMatching()
{
  debugInit();

  int nmax = num_vertices(g);

  Set tildeEd;
  Set diffMatched;

  Interval i(0, 1, 0);
  MultiInterval mi;
  for (int j = 0; j < mapF.ndim(); j++)
    mi.addInter(i);
  Set zero = createSet(mi);

  PWLMap idMap(allVertices);

  PWLMap oldVmap = Vmap;

  do {
    tildeEd = allEdges;
    unmatchedV = allVertices.diff(matchedV);
    OrdCT<INT> maxV = allVertices.maxElem();

    // *** Forward direction

    // Offset vertices that are the image of edges in the backward direction
    Set unmatchedRight = mapU.image(tildeEd);
    unmatchedRight = unmatchedRight.cap(unmatchedV);
    PWLMap offsetUD = mmap.restrictMap(unmatchedRight);
    offsetUD = offsetUD.offsetImageMap(maxV);

    PWLMap mmapD = offsetUD.combine(mmap);
    PWLMap mmapInv = mmap.minInv(mmapD.image());
  
    Vmap = oldVmap.offsetDomMap(mmapD);
    PWLMap map_D = mmapD.compPW(mapD);
    PWLMap map_B = mmapD.compPW(mapB);

    // Get minimum reachable
    std::pair<PWLMap, PWLMap> p = minReachable(nmax, mmapD.image(allVertices), tildeEd, Vmap, Emap, map_D, map_B, smap, rmap);
    PWLMap rmapD = std::get<1>(p);
    rmapD = mmapInv.compPW(rmapD.compPW(mmapD));
    rmapD = rmapD.combine(idMap);

    // Get edges that reach unmatched vertices in forward direction
    Set tildeV = rmapD.preImage(F.cap(unmatchedV));
    Set auxTildeEd = mapU.preImage(tildeV).cup(mapF.preImage(tildeV));
    Set edgesInPaths = filterSuccsEdges(map_D, map_B, std::get<0>(p));
    tildeEd = edgesInPaths.cap(auxTildeEd);

    // *** Backward direction

    // Offset vertices that are the image of edges in the forward direction
    Set unmatchedLeft = mapF.image(tildeEd);
    unmatchedLeft = unmatchedLeft.cap(unmatchedV);
    PWLMap offsetUB = mmap.restrictMap(unmatchedLeft);
    offsetUB = offsetUB.offsetImageMap(maxV);

    PWLMap mmapB = offsetUB.combine(mmap);
    mmapInv = mmap.minInv(mmapB.image());
    
    Vmap = oldVmap.offsetDomMap(mmapB);
    map_D = mmapB.compPW(mapB);
    map_B = mmapB.compPW(mapD); 

    // Get minimum reachable
    p = minReachable(nmax, mmapB.image(allVertices), tildeEd, Vmap, Emap, map_D, map_B, smap, rmap);
    PWLMap rmapB = std::get<1>(p);
    rmapB = mmapInv.compPW(rmapB.compPW(mmapB));
    rmapB = rmapB.combine(idMap);

    // Get edges that reach unmatched vertices in backward direction
    tildeV = rmapB.preImage(U.cap(unmatchedV));
    auxTildeEd = mapU.preImage(tildeV).cup(mapF.preImage(tildeV));
    edgesInPaths = filterSuccsEdges(map_D, map_B, std::get<0>(p));
    tildeEd = auxTildeEd.cap(edgesInPaths).cap(tildeEd);

    // *** Revert matched and unmatched edges in augmenting paths
    PWLMap mapDEd = mapD.restrictMap(tildeEd);
    PWLMap mapBEd = mapB.restrictMap(tildeEd);
    mapD = mapBEd.combine(mapD);
    map_D = map_D;
    mapB = mapDEd.combine(mapB);
    map_B = map_B;

    // *** Update matched edges and vertices
    matchedE = mapD.preImage(U);
    matchedV = mapD.image(matchedE);
    matchedV = matchedV.cup(mapB.image(matchedE));
    diffMatched = U.diff(matchedV);

    // *** Update mmap for matched vertices
    PWLMap auxM = mmap.restrictMap(matchedV);
    auxM = auxM.offsetImageMap(maxV);
    auxM = auxM.offsetImageMap(maxV);
    mmap = auxM.combine(mmap);

    unmatchedV = allVertices.diff(matchedV);
    PWLMap idUnmatched(unmatchedV);
    mmap = idUnmatched.combine(mmap);

    debugStep();
  }
  while (!diffMatched.empty() && !tildeEd.empty());

  matchedE = mapD.preImage(U);
  return matchedE;
}

void MatchingStruct::debugInit()
{
  std::cout << "\n\n";
  BOOST_FOREACH (SetVertexDesc vi, vertices(g)) {
    SetVertex v = g[vi];
    Set vs = v.range();

    Set e1 = mapF.preImage(vs);
    Set e2 = mapU.preImage(vs);

    std::cout << "-------\n";
    std::cout << v << " | " << e1 << " | " << e2 << "\n";
  }
  std::cout << "-------\n\n";

  std::cout << "Vmap: " << Vmap << "\n\n";
  std::cout << "Emap: " << Emap << "\n\n";

  std::cout << "*******************************\n\n";
}

void MatchingStruct::debugStep()
{
  bool diffMatched = (U.diff(matchedV)).empty();
  std::cout << "matchedE: " << matchedE << "\n";
  std::cout << "matched all unknwons: " << diffMatched << "\n\n";

  BOOST_FOREACH (AtomSet as1, matchedE.asets()) {
    BOOST_FOREACH (AtomSet as2, matchedE.asets()) {
      if (as1 != as2) {
        Set s1 = createSet(as1);
        Set s2 = createSet(as2);

        Set inter1 = mapF.image(s1).cap(mapF.image(s2));
        Set inter2 = mapU.image(s1).cap(mapU.image(s2));
 
        if (!inter1.empty() || !inter2.empty())
          std::cout << "adj: " << as1 << " | " << as2 << "\n\n";
      }
    }
  }
}
