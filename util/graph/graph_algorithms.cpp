/*****************************************************************************

    This file is part of Modelica C Compiler.

    Modelica C Compiler is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Modelica C Compiler is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Modelica C Compiler.  If not, see <http://www.gnu.org/licenses/>.

******************************************************************************/

#include <iostream>
#include <list>
#include <map>
#include <math.h>
#include <utility>

#include <boost/config.hpp>
#include <boost/foreach.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/unordered_set.hpp>

#include <util/debug.h>
#include <util/graph/graph_algorithms.h>
#include <util/table.h>

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

  if(vi_start != vi_end){
    Set vss;
    while (vi_start != vi_end) {
      Set aux = (g[*vi_start]).vs_();
      vss = vss.cup(aux);

      ++vi_start;
    }

    PWLMap auxres(vss);
    res = auxres;

    if (ei_start == ei_end) return res;

    PWLMap emap1 = (g[*ei_start]).es1_();
    PWLMap emap2 = (g[*ei_start]).es2_();
    ++ei_start;

    while (ei_start != ei_end) {
      emap1 = (g[*ei_start]).es1_().combine(emap1);
      emap2 = (g[*ei_start]).es2_().combine(emap2);

      ++ei_start;
    }

    Set lastIm;
    Set newIm = vss;
    Set diffIm = vss;

    while (!diffIm.empty()) {
      PWLMap ermap1 = res.compPW(emap1);
      PWLMap ermap2 = res.compPW(emap2);

      PWLMap rmap1 = minAdjMap(ermap1, ermap2);
      PWLMap rmap2 = minAdjMap(ermap2, ermap1);
      rmap1 = rmap1.combine(res);
      rmap2 = rmap2.combine(res);

      PWLMap newRes = minMap(rmap1, rmap2);

      lastIm = newIm;
      newIm = newRes.image(vss);
      diffIm = lastIm.diff(newIm);

      if (!diffIm.empty()) {
        res = newRes;
        res = mapInf(res, std::log2);
        newIm = res.image(vss);
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
    PWLMap fmap = (g[ei]).es1_();
    PWLMap umap = (g[ei]).es2_();

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

NI2 linear(NI2 arg) 
{
  return arg;
}

void MatchingStruct::minReachable(Set E, PWLMap m_map, PWLMap map_D, PWLMap map_B)
{
  cout << "E: " << E << "\n";
  map_D = map_D.restrictMap(E);
  map_B = map_B.restrictMap(E);

  Set tildeV = m_map.image(allVertices);

  PWLMap tildeD = m_map.compPW(map_D);
  PWLMap tildeB = m_map.compPW(map_B);

  PWLMap tildeVid(tildeV);
  rmap = tildeVid;
  smap = tildeVid;

  PWLMap oldrmap = rmap;

  Set m_mapDom = m_map.wholeDom();
  Set m_mapImage = m_map.image(m_mapDom);
  PWLMap m_mapInv = minInv(m_map, m_mapImage);

  Set diffIm;
  Set dom = rmap.wholeDom();

  do {
    PWLMap dr = rmap.compPW(tildeD);
    PWLMap br = rmap.compPW(tildeB);

    PWLMap r1map = minAdjMap(dr, br);
    PWLMap tildermap = r1map.combine(rmap);
    tildermap = minMap(tildermap, rmap);

    oldrmap = rmap;
    rmap = mapInf(tildermap, linear);
    //smap = minAdjMap(map_D, map_B, rmap);
    cout << "iter: " << rmap << "\n";
   
    Set oldIm = oldrmap.image(dom);
    Set newIm = rmap.image(dom);
    diffIm = oldIm.diff(newIm);
  }
  while (!diffIm.empty());

  rmap = rmap.compPW(m_map);
  rmap = m_mapInv.compPW(rmap);
  cout << "rmap: " << rmap << "\n";
  //smap = smap.compPW(mmap);
  //smap = mmapInv.compPW(smap);
}

Set MatchingStruct::SBGMatching()
{
  BOOST_FOREACH (SetVertexDesc vi, vertices(g)) {
    SetVertex v = g[vi];
    Set vs = v.vs_();

    Set e1 = mapF.preImage(vs);
    Set e2 = mapU.preImage(vs);

    cout << "-------\n";
    cout << vs << " | " << e1 << " | " << e2 << "\n";
  }
  cout << "\n";

  Set tildeEd;
  Set diffMatched;

  Interval i(0, 1, 0);
  MultiInterval mi;
  for (int j = 0; j < mapF.ndim_(); j++)
    mi.addInter(i);
  AtomSet as(mi);
  Set zero;
  zero.addAtomSet(as);

  Interval i2(2, 1, 500);
  MultiInterval mi2;
  mi2.addInter(i2);
  AtomSet as2(mi2);
  Set s2;
  s2.addAtomSet(as2);

  Interval i3(501, 1, 999);
  MultiInterval mi3;
  mi3.addInter(i3);
  AtomSet as3(mi3);
  Set s3;
  s3.addAtomSet(as3);

  LMap lm1;
  lm1.addGO(1, 499);
  LMap lm2;
  lm2.addGO(1, 0);

  PWLMap aux1;
  aux1.addSetLM(s2, lm1);
  mapD = aux1.combine(mapD);
  PWLMap aux2;
  aux2.addSetLM(s2, lm2);
  mapB = aux2.combine(mapB);

  matchedE = s2;
  matchedV = mapF.image(s2);
  Set auxmatchedV = mapU.image(s2);
  matchedV = matchedV.cup(auxmatchedV);

  OrdCT<NI1> maxAux;
  maxAux.insert(maxAux.begin(), 2000);
  PWLMap auxmmap = mmap.restrictMap(matchedV);
  auxmmap = offsetMap(maxAux, auxmmap);
  mmap = auxmmap.combine(mmap);

  cout << "\n";
  do {
    Ed = allEdges;
    Set unmatchedV = allVertices.diff(matchedV);
    OrdCT<NI1> maxV = allVertices.maxElem();

    // Forward direction
    Set unmatchedRight = mapB.image(Ed);
    unmatchedRight = unmatchedRight.cap(unmatchedV);
    PWLMap auxD = mmap.restrictMap(unmatchedRight);
    auxD = offsetMap(maxV, auxD);
    PWLMap mmapD = auxD.combine(mmap);
    
    minReachable(Ed, mmapD, mapB, mapD);
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
    Set unmatchedLeft = mapD.image(tildeEd);
    unmatchedLeft = unmatchedLeft.cap(unmatchedV);
    PWLMap auxB = mmap.restrictMap(unmatchedLeft);
    auxB = offsetMap(maxV, auxB);
    PWLMap mmapB = auxB.combine(mmap);
    
    minReachable(tildeEd, mmapB, mapD, mapB);
    PWLMap rmapB = rmap;

    tildeV = rmapB.preImage(U);
    Set auxTildeEd = mapU.preImage(tildeV);
    tildeEd = auxTildeEd.cap(tildeEd);

    edgesRmapD = rmapB.compPW(mapD);
    edgesRmapB = rmapB.compPW(mapB);
    mapd = edgesRmapB.diffMap(edgesRmapD);
    auxTildeEd = mapd.preImage(zero);
    tildeEd = auxTildeEd.cap(tildeEd); 

    // Update matched vertices
    Set matchedD = (rmapD.compPW(mapD)).image(tildeEd);
    Set matchedB = (rmapB.compPW(mapB)).image(tildeEd);
    matchedV = matchedV.cup(matchedD);
    matchedV = matchedV.cup(matchedB);
    diffMatched = allVertices.diff(matchedV);

    // Update mmap
    PWLMap auxM = mmap.restrictMap(matchedV);
    auxM = offsetMap(maxV, auxM);
    auxM = offsetMap(maxV, auxM);
    mmap = auxM.combine(mmap);

    // Revert matched and unmatched edges
    PWLMap mapDEd = mapD.restrictMap(tildeEd);
    PWLMap mapBEd = mapB.restrictMap(tildeEd);
    mapD = mapBEd.combine(mapD);
    mapB = mapDEd.combine(mapB);

    matchedE = mapD.preImage(U);
  }
  while (!diffMatched.empty() && !tildeEd.empty());

  matchedE = mapD.preImage(U);
  return matchedE;
}
