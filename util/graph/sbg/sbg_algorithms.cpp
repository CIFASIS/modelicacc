/*****************************************************************************
 
    This file is part of the Set-Based Graph library.

******************************************************************************/

#include <boost/graph/subgraph.hpp>
#include <util/graph/sbg/sbg_algorithms.h>

#include <chrono>
#include <time.h>

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

bool eqId(Set dom, LMap lm) 
{
  ORD_REALS::iterator itg = lm.gain_ref().begin();
  ORD_REALS::iterator ito = lm.offset_ref().begin();

  for (int i = 0; i < lm.ndim(); i++) 
    if (*itg != 1 || *ito != 0)
      return false;

  return true;
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

// Use result of newMap if offMap(newMap(v)) < offMap(currentMap(v))
// (In the minReachable context, is used to select a new successor
//  only if the new successor has a "<" representative than the old
//  successor)
PWLMap updateMap(Set V, PWLMap currentMap, PWLMap newMap, PWLMap offMap)
{
  Interval i(0, 1, 0);
  MultiInterval mi;
  for (int j = 0; j < currentMap.ndim(); j++)
    mi.addInter(i);
  Set zero = createSet(mi);

  PWLMap offCurrent = offMap.compPW(currentMap);
  PWLMap offNew = offMap.compPW(newMap);

  PWLMap deltaMap = offCurrent.diffMap(offNew);
  Set notUpdateVerts = deltaMap.preImage(zero);
  Set updateVerts = V.diff(notUpdateVerts);
 
  PWLMap resMap = newMap.restrictMap(updateVerts);
  resMap = resMap.combine(currentMap.restrictMap(notUpdateVerts));

  return resMap;
}

std::pair<PWLMap, PWLMap> recursion(int n, Set ER, Set V, Set E, PWLMap Emap, PWLMap map_D, PWLMap map_B, PWLMap currentSmap, PWLMap currentSEmap, PWLMap currentRmap)
{
  // *** Initialization

  std::cout << "ER: " << ER << " | " << n << "\n\n";

  PWLMap Vid(V);

  PWLMap newSmap;
  PWLMap newRmap; // Minimum reachable map

  Interval i(0, 1, 0);
  MultiInterval mi;
  for (int j = 0; j < map_D.ndim(); j++)
    mi.addInter(i);
  Set zero = createSet(mi);

  // *** Traverse graph

  PWLMap semapNth = currentSEmap;
  Set Erec = ER;
  PWLMap originalSmap = currentSmap;
  PWLMap originalSEmap = currentSEmap;
  auto start_loop = std::chrono::system_clock::now();
  for (int i = 0; i < n; i++) {
    Erec = Erec.cup(semapNth.image(ER));
    semapNth = semapNth.compPW(originalSEmap);
  }
  auto end_loop = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds_loop = end_loop - start_loop;
  std::cout << "time loop rec: " << elapsed_seconds_loop.count() << "\n\n";

  Set ERplus = Emap.preImage(Emap.image(Erec)); // Edges that share sub-set-edges with those in Erec
  ERplus = ERplus.cap(E);
  
  PWLMap tildeD = map_D.restrictMap(ERplus);
  PWLMap tildeB = map_B.restrictMap(ERplus);
  PWLMap smapPlus = tildeD.compPW(tildeB.minInv(tildeB.image())); // Successor map through ERplus

  PWLMap finalsRec = currentSmap.restrictMap(map_D.image(semapNth.image(ER)));
  smapPlus = finalsRec.combine(smapPlus); // Preserve the successors of the final vertices
  newSmap = smapPlus;

  auto start_inf = std::chrono::system_clock::now();
  PWLMap rmapPlus = currentRmap.compPW(newSmap.mapInf(n)); // Minimum reachable through ERplus
  newSmap = newSmap.combine(currentSmap);
  auto end_inf = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds_inf = end_inf - start_inf;
  std::cout << "time inf rec: " << elapsed_seconds_inf.count() << "\n\n";
  newRmap = currentRmap.minMap(rmapPlus); // New minimum reachable map
  newRmap = newRmap.combine(currentRmap);

  std::cout << "smap rec: " << newSmap << "\n\n";
  std::cout << "rmap rec: " << newRmap << "\n\n";

  return std::pair<PWLMap, PWLMap>(newSmap, newRmap);
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

  PWLMap idCurrentSmap = currentSmap.filterMap(eqId);
  Set idVerts = idCurrentSmap.diffMap(currentRmap).preImage(zero);
  Set usableVerts = idVerts.cup(currentSmap.filterMap(notEqId).wholeDom());

  PWLMap adjSmap = map_B.minAdjMap(map_D, currentRmap); // Get minimum adjacent vertex with minimum representative
  PWLMap newSmap = adjSmap.minMap(currentSmap.restrictMap(usableVerts), currentRmap); // Old successor vs new successor  
  newSmap = updateMap(V, currentSmap, newSmap, currentRmap);
  newSmap = newSmap.combine(adjSmap);
  newSmap = newSmap.combine(currentSmap);

  return newSmap;
}

// Minimum reachable in several steps
std::pair<PWLMap, PWLMap> minReachable(int nmax, Set V, Set E, PWLMap Vmap, PWLMap Emap, PWLMap map_D, PWLMap map_B)
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

  PWLMap oldSmap;
  PWLMap oldSEmap;

  PWLMap Vid(V);
  newRmap = Vid;
  newSmap = Vid;

  PWLMap Eid(E);
  PWLMap newSEmap = Eid;

  Set Vc; // Vertices that changed its successor
  Set Ec; // Edges that changed its successor

  // *** Traverse graph
  do {
    oldSmap = newSmap;
    newSmap = minReach1(V, map_D, map_B, newSmap, newRmap); // Get successor
    auto start_inf = std::chrono::system_clock::now();
    newRmap = newSmap.mapInf(1); // Get representative
    auto end_inf = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds_inf = end_inf - start_inf;
    std::cout << "time inf minreach: " << elapsed_seconds_inf.count() << "\n\n";

    PWLMap deltaSmap = newSmap.diffMap(oldSmap); 
    Vc = V.diff(deltaSmap.preImage(zero)); // Vertices that changed its successor
  
    if (!Vc.empty()) {
      oldSEmap = newSEmap;
      Set Esucc = newSmap.compPW(map_B).diffMap(map_D).preImage(zero); // Edges that connect vertices with successors
      newSEmap = (map_B.restrictMap(Esucc).minInv(newSmap.image())).compPW(map_D.restrictMap(Esucc));
      newSEmap = newSEmap.combine(oldSEmap);

      int n = 1;
      Set ER; // Recursive edges that changed its successor
      PWLMap se2map = newSEmap.compPW(newSEmap);
      PWLMap semapNth = se2map;
      auto start_loop = std::chrono::system_clock::now();
      do {
        PWLMap deltaEmap = Emap.compPW(semapNth.filterMap(notEqId)).diffMap(Emap);
        ER = deltaEmap.preImage(zero);
        semapNth = semapNth.compPW(newSEmap);

        n++;
      }
      while(ER.empty() && n < nmax);
      auto end_loop = std::chrono::system_clock::now();
      std::chrono::duration<double> elapsed_seconds_loop = end_loop - start_loop;
      std::cout << "time loop minreach: " << elapsed_seconds_loop.count() << "\n\n";

      // *** Handle recursion
      if (!ER.empty()) { 
        ER = ER.normalize();
        //ER = createSet(*(ER.asets().begin()));
        std::pair<PWLMap, PWLMap> p = recursion(n, ER, V, E, Emap, map_D, map_B, newSmap, newSEmap, newRmap);
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
  Ed = allEdges;

  Set emptySet;
  allVertices = emptySet;
  F = mapF.image(allEdges);
  U = mapU.image(allEdges);
  nmax = num_vertices(g);
  int dims = F.ndim();

  PWLMap vmap;
  BOOST_FOREACH (SetVertexDesc vdi, vertices(g)) {
    SetVertex vi = g[vdi];
    allVertices.addAtomSets(vi.range_ref().asets());

    LMap lm;
    for (int i = 0; i < dims; i++) 
      lm.addGO(0, vi.id());
    vmap.addSetLM(vi.range(), lm);
  }
  Vmap = vmap;
  maxV = allVertices.maxElem();

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

  matchedV = emptySet;
  unmatchedV = emptySet;
  matchedE = emptySet;

  PWLMap idMap(allVertices);
  mmap = idMap;

  PWLMap emptyMap;
  rmap = emptyMap;
  smap = emptyMap;
}

void MatchingStruct::directedMinReach(PWLMap sideMap)
{
  // Offset unmatched vertices on the opposite side of the search
  Set unmatchedSide = sideMap.image(Ed);
  unmatchedSide = unmatchedSide.cap(unmatchedV);
  PWLMap offsetUS = mmap.restrictMap(unmatchedSide);
  offsetUS = offsetUS.offsetImageMap(maxV);

  PWLMap mmapSide = offsetUS.combine(mmap);
  Set VSide = mmapSide.image(allVertices);
  PWLMap mmapSideInv = mmapSide.minInv(mmapSide.image());

  // Offset image of Vmap, mapD and mapB according to mmapSide 
  PWLMap VmapSide = Vmap.offsetDomMap(mmapSide);
  PWLMap mapDSide = mmapSide.compPW(mapD);
  PWLMap mapBSide = mmapSide.compPW(mapB);

  std::cout << "mmap: " << mmapSide << "\n\n";

  // Get minimum reachable
  auto start = std::chrono::system_clock::now();
  std::pair<PWLMap, PWLMap> p = minReachable(nmax, VSide, Ed, VmapSide, Emap, mapDSide, mapBSide);
  auto end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;
  std::cout << "time minreach: " << elapsed_seconds.count() << "\n\n";
  PWLMap directedSmap = std::get<0>(p);
  smap = mmapSideInv.compPW(directedSmap.compPW(mmapSide));
  PWLMap directedRmap = std::get<1>(p);
  rmap = mmapSideInv.compPW(directedRmap.compPW(mmapSide));

  std::cout << "smap: " << smap << "\n\n";
  std::cout << "rmap: " << rmap << "\n\n";
}

//Set MatchingStruct::SBGComponentMatching()
Set MatchingStruct::SBGMatching()
{
  debugInit();

  Set diffMatched;

  Interval i(0, 1, 0);
  MultiInterval mi;
  for (int j = 0; j < mapF.ndim(); j++)
    mi.addInter(i);
  Set zero = createSet(mi);

  PWLMap idMap(allVertices);

  PWLMap oldVmap = Vmap;

  do {
    Ed = allEdges;
    unmatchedV = allVertices.diff(matchedV);

    // *** Forward direction

    directedMinReach(mapU);

    // Get edges that reach unmatched vertices in forward direction
    Set tildeV = rmap.preImage(F.cap(unmatchedV));
    Set tildeEd = mapU.preImage(tildeV).cup(mapF.preImage(tildeV));

    // Leave edges in paths that reach unmatched left vertices
    Set edgesInPaths = smap.compPW(mapB).diffMap(mapD).preImage(zero); // Edges that connect vertices with successors
    Ed = edgesInPaths.cap(tildeEd);

    // *** Backward direction

    PWLMap auxMapD = mapD;
    mapD = mapB;
    mapB = auxMapD;
    directedMinReach(mapF);

    // Get edges that reach unmatched vertices in backward direction
    tildeV = rmap.preImage(U.cap(unmatchedV));
    tildeEd = mapU.preImage(tildeV).cup(mapF.preImage(tildeV));

    // Leave edges in paths that reach unmatched left and right vertices
    edgesInPaths = smap.compPW(mapB).diffMap(mapD).preImage(zero); // Edges that connect vertices with successors
    Ed = Ed.cap(edgesInPaths).cap(tildeEd);

    mapB = mapD;
    mapD = auxMapD;

    // *** Revert matched and unmatched edges in augmenting paths
    PWLMap mapDEd = mapD.restrictMap(Ed);
    PWLMap mapBEd = mapB.restrictMap(Ed);
    mapD = mapBEd.combine(mapD);
    mapB = mapDEd.combine(mapB);

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
  while (!diffMatched.empty() && !Ed.empty());

  matchedE = mapD.preImage(U);
  return matchedE;
}

/*
Set MatchingStruct::SBGMatching()
{
  PWLMap cc = connectedComponents(g);
  Set reps = cc.image();

  if (reps.asets_ref().size() == 1)
    return SBGComponentMatching();

  else {
    Set matched; 

    BOOST_FOREACH (AtomSet as, reps.asets()) {
      SBGraph gComponent;
      std::set<SetVertexDesc> visitedNodes;

      Set repds = cc.preImage(createSet(as));

      BOOST_FOREACH (SetVertexDesc vi, vertices(g)) {
        if (visitedNodes.find(vi) == visitedNodes.end()) {
          Set vs = g[vi].range();
          if (!vs.cap(repds).empty()) {
            SetVertexDesc viComponent = boost::add_vertex(gComponent);
            gComponent[viComponent] = g[vi];
            visitedNodes.insert(vi);

            BOOST_FOREACH (SetEdgeDesc ej, out_edges(vi, g)) {
              SetVertexDesc vj = source(ej, g);
              if (source(ej, g) == vi)
                vj = target(ej, g);

              SetVertexDesc vjComponent = vj;
 
              if (visitedNodes.find(vjComponent) == visitedNodes.end()) {
                SetVertexDesc vjComp = boost::add_vertex(gComponent);
                gComponent[vjComp] = g[vj];
                visitedNodes.insert(vj);
                vjComponent = vjComp;
              }

              else {
                BOOST_FOREACH (SetVertexDesc vComponent, vertices(gComponent)) {
                  if (!gComponent[vComponent].range().cap(g[vj].range()).empty())
                    vjComponent = vComponent;
                }
              }

              SetEdgeDesc eiComponent;
              bool b;
              boost::tie(eiComponent, b) = boost::add_edge(viComponent, vjComponent, gComponent);
              gComponent[eiComponent] = g[ej];
            }
          }
        }
      }

      MatchingStruct matchingComponent(gComponent);
      matched = matched.cup(matchingComponent.SBGComponentMatching());
    }

    return matched;
  }

  return Set();
}
*/

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
  std::cout << "matched all unknowns: " << diffMatched << "\n\n";

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
