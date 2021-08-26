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

Set MatchingStruct::recursion(PWLMap s_map, int iters, Set E)
{
  PWLMap tildesmap = s_map;
 
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

    tildesmap = tildesmap.compPW(s_map);
  }

  return E;
}

void MatchingStruct::minReachable(PWLMap m_map, Set E)
{
  std::cout << "E: " << E << "\n\n";

  ORD_REALS g;
  g.insert(g.begin(), 0);
  ORD_REALS o;
  o.insert(o.begin(), 0);
  PWLMap lengths;
  lengths.addSetLM(allVertices, LMap(g, o));
  lmap = lengths;

  Set tildeV = m_map.image(allVertices);
  Set tildeMatchedV = m_map.image(matchedV);

  map_D = map_D.restrictMap(E);
  map_B = map_B.restrictMap(E);
  PWLMap tildeD = m_map.compPW(map_D);
  PWLMap tildeB = m_map.compPW(map_B);

  Set unmatchedD = tildeD.image(allEdges);
  unmatchedD = unmatchedD.diff(tildeMatchedV);

  PWLMap tildeVid(tildeV);
  rmap = tildeVid;
  smap = tildeVid;

  PWLMap oldrmap = rmap;

  do {
    map_D = map_D.restrictMap(E);
    map_B = map_B.restrictMap(E);
    tildeD = m_map.compPW(map_D);
    tildeB = m_map.compPW(map_B);
  
    PWLMap dr = rmap.compPW(tildeD);
    PWLMap br = tildeB;

    PWLMap r1map = br.minAdjMap(dr);
    PWLMap tildermap = r1map.combine(rmap);

    oldrmap = rmap;
    rmap = tildermap.mapInf();
    rmap = rmap.minMap(oldrmap);
    Set aux = rmap.preImage(unmatchedD);
    rmap = rmap.restrictMap(aux);

    tildeD = m_map.compPW(map_D);
    smap = tildeB.minAdjMap(tildeD, rmap);
  }
  while (!oldrmap.equivalentPW(rmap));

  Set m_mapImage = m_map.image(m_map.wholeDom());
  PWLMap m_mapInv = m_map.minInv(m_mapImage);

  rmap = rmap.compPW(m_map);
  rmap = m_mapInv.compPW(rmap);
  Set reps = rmap.image(allVertices);

  smap = smap.compPW(m_map);
  smap = m_mapInv.compPW(smap);
  smap = smap.restrictMap(rmap.wholeDom());

  // Create lmap
  PWLMap auxsmap = smap;
  PWLMap originalsmap = auxsmap;
  PWLMap oldsmap = auxsmap;

  PWLMap auxlmap;
  ORD_INTS off;
  off.insert(off.begin(), 1);

  auxlmap = lmap.restrictMap(allVertices.diff(reps));
  auxlmap = auxlmap.offsetMap(off);
  lmap = auxlmap.combine(lmap);

  do {
    Set converged = auxsmap.preImage(reps);
    Set unconverged = auxsmap.wholeDom().diff(converged);
    auxlmap = lmap.restrictMap(unconverged);
    auxlmap = auxlmap.offsetMap(off); 
    lmap = auxlmap.combine(lmap);

    oldsmap = auxsmap;
    auxsmap = auxsmap.compPW(originalsmap);
  }
  while (!oldsmap.equivalentPW(auxsmap));

  if (rmap.wholeDom() != allVertices) {
    ORD_REALS g;
    g.insert(g.begin(), 0);
    ORD_REALS o;
    o.insert(o.begin(), 0);
    PWLMap auxmap;
    auxmap.addSetLM(allVertices.diff(rmap.wholeDom()), LMap(g, o));
    auxlmap = auxmap;
    lmap = auxlmap.combine(lmap);
  }
}

Set MatchingStruct::findPathEdges(int lD, int lB, PWLMap smapD, PWLMap smapB, Set v)
{
  Set edges;

  Set auxv = v;

  PWLMap auxsmapD = smapD;
  for (int i = 0; i < lD; i++) {
    Set adjEdges = mapB.preImage(v);
    Set succ = smapD.image(v);
    adjEdges = adjEdges.cap(mapD.preImage(succ));

      edges = edges.cup(adjEdges);

    v = succ;
  }

  PWLMap auxsmapB = smapB;
  v = auxv;
  for (int i = 0; i < lB; i++) {
    Set adjEdges = mapD.preImage(v);
    Set succ = smapB.image(v);
    adjEdges = adjEdges.cap(mapB.preImage(succ));

    edges = edges.cup(adjEdges);

    v = succ;
  }

  std::cout << "edges: " << edges << "\n\n";
  return edges;
}

Set MatchingStruct::longestPath(PWLMap rmapD, PWLMap rmapB, PWLMap smapD, PWLMap smapB)
{
  Set edges;

  Set inAlternatingPath = rmapD.wholeDom().cap(rmapB.wholeDom()); // Vertices that reach unmatched right and left vertices
  std::cout << "inAlternating: " << inAlternatingPath << "\n\n";
  Set reps = rmapD.image(inAlternatingPath); // Representants of inAlternatingPath
  reps = reps.cup(rmapB.image(inAlternatingPath));

  std::cout << "rmapD: " << rmapD << "\n";
  std::cout << "rmapB: " << rmapB << "\n\n";

  // Traverse "connected" components
  BOOST_FOREACH (AtomSet as, reps.asets()) {
    Set sas = createSet(as);
    Set repd = rmapD.preImage(sas); // Represented vertices in an alternating path
    repd = repd.cup(rmapB.preImage(sas));
    repd = repd.cap(inAlternatingPath);      
    std::cout << "reps: " << reps << "\n";
    std::cout << "repd: " << repd << "\n\n";

    PWLMap lmap = lmapD.addMap(lmapB);
    std::cout << "lmap: " << lmap << "\n";
    ORD_INTS longest = (lmap.image(repd)).maxElem(); // Length of longest alternating path with vertices in repd
    INT l = *(longest.begin());
    std::cout << "l: " << l << "\n";

    Interval iLongest(l, 1, l);
    Set sLongest = createSet(iLongest);
    Set vertexInLongest = lmap.preImage(sLongest); // Vertex in longest alternating path
    std::cout << "vertexl: " << vertexInLongest << "\n";

    ORD_INTS longestD = lmapD.image(vertexInLongest).maxElem();
    INT lD = *(longestD.begin()); 
    ORD_INTS longestB = lmapB.image(vertexInLongest).maxElem();
    INT lB = *(longestB.begin()); 

    BOOST_FOREACH (SetVertexDesc vdi, vertices(g)) {
      Set vi = g[vdi].range();

      Set setVertexLongest = vertexInLongest.cap(vi);
      if (!setVertexLongest.empty()) {
        setVertexLongest = setVertexLongest.cap(vi);
        std::cout << "vertex: " << setVertexLongest << "\n";
        std::cout << "as: " << as << "\n\n";
        Set edgesInPath = findPathEdges(lD, lB, smapD, smapB, setVertexLongest);
        edges = edges.cup(edgesInPath);
  
        break;
      }
    }
  }
  
  return edges;
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
    Set unmatchedV = allVertices.diff(matchedV);
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

    PWLMap mmapB = offsetUB.combine(mmap);
    
    map_D = mapB;
    map_B = mapD; 
    minReachable(mmapB, tildeEd);
    PWLMap rmapB = rmap;
    PWLMap smapB = smap;
    lmapB = lmap;

    tildeV = rmapB.preImage(U);
    Set auxTildeEd = mapU.preImage(tildeV);
    tildeEd = auxTildeEd.cap(tildeEd);

    edgesRmapD = rmapB.compPW(mapD);
    edgesRmapB = rmapB.compPW(mapB);
    mapd = edgesRmapB.diffMap(edgesRmapD);
    auxTildeEd = mapd.preImage(zero);
    tildeEd = auxTildeEd.cap(tildeEd); 

    // Leave only one edge departuring from each vertex
    Set longest = longestPath(rmapD, rmapB, smapD, smapB);
    tildeEd = longest;
    std::cout << "tildeEd: " << tildeEd << "\n\n";

    PWLMap mapDEd = mapD.restrictMap(tildeEd);
    PWLMap invD = mapDEd.minInv(mapD.image(tildeEd));
    Set auxtildeEd = invD.compPW(mapD).image(tildeEd);
    tildeEd = auxtildeEd.cap(tildeEd);

    PWLMap mapBEd = mapB.restrictMap(tildeEd);
    PWLMap invB = mapBEd.minInv(mapB.image(tildeEd));
    auxtildeEd = invB.compPW(mapB).image(tildeEd);
    tildeEd = auxtildeEd.cap(tildeEd);

    // Update matched vertices

    // Revert matched and unmatched edges
    mapDEd = mapD.restrictMap(tildeEd);
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
