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
  std::cout << "\nEmap: " << Emap << "\n\n";

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

PWLMap MatchingStruct::recursion(int n, Set VR, PWLMap map_D, PWLMap map_B)
{
  Interval i(0, 1, 0);
  MultiInterval mi;
  for (int j = 0; j < mapF.ndim(); j++)
    mi.addInter(i);
  Set zero = createSet(mi);

  Set ES = smap.compPW(map_B).diffMap(map_D).preImage(zero);

  Set ER;
  PWLMap smapNth = smap;
  PWLMap originalSmap = smap;
  for (int i = 0; i < n; i++) {
    smapNth = smapNth.compPW(originalSmap); 
    ER = ER.cup(map_D.preImage(smapNth.image(VR)));
  }
  ER = ER.cap(ES);

  Set ERplus = Emap.preImage(Emap.image(ER));
  
  PWLMap tildeD = map_D.restrictMap(ERplus);
  PWLMap tildeB = map_B.restrictMap(ERplus);
  PWLMap smapPlus = tildeD.compPW(tildeB.minInv(tildeB.image()));

  PWLMap finals = smap.restrictMap(smapNth.image(VR));
  smapPlus = finals.combine(smapPlus);

  PWLMap rmapPlus = rmap.compPW(smapPlus.mapInf());
  PWLMap tildeRmap = rmap.minMap(rmapPlus);
  rmap = tildeRmap.combine(rmap);

  return rmap;
}

PWLMap MatchingStruct::minReachable(Set V, Set E, PWLMap map_D, PWLMap map_B)
{
  int nmax = num_vertices(g);
 
  Interval i(0, 1, 0);
  MultiInterval mi;
  for (int j = 0; j < mapF.ndim(); j++)
    mi.addInter(i);
  Set zero = createSet(mi);

  map_D = map_D.restrictMap(E);
  map_B = map_B.restrictMap(E);

  PWLMap Vid(V);
  rmap = Vid;
  smap = Vid;

  PWLMap oldsmap = smap;
  PWLMap oldrmap = rmap;

  Set Vc;

  do {
    oldsmap = smap;
    smap = map_B.minAdjMap(map_D, rmap);
    smap = smap.combine(Vid);
    rmap = smap.mapInf();
    PWLMap deltaSmap = smap.diffMap(oldsmap);
    Vc = V.diff(deltaSmap.preImage(zero));

    if (!Vc.empty()) {
      int n = 0;
      Set VR;
      do {
        n++;
        PWLMap deltaVmap = Vmap.compPW(smap.compPW(n)).diffMap(Vmap); 
        VR = deltaVmap.preImage(zero).cap(Vc);
      }
      while(VR.empty() && n < nmax);

      if (!VR.empty()) 
        rmap = recursion(n, VR, map_D, map_B);
    }
  }
  while (!Vc.empty());

  return rmap;
}

Set MatchingStruct::SBGMatching()
{
  debug();

  Set tildeEd;
  Set diffMatched;

  Interval i(0, 1, 0);
  MultiInterval mi;
  for (int j = 0; j < mapF.ndim(); j++)
    mi.addInter(i);
  Set zero = createSet(mi);

  std::cout << "\n";
  do {
    Ed = allEdges;
    unmatchedV = allVertices.diff(matchedV);
    OrdCT<INT> maxV = allVertices.maxElem();

    // Forward direction
    Set unmatchedRight = mapU.image(Ed);
    unmatchedRight = unmatchedRight.cap(unmatchedV);
    PWLMap offsetUD = mmap.restrictMap(unmatchedRight);
    offsetUD = offsetUD.offsetMap(maxV);

    PWLMap mmapD = offsetUD.combine(mmap);
    PWLMap mmapInv = mmap.minInv(mmapD.image());
   
    PWLMap map_D = mmapD.compPW(mapD);
    PWLMap map_B = mmapD.compPW(mapB); 
    PWLMap rmapD = minReachable(mmapD.image(allVertices), Ed, map_D, map_B);
    rmapD = mmapInv.compPW(rmapD.compPW(mmapD));

    Set tildeV = rmapD.preImage(F.cap(unmatchedV));
    tildeEd = mapU.preImage(tildeV).cup(mapF.preImage(tildeV));
    tildeEd = tildeEd.cap(Ed);

    PWLMap edgesRmapD = rmapD.compPW(mapD);
    PWLMap edgesRmapB = rmapD.compPW(mapB);
    PWLMap mapd = edgesRmapD.diffMap(edgesRmapB);
    Set auxEd = mapd.preImage(zero);
    tildeEd = auxEd.cap(tildeEd); 

    PWLMap mapBEd = mapB.restrictMap(tildeEd);
    PWLMap invB = mapBEd.minInv(mapB.image(tildeEd));
    PWLMap teo = invB.compPW(mapBEd);
    Set auxtildeEd = invB.compPW(mapBEd).image(tildeEd);
    tildeEd = auxtildeEd.cap(tildeEd);

    // Backward direction
    Set unmatchedLeft = mapF.image(tildeEd);
    unmatchedLeft = unmatchedLeft.cap(unmatchedV);
    PWLMap offsetUB = mmap.restrictMap(unmatchedLeft);
    offsetUB = offsetUB.offsetMap(maxV);

    PWLMap mmapB = offsetUB.combine(mmap);
    mmapInv = mmap.minInv(mmapB.image());
    
    map_D = mmapB.compPW(mapB);
    map_B = mmapB.compPW(mapD); 
    PWLMap rmapB = minReachable(mmapB.image(allVertices), tildeEd, map_D, map_B);
    rmapB = mmapInv.compPW(rmapB.compPW(mmapB));

    tildeV = rmapB.preImage(U.cap(unmatchedV));
    Set auxTildeEd = mapU.preImage(tildeV).cup(mapF.preImage(tildeV));
    tildeEd = auxTildeEd.cap(tildeEd);

    edgesRmapD = rmapB.compPW(mapD);
    edgesRmapB = rmapB.compPW(mapB);
    mapd = edgesRmapB.diffMap(edgesRmapD);
    auxTildeEd = mapd.preImage(zero);
    tildeEd = auxTildeEd.cap(tildeEd); 

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
    auxM = auxM.offsetMap(maxV);
    auxM = auxM.offsetMap(maxV);
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
