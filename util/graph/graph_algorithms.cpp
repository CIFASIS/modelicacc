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

  VertexIt vi_start, vi_end;
  boost::tie(vi_start, vi_end) = vertices(g);

  NI1 n = 0;

  for (; vi_start != vi_end; ++vi_start) {
    SetVertex v = g[*vi_start];

    Set vs = v.vs_();
    n += vs.size();
  }

  EdgeIt ei_start, ei_end;
  boost::tie(ei_start, ei_end) = edges(g);

  for (; ei_start != ei_end; ++ei_start) {
    PWLMap fmap = (g[*ei_start]).es1_();
    PWLMap umap = (g[*ei_start]).es2_();

    umap = umap.offsetImage(n);

    mapF = mapF.concat(fmap);
    mapU = mapU.concat(umap);

    SetVertexDesc dv = source(*ei_start, g);
    SetVertex s = g[dv];
    Set vs = s.vs_();
    SetVertexDesc dt = target(*ei_start, g);
    SetVertex t = g[dt];
    Set ts = t.vs_();

    Set wDom = fmap.wholeDom();
    Set im = fmap.image(wDom);

    Set scap = vs.cap(im);
    if (!scap.empty()) { // Edges are undirected, therefore source or target can be the left side
      ts = ts.offset(n);
      SetVertex svt(t.name, ts);
      g[dt] = svt; 
    }

    else {
      vs = vs.offset(n);
      SetVertex svv(s.name, vs);
      g[dv] = svv; 
    }
  }

  Set emptySet;
  matchedE = emptySet;
}

PWLMap MatchingStruct::minLeft(Set E)
{
  PWLMap auxMapF = mapF.restrictMap(E);
  PWLMap auxMapU = mapU.restrictMap(E);
  PWLMap res = minAdjMap(auxMapU, auxMapF);

  return res;
}

void MatchingStruct::swapMatched(PWLMap augmentingPaths)
{
  PWLMap edgesFrep = augmentingPaths.compPW(mapF);
  PWLMap edgesUrep = augmentingPaths.compPW(mapU);

  Set edgesInPath = edgesFrep.sameImage(edgesUrep);

  Set matchedEInPath = edgesInPath.cap(matchedE);
  Set unmatchedEInPath = edgesInPath.diff(matchedEInPath);

  matchedE = matchedE.diff(matchedEInPath);
  matchedE = matchedE.cup(unmatchedEInPath);
}

PWLMap MatchingStruct::minReachable()
{
  PWLMap rmap;

  VertexIt vi_start, vi_end;
  boost::tie(vi_start, vi_end) = vertices(g);
  EdgeIt ei_start, ei_end;
  boost::tie(ei_start, ei_end) = edges(g);

  if(vi_start != vi_end){
    Set allEdges = mapF.wholeDom();
    Set En = allEdges.diff(matchedE);

    Set F = mapF.image(allEdges);
    Set U = mapU.image(allEdges);
    Set allVertices = F.cup(U);

    PWLMap auxmap1(allVertices);
    rmap = auxmap1;

    Set lastMatched, lastIm;
    Set newIm = allVertices;
    Set diffMatched, diffIm;

    do {
      PWLMap unmatchedU = mapU.restrictMap(En);
      PWLMap matchedU = mapU.restrictMap(matchedE);
      PWLMap unmatchedF = mapF.restrictMap(En);
      PWLMap matchedF = mapF.restrictMap(matchedE);

      PWLMap URn = rmap.compPW(unmatchedU);
      PWLMap URm = rmap.compPW(matchedU);
      PWLMap FRn = rmap.compPW(unmatchedF);
      PWLMap FRm = rmap.compPW(matchedF);

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

        lastMatched = matchedE;
        swapMatched(rmap);
        diffMatched = matchedE.diff(lastMatched);

        if (!diffMatched.empty()) {   
          En = allEdges.diff(matchedE);
        }
      } 
    } while (!diffIm.empty());
  }

  return rmap;
}

Set MatchingStruct::SBGMatching()
{
  PWLMap comps = minReachable();

  return matchedE;
}

