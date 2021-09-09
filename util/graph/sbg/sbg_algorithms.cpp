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
  map_D = mapD;
  mapB = mapU;
  map_B = mapB;

  allEdges = mapF.wholeDom();
  F = mapF.image(allEdges);
  U = mapU.image(allEdges);
  allVertices = F.cup(U);  

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
  lmap = emptyMap;
  lmapD = emptyMap;
  lmapB = emptyMap;
}

void MatchingStruct::minReachable(PWLMap m_map, Set E)
{
  std::cout << "E: " << E << "\n\n";
  std::cout << "m_map: " << m_map << "\n\n";
 
  Interval i(0, 1, 0);
  MultiInterval mi;
  for (int j = 0; j < mapF.ndim(); j++)
    mi.addInter(i);
  Set zero = createSet(mi);

  Set tildeV = m_map.image(allVertices);

  map_D = map_D.restrictMap(E);
  map_B = map_B.restrictMap(E);
  PWLMap tildeD = m_map.compPW(map_D);
  PWLMap tildeB = m_map.compPW(map_B);

  PWLMap tildeVid(tildeV);
  rmap = tildeVid;
  smap = tildeVid;

  PWLMap oldsmap = smap;
  PWLMap oldrmap = rmap;

  Set Vc;

  Set m_mapImage = m_map.image(m_map.wholeDom());
  PWLMap m_mapInv = m_map.minInv(m_mapImage);

  do {
    oldsmap = smap;
    smap = tildeB.minAdjMap(tildeD, rmap);
    smap = smap.combine(tildeVid);
    rmap = smap.mapInf();
    PWLMap deltaSmap = smap.diffMap(oldsmap);
    Vc = tildeV.diff(deltaSmap.preImage(zero));
  }
  while (!Vc.empty());


  Set first = map_D.image(allEdges).cap(unmatchedV); // Unmatched vertices at start of alternating path
  Set inPath = rmap.preImage(first); // Vertices that reach unmatched vertices
  //std::cout << "first: " << first << "\n\n";
  rmap = rmap.restrictMap(inPath);
  rmap = rmap.compPW(m_map);
  rmap = m_mapInv.compPW(rmap);

  smap = smap.compPW(m_map);
  smap = m_mapInv.compPW(smap);
  smap = smap.restrictMap(rmap.wholeDom());
  std::cout << "smap: " << smap << "\n\n";
  std::cout << "rmap: " << rmap << "\n\n";
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
   
    map_D = mapD;
    map_B = mapB; 
    minReachable(mmapD, Ed);
    PWLMap rmapD = rmap;
    PWLMap smapD = smap;
    lmapD = lmap;

    Set tildeV = rmapD.preImage(F);
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
    
    map_D = mapB;
    map_B = mapD; 
    minReachable(mmapB, tildeEd);
    PWLMap rmapB = rmap;
    PWLMap smapB = smap;
    lmapB = lmap;

    tildeV = rmapB.preImage(U);
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
