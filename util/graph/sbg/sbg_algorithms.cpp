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
        res = res.mapInf(1);
      }
    }
  }

  return res;
}

// Minimum reachable ------------------------------------------------------------------------------

// Emap, allVertices
PWLMap recursion(int n, Set VR, Set E, PWLMap Emap, PWLMap map_D, PWLMap map_B, PWLMap currentSmap, PWLMap currentRmap)
{
  PWLMap newRmap;

  Interval i(0, 1, 0);
  MultiInterval mi;
  for (int j = 0; j < map_D.ndim(); j++)
    mi.addInter(i);
  Set zero = createSet(mi);

  Set ES = currentSmap.compPW(map_B).diffMap(map_D).preImage(zero);

  PWLMap smapNth = currentSmap;
  Set ER;
  PWLMap originalSmap = currentSmap;
  for (int i = 0; i < n - 1; i++) {
    ER = ER.cup(map_B.preImage(smapNth.image(VR)));
    smapNth = smapNth.compPW(originalSmap); 
  }
  ER = ER.cap(ES);

  Set ERplus = Emap.preImage(Emap.image(ER));
  ERplus = ERplus.cap(E);
  
  PWLMap tildeD = map_D.restrictMap(ERplus);
  PWLMap tildeB = map_B.restrictMap(ERplus);
  PWLMap smapPlus = tildeD.compPW(tildeB.minInv(tildeB.image()));

  PWLMap finalsRec = currentSmap.restrictMap(smapNth.image(VR));
  smapPlus = finalsRec.combine(smapPlus);
  smapPlus = smapPlus.combine(currentSmap);

  std::cout << "rec1\n";
  PWLMap rmapPlus = currentRmap.compPW(smapPlus.mapInf(1));
  std::cout << "rec2\n\n";
  PWLMap tildeRmap = currentRmap.minMap(rmapPlus);
  newRmap = tildeRmap.combine(currentRmap);

  return newRmap;
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

std::pair<PWLMap, Set> minReachable(int nmax, Set V, Set E, PWLMap Vmap, PWLMap Emap, PWLMap map_D, PWLMap map_B, PWLMap currentSmap, PWLMap currentRmap)
{
  PWLMap newSmap;
  PWLMap newRmap;

  std::cout << "E: " << E << "\n\n";
 
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

  Set Vc;

  do {
    oldsmap = newSmap;
    newSmap = map_B.minAdjMap(map_D, newRmap);
    newSmap = newSmap.combine(Vid);
    newRmap = newSmap.mapInf(1);
    PWLMap deltaSmap = newSmap.diffMap(oldsmap);
    Vc = V.diff(deltaSmap.preImage(zero));

    if (!Vc.empty()) {
      int n = 0;
      Set VR;
      do {
        n++;
        PWLMap deltaVmap = Vmap.compPW(newSmap).diffMap(Vmap);
        VR = deltaVmap.preImage(zero).cap(Vc);
      }
      while(VR.empty() && n < nmax);

      if (!VR.empty()) 
        newRmap = recursion(n, VR, E, Emap, map_D, map_B, newSmap, newRmap);
    }
  }
  while (!Vc.empty());

  PWLMap auxSmap = newSmap.filterMap(notEqId);

  Set edgesB = map_B.preImage(auxSmap.wholeDom());
  Set edgesD = map_D.preImage(auxSmap.image());
  Set edgesBD = edgesB.cap(edgesD);
  Set edges;

  BOOST_FOREACH (AtomSet as, edgesBD.asets()) 
    if (auxSmap.image(map_B.image(as)) == map_D.image(as))
      edges.addAtomSet(as);

  return std::pair<PWLMap, Set>(newRmap, edges);
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
  Ed = allEdges; 

  PWLMap idMap(allVertices);
  mmap = idMap;

  PWLMap emptyMap;
  rmap = emptyMap;
  smap = emptyMap;
}

Set MatchingStruct::SBGMatching()
{
  debug();

  int nmax = num_vertices(g);

  Set tildeEd;
  Set diffMatched;

  Interval i(0, 1, 0);
  MultiInterval mi;
  for (int j = 0; j < mapF.ndim(); j++)
    mi.addInter(i);
  Set zero = createSet(mi);

  PWLMap oldVmap = Vmap;

  std::cout << "\n";
  do {
    Ed = allEdges;
    unmatchedV = allVertices.diff(matchedV);
    OrdCT<INT> maxV = allVertices.maxElem();

    // Forward direction
    Set unmatchedRight = mapU.image(Ed);
    unmatchedRight = unmatchedRight.cap(unmatchedV);
    PWLMap offsetUD = mmap.restrictMap(unmatchedRight);
    offsetUD = offsetUD.offsetImageMap(maxV);

    PWLMap mmapD = offsetUD.combine(mmap);
    PWLMap mmapInv = mmap.minInv(mmapD.image());
  
    Vmap = oldVmap.offsetDomMap(mmapD);
    PWLMap map_D = mmapD.compPW(mapD);
    PWLMap map_B = mmapD.compPW(mapB);
    std::cout << "mmapD: " << mmapD << "\n\n";
    std::pair<PWLMap, Set> p = minReachable(nmax, mmapD.image(allVertices), Ed, Vmap, Emap, map_D, map_B, smap, rmap);
    PWLMap rmapD = std::get<0>(p);
    rmapD = mmapInv.compPW(rmapD.compPW(mmapD));

    Set tildeV = rmapD.preImage(F.cap(unmatchedV));
    tildeEd = mapU.preImage(tildeV).cup(mapF.preImage(tildeV));
    tildeEd = tildeEd.cap(Ed);
    std::cout << "tildeEd: " << tildeEd << "\n\n";

    PWLMap edgesRmapD = rmapD.compPW(mapD);
    PWLMap edgesRmapB = rmapD.compPW(mapB);
    PWLMap mapd = edgesRmapD.diffMap(edgesRmapB);
    Set auxEd = mapd.preImage(zero);
    tildeEd = auxEd.cap(tildeEd); 
    std::cout << "tildeEd: " << tildeEd << "\n\n";

    // Only one outing edge
    //tildeEd = tildeEd.cap(std::get<1>(p));
    /*
    PWLMap mapBEd = mapB.restrictMap(tildeEd);
    PWLMap invB = mapBEd.minInv(mapB.image(tildeEd));
    Set auxtildeEd = invB.compPW(mapBEd).image(tildeEd);
    tildeEd = auxtildeEd.cap(tildeEd);
    PWLMap mapBEd = mapB.restrictMap(tildeEd);
    PWLMap invB = mapBEd.restrictInv(mapB.image(tildeEd), std::get<1>(p));
    std::cout << "invB: " << invB << "\n\n";
    Set auxtildeEd = invB.compPW(mapBEd).image(tildeEd);
    tildeEd = auxtildeEd.cap(tildeEd);
    std::cout << "tildeEd: " << tildeEd << "\n\n";
    */
    PWLMap mapBEd = mapB.restrictMap(tildeEd);
    PWLMap invB = mapB.restrictMap(std::get<1>(p)).minInv(mapB.image(tildeEd));
    Set auxtildeEd = invB.compPW(mapBEd).image(tildeEd);
    tildeEd = auxtildeEd.cap(tildeEd);
    std::cout << "tildeEd: " << tildeEd << "\n\n";

    // Backward direction
    Set unmatchedLeft = mapF.image(tildeEd);
    unmatchedLeft = unmatchedLeft.cap(unmatchedV);
    PWLMap offsetUB = mmap.restrictMap(unmatchedLeft);
    offsetUB = offsetUB.offsetImageMap(maxV);

    PWLMap mmapB = offsetUB.combine(mmap);
    mmapInv = mmap.minInv(mmapB.image());
    
    Vmap = oldVmap.offsetDomMap(mmapB);
    map_D = mmapB.compPW(mapB);
    map_B = mmapB.compPW(mapD); 
    std::cout << "mmapB: " << mmapB << "\n\n";
    p = minReachable(nmax, mmapB.image(allVertices), tildeEd, Vmap, Emap, map_D, map_B, smap, rmap);
    PWLMap rmapB = std::get<0>(p);
    rmapB = mmapInv.compPW(rmapB.compPW(mmapB));

    tildeV = rmapB.preImage(U.cap(unmatchedV));
    Set auxTildeEd = mapU.preImage(tildeV).cup(mapF.preImage(tildeV));
    tildeEd = auxTildeEd.cap(tildeEd);
    std::cout << "tildeEd: " << tildeEd << "\n\n";

    edgesRmapD = rmapB.compPW(mapD);
    edgesRmapB = rmapB.compPW(mapB);
    mapd = edgesRmapB.diffMap(edgesRmapD);
    auxTildeEd = mapd.preImage(zero);
    tildeEd = auxTildeEd.cap(tildeEd); 
    std::cout << "tildeEd: " << tildeEd << "\n\n";

    // Only one outing edge
    /*
    PWLMap mapDEd = mapD.restrictMap(tildeEd);
    PWLMap invD = mapDEd.minInv(mapD.image(tildeEd));
    auxtildeEd = invD.compPW(mapDEd).image(tildeEd);
    tildeEd = auxtildeEd.cap(tildeEd);
    std::cout << "tildeEd: " << tildeEd << "\n\n";
    */
    //tildeEd = tildeEd.cap(std::get<1>(p));
    //std::cout << "tildeEd: " << tildeEd << "\n\n";

    // Update matched vertices

    // Revert matched and unmatched edges
    PWLMap mapDEd = mapD.restrictMap(tildeEd);
    mapBEd = mapB.restrictMap(tildeEd);
    mapD = mapBEd.combine(mapD);
    map_D = map_D;
    mapB = mapDEd.combine(mapB);
    map_B = map_B;

    matchedE = mapD.preImage(U);
    matchedV = mapD.image(matchedE);
    matchedV = matchedV.cup(mapB.image(matchedE));
    diffMatched = U.diff(matchedV);

    // Update mmap
    PWLMap auxM = mmap.restrictMap(matchedV);
    auxM = auxM.offsetImageMap(maxV);
    auxM = auxM.offsetImageMap(maxV);
    mmap = auxM.combine(mmap);

    unmatchedV = allVertices.diff(matchedV);
    PWLMap idUnmatched(unmatchedV);
    mmap = idUnmatched.combine(mmap);

    bool den = diffMatched.empty();
    std::cout << "matchedE: " << matchedE << "\n";
    std::cout << "matchedV: " << matchedV << "\n";
    std::cout << "den: " << den << "\n\n";
  }
  while (!diffMatched.empty() && !tildeEd.empty());

  matchedE = mapD.preImage(U);
  return matchedE;
}

void MatchingStruct::debug()
{
  BOOST_FOREACH (SetVertexDesc vi, vertices(g)) {
    SetVertex v = g[vi];
    Set vs = v.range();

    Set e1 = mapF.preImage(vs);
    Set e2 = mapU.preImage(vs);

    std::cout << "-------\n";
    std::cout << vs << " | " << e1 << " | " << e2 << "\n";
  }
  std::cout << "\n";
}
