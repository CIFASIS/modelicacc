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

PWLMap connectedComponents(SBGraph g){
  PWLMap res;

  VertexIt vi_start, vi_end;
  boost::tie(vi_start, vi_end) = vertices(g);
  EdgeIt ei_start, ei_end;
  boost::tie(ei_start, ei_end) = edges(g);

  if(vi_start != vi_end){
    Set vss;
    while(vi_start != vi_end){
      Set aux = (g[*vi_start]).vs_();
      vss = vss.cup(aux);

      ++vi_start;
    }

    PWLMap auxres(vss); 
    res = auxres;

    if(ei_start == ei_end)
      return res;

    PWLMap emap1 = (g[*ei_start]).es1_();
    PWLMap emap2 = (g[*ei_start]).es2_();
    ++ei_start;

    while(ei_start != ei_end){
      emap1 = (g[*ei_start]).es1_().combine(emap1); 
      emap2 = (g[*ei_start]).es2_().combine(emap2); 

      ++ei_start;
    }

    Set lastIm;
    Set newIm = vss;
    Set diffIm = vss;

    while(!diffIm.empty()){
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

      if(!diffIm.empty()){
        res = newRes;
        res = mapInf(res);
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

  Set allVertices;
  NI1 n = 0;

  BOOST_FOREACH (SetVertexDesc vi, vertices(g)) {
    SetVertex v = g[vi];
    Set vs = v.vs_();

    allVertices = allVertices.cup(vs);
    n += vs.size();
  }

  EdgeIt ei_start, ei_end;
  boost::tie(ei_start, ei_end) = edges(g);

  for (; ei_start != ei_end; ++ei_start) {
    PWLMap fmap = (g[*ei_start]).es1_();
    PWLMap umap = (g[*ei_start]).es2_();

    mapF = mapF.concat(fmap);
    mapU = mapU.concat(umap);
  }

  /* TODO: variables se crean global o por uso?
  Set Udom = mapU.wholeDom();
  Set UIm = mapU.image(Udom);
  Set Fdom = mapF.wholeDom();
  Set FIm = mapF.image(Udom);
  Set noEdges = allVertices.diff(FIm).diff(UIm);
  */

  Set Udom = mapU.wholeDom();
  Set UIm = mapU.image(Udom);

  BOOST_FOREACH (SetVertexDesc vi, vertices(g)) {
    SetVertex v = g[vi];
    Set vs = v.vs_();

    if (!UIm.cap(vs).empty()) { 
      SetVertex newV(v.name, vs.offset(n));
      g[vi] = newV;
    }
  }

  mapU = mapU.offsetImage(n);

  allEdges = mapF.wholeDom();

  Set emptySet;
  matchedE = emptySet;
  unmatchedE = allEdges;

  PWLMap emptyMap;
  rmap = emptyMap;
}

UnordCT<Set> MatchingStruct::split(Set ftilde)
{
  UnordCT<Set> res;

  // Find the set-vertex where ftilde is included
  SetVertexDesc vertexFTilde;
  BOOST_FOREACH (SetVertexDesc vi, vertices(g)) {
    vertexFTilde = vi;
    SetVertex v = g[vi];
    Set vs = v.vs_();

    if(!(vs.cap(ftilde)).empty())
      break;
  }

  // Find set-edges connected to the set-vertex
  Set adjEdges = mapF.preImage(ftilde);
  BOOST_FOREACH (SetEdgeDesc ei, out_edges(vertexFTilde, g)) {
    SetEdge e = g[ei];
    PWLMap es2 = e.es2_();
    OrdCT<Set> dome2 = es2.dom_();

    BOOST_FOREACH(Set d2, dome2){
      Set etildei = d2.cap(adjEdges);
      if(!etildei.empty())
        res.insert(etildei);
    }
  }

  return res;
}

Set MatchingStruct::widest()
{
  Set res;

  Set availableEdges = allEdges.cap(unmatchedE);
  Set FIm = mapF.image(allEdges);
  Set unmatchedF = mapF.image(availableEdges);

  PWLMap auxrmap = rmap.atomize();

  BOOST_FOREACH (SetVertexDesc vi, vertices(g)) {
    SetVertex v = g[vi];
    Set vs = v.vs_();

    Set wide;
    int wmax = 0;

    // Traverse equations
    if (!(vs.cap(FIm)).empty()) {
      if (!vs.empty()) {
        BOOST_FOREACH (Set part, split(vs)) {
          part = part.cap(availableEdges);
          int sz = part.size();

          if (sz > wmax) {
            wide = part;
            wmax = sz;
          }
        }

        Set UIm = mapU.image(wide);

        BOOST_FOREACH (Set dom, auxrmap.dom_()) {
          Set domDiff = dom.diff(UIm);

          if (!dom.empty() && domDiff.empty()) {
            Set fpart = mapU.preImage(dom);
            fpart = fpart.cap(wide);
            res = res.cup(fpart);

            // Quit matching unknown from other equations
            Set preU = mapU.preImage(UIm);
            availableEdges = availableEdges.diff(preU);
          }
        } 
      }
    }
  }

  return res;
}

void MatchingStruct::connectAlternating()
{
  BOOST_FOREACH (AtomSet as, matchedE.asets_()) {
    Set sas;
    sas.addAtomSet(as);  

    Set matchedFas = mapF.image(sas);
    Set connections = mapF.preImage(matchedFas);
    connections = connections.cap(unmatchedE);
    Set connectUIm = mapU.image(connections);

    Set adjAlternate = mapU.image(connections);
    adjAlternate = adjAlternate.cap(matchedU);
    adjAlternate = adjAlternate.diff(connectUIm);

    Set preAdjAlternate = mapU.preImage(adjAlternate);
    connections = connections.cap(preAdjAlternate);

    BOOST_FOREACH (AtomSet connect, connections.asets_()) {
      Set connection;
      connection.addAtomSet(connect);

      PWLMap Frep = mapF.restrictMap(connection);
      PWLMap connectFrep = rmap.compPW(Frep);
      PWLMap Urep = mapU.restrictMap(connection);
      PWLMap connectUrep = rmap.compPW(Urep);

      PWLMap pw = minAdjMap(connectFrep, connectUrep);
      PWLMap auxrmap = pw.combine(rmap);
      rmap = minMap(rmap, auxrmap);
    }
  }
}

void MatchingStruct::swapMatched(Set unmatchedPart) 
{
  Set availableEdges = unmatchedPart.cup(matchedE);

  PWLMap auxMapF = mapF.restrictMap(availableEdges);
  PWLMap auxMapU = mapU.restrictMap(availableEdges);
  PWLMap edgesFrep = rmap.compPW(mapF);
  PWLMap edgesUrep = rmap.compPW(mapU);

  UnordCT<Set> edgesInPaths = edgesFrep.sameImage(edgesUrep);

  BOOST_FOREACH (Set path, edgesInPaths) {
    path = path.cap(availableEdges);
    Set matchedEInPath = path.cap(matchedE);
    Set unmatchedEInPath = path.cap(unmatchedPart);
    NI1 sz1 = matchedEInPath.asets_().size();
    NI1 sz2 = unmatchedEInPath.asets_().size();

    if (sz2 > sz1) {
      matchedE = matchedE.diff(matchedEInPath);
      matchedE = matchedE.cup(unmatchedEInPath);

      matchedF = mapF.image(matchedE);
      matchedU = mapU.image(matchedE);
    }

    else if (sz1 == 1 && sz2 == 1) {
      matchedE = matchedE.cup(unmatchedEInPath);

      matchedF = mapF.image(matchedE);
      matchedU = mapU.image(matchedE);
    }
  }
}

void MatchingStruct::minReachable()
{
  VertexIt vi_start, vi_end;
  boost::tie(vi_start, vi_end) = vertices(g);
  EdgeIt ei_start, ei_end;
  boost::tie(ei_start, ei_end) = edges(g);

  if(vi_start != vi_end){
    Set F = mapF.image(allEdges);
    Set U = mapU.image(allEdges);
    Set allVertices = F.cup(U);

    PWLMap auxmap1(allVertices);
    rmap = auxmap1;

    Set lastMatched, lastIm;
    Set newIm = allVertices;
    Set diffMatched, diffIm;

    Set diffU;

    do {
      Set unmatchedPart = widest();

      PWLMap unmatchedMapU = mapU.restrictMap(unmatchedPart);
      PWLMap matchedMapU = mapU.restrictMap(matchedE);
      PWLMap unmatchedMapF = mapF.restrictMap(unmatchedPart);
      PWLMap matchedMapF = mapF.restrictMap(matchedE);

      PWLMap URn = rmap.compPW(unmatchedMapU);
      PWLMap URm = rmap.compPW(matchedMapU);
      PWLMap FRn = rmap.compPW(unmatchedMapF);
      PWLMap FRm = rmap.compPW(matchedMapF);

      PWLMap rmap1 = minAdjMap(URn, FRn);
      PWLMap rmap2 = minAdjMap(FRm, URm);

      rmap1 = rmap1.combine(rmap);
      rmap2 = rmap2.combine(rmap);

      PWLMap newRmap = minMap(rmap1, rmap2);

      lastIm = newIm;
      newIm = newRmap.image(allVertices);
      diffIm = lastIm.diff(newIm);      

      if (!diffIm.empty()) {
        rmap = newRmap; 
        rmap = mapInf(rmap);
        newIm = rmap.image(allVertices);

        connectAlternating();

        rmap = mapInf(rmap);
     
        lastMatched = matchedE;
        swapMatched(unmatchedPart);
        diffMatched = matchedE.diff(lastMatched);

        if (!diffMatched.empty()) {   
          unmatchedE = allEdges.diff(matchedE);
        }
      } 

      matchedF = mapF.image(matchedE);
      matchedU = mapU.image(matchedE);

      diffU = (mapU.image(allEdges)).diff(matchedU);

    } while (!diffU.empty());
  }
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
  minReachable();

  return matchedE;
}
