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

  Set emptySet;
  matchedV = emptySet;
  matchedE = emptySet;
  Ed = allEdges; 

  PWLMap emptyMap;
  rmap = emptyMap;
  smap = emptyMap;

  PWLMap idMap(allVertices);
  mmap = idMap;
}

Set MatchingStruct::recursion(PWLMap smap, int iters, Set E)
{
  PWLMap tildesmap = smap;
 
  for (int i = 0; i < iters; i++) {
    std::set<SetVertexDesc> visited;

    BOOST_FOREACH (SetVertexDesc vdi, vertices(g)) {
      Set vi = g[vdi].range();
      Set pres = tildesmap.preImage(vi);
      Set vis = vi.cap(pres);

      if (!vis.empty()) {
        visited.insert(vdi);
        std::cout << vi << "\n";
      }
    }

    if (!visited.empty()) { 
      Set edgesRec;
      BOOST_FOREACH (SetEdgeDesc edi, edges(g)) {
        SetVertexDesc v1 = source(edi, g);
        SetVertexDesc v2 = target(edi, g);

        std::set<SetVertexDesc>::iterator it1 = visited.find(v1);
        std::set<SetVertexDesc>::iterator it2 = visited.find(v2);

        Set dom;
        BOOST_FOREACH (Set domi, g[edi].map_f().dom())
          dom = dom.cup(domi);

        if (it1 != visited.end() && it2 != visited.end())
          edgesRec = edgesRec.cup(dom); 
      }
      Set edgesRecM = edgesRec.cap(matchedE);

      E = E.diff(edgesRecM);
      PWLMap mapDE = mapD.restrictMap(E);
      PWLMap mapBE = mapB.restrictMap(E);
      mapD = mapBE.combine(mapD);
      mapB = mapDE.combine(mapB);

      Set vertsRecMD = mapD.image(edgesRecM);
      Set vertsRecMB = mapB.image(edgesRecM);
      Set vertsRecM = vertsRecMD.cup(vertsRecMB);
      PWLMap idRecM(vertsRecM);
      rmap = idRecM.combine(rmap);
    }

    tildesmap = tildesmap.compPW(smap);
  }

  return E;
}

void MatchingStruct::minReachable(PWLMap m_map, PWLMap map_D, PWLMap map_B, Set E)
{
  Set unmatchedE = allEdges.diff(matchedE);

  Set tildeV = m_map.image(allVertices);

  map_D = map_D.restrictMap(E);
  map_B = map_B.restrictMap(E);
  PWLMap tildeD = m_map.compPW(map_D);
  PWLMap tildeB = m_map.compPW(map_B);

  Set unmatchedD = tildeD.image(unmatchedE);

  PWLMap tildeVid(tildeV);
  rmap = tildeVid;
  smap = tildeVid;

  PWLMap oldrmap = rmap;

  int iters = 1;

  Set m_mapDom = m_map.wholeDom();
  Set m_mapImage = m_map.image(m_mapDom);
  PWLMap m_mapInv = m_map.minInv(m_mapImage);

  Set dom = rmap.wholeDom();

  do {
    map_D = map_D.restrictMap(E);
    map_B = map_B.restrictMap(E);
    tildeD = m_map.compPW(map_D);
    tildeB = m_map.compPW(map_B);
  
    PWLMap dr = rmap.compPW(tildeD);
    PWLMap br = rmap.compPW(tildeB);

    PWLMap r1map = br.minAdjMap(dr);
    Set aux = r1map.preImage(unmatchedD);
    r1map = r1map.restrictMap(aux);
    PWLMap tildermap = r1map.combine(rmap);
    tildermap = tildermap.minMap(rmap);

    oldrmap = rmap;
    rmap = tildermap.mapInf();
    if (!oldrmap.equivalentPW(rmap)) {
      smap = tildeB.minAdjMap(tildeD, rmap);
      smap = smap.combine(rmap);
      smap = smap.minMap(rmap);
      smap = smap.mapInf();
      //E = recursion(smap, iters, E);
    }

    iters++;
  }
  while (!oldrmap.equivalentPW(rmap));

  rmap = rmap.compPW(m_map);
  rmap = m_mapInv.compPW(rmap);
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

  Set unknowns = mapU.image(allVertices);

  std::cout << "\n";
  do {
    Ed = allEdges;
    Set unmatchedV = allVertices.diff(matchedV);
    OrdCT<INT> maxV = allVertices.maxElem();

    // Forward direction
    Set unmatchedRight = mapU.image(Ed);
    unmatchedRight = unmatchedRight.cap(unmatchedV);
    PWLMap offsetUD = mmap.restrictMap(unmatchedRight);
    offsetUD = offsetUD.offsetMap(maxV);

    Set matchedRight = mapU.image(Ed);
    matchedRight = matchedRight.cap(matchedV);
    PWLMap offsetMD = mmap.restrictMap(matchedRight);
    offsetMD = offsetMD.offsetMap(maxV);

    PWLMap mmapD = offsetUD.combine(mmap);
    mmapD = offsetMD.combine(mmapD);
    
    minReachable(mmapD, mapD, mapB, Ed);
    PWLMap rmapD = rmap;

    Set tildeV = rmapD.preImage(F);
    tildeEd = mapU.preImage(tildeV);
    tildeEd = tildeEd.cap(Ed);

    PWLMap edgesRmapD = rmapD.compPW(mapD);
    PWLMap edgesRmapB = rmapD.compPW(mapB);
    PWLMap mapd = edgesRmapD.diffMap(edgesRmapB);
    Set auxEd = mapd.preImage(zero);
    tildeEd = auxEd.cap(tildeEd); 

    // Backward direction
    Set unmatchedLeft = mapF.image(tildeEd);
    unmatchedLeft = unmatchedLeft.cap(unmatchedV);
    PWLMap offsetUB = mmap.restrictMap(unmatchedLeft);
    offsetUB = offsetUB.offsetMap(maxV);

    Set matchedLeft = mapF.image(Ed);
    matchedLeft = matchedLeft.cap(matchedV);
    PWLMap offsetMB = mmap.restrictMap(matchedLeft);
    offsetMB = offsetMB.offsetMap(maxV);

    PWLMap mmapB = offsetUB.combine(mmap);
    mmapB = offsetMB.combine(mmapB);
    
    minReachable(mmapB, mapB, mapD, tildeEd);
    PWLMap rmapB = rmap;

    tildeV = rmapB.preImage(U);
    Set auxTildeEd = mapU.preImage(tildeV);
    tildeEd = auxTildeEd.cap(tildeEd);

    edgesRmapD = rmapB.compPW(mapD);
    edgesRmapB = rmapB.compPW(mapB);
    mapd = edgesRmapB.diffMap(edgesRmapD);
    auxTildeEd = mapd.preImage(zero);
    tildeEd = auxTildeEd.cap(tildeEd); 

    // Leave only one edge departuring from each vertex
    PWLMap invB = mapB.minInv(mapB.image(mapB.wholeDom()));
    tildeEd = invB.compPW(mapB).image(tildeEd);

    // Update matched vertices
    Set matchedD = (rmapD.compPW(mapD)).image(tildeEd);
    Set matchedB = (rmapB.compPW(mapB)).image(tildeEd);
    matchedV = matchedV.cup(matchedD);
    matchedV = matchedV.cup(matchedB);
    diffMatched = unknowns.diff(matchedV);

    // Update mmap
    PWLMap auxM = mmap.restrictMap(matchedV);
    auxM = auxM.offsetMap(maxV);
    auxM = auxM.offsetMap(maxV);
    mmap = auxM.combine(mmap);

    // Revert matched and unmatched edges
    PWLMap mapDEd = mapD.restrictMap(tildeEd);
    PWLMap mapBEd = mapB.restrictMap(tildeEd);
    mapD = mapBEd.combine(mapD);
    mapB = mapDEd.combine(mapB);

    matchedE = mapD.preImage(U);
    bool den = diffMatched.empty();
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
