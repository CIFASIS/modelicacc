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

  if (vi_start != vi_end /*&& ei_start != ei_end*/) {
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
        res = mapInf(res);
        newIm = res.image(vss);
      }
    }
  }

  return res;
}

// Matching ---------------------------------------------------------------------------------------

UnordCT<Set> MatchingStruct::split(Set ftilde)
{
  UnordCT<Set> res;

  // Find the set-vertex where ftilde is included
  VertexIt vi_start, vi_end;
  boost::tie(vi_start, vi_end) = vertices(g);

  for (; vi_start != vi_end; ++vi_start) {
    SetVertex v = g[*vi_start];

    Set vs = v.vs_();
    Set inter = vs.cap(ftilde);
    if (!inter.empty()) break;
  }

  // Find set-edges connected to the set-vertex
  OutEdgeIt ei_start, ei_end;
  boost::tie(ei_start, ei_end) = boost::out_edges(*vi_start, g);

  for (; ei_start != ei_end; ++ei_start) {
    SetEdge e = g[*ei_start];
    PWLMap es1 = e.es1_();

    Set pre = es1.preImage(ftilde);
    Set etilde = pre.diff(matchedE);
    res.insert(etilde);
  }

  return res;
}

MatchingStruct::MatchingStruct(SBGraph garg)
{
  g = garg;

  EdgeIt ei_start, ei_end;
  boost::tie(ei_start, ei_end) = edges(g);

  for (; ei_start != ei_end; ++ei_start) {
    PWLMap fmap = (g[*ei_start]).es1_();
    PWLMap umap = (g[*ei_start]).es2_();

    mapF = mapF.concat(fmap);
    mapU = mapU.concat(umap);

    SetVertex s = g[source(*ei_start, g)];
    Set vs = s.vs_();
    SetVertex t = g[target(*ei_start, g)];
    Set ts = t.vs_();

    Set wDom = fmap.wholeDom();
    Set im = fmap.image(wDom);

    Set scap = vs.cap(im);
    if (!scap.empty())  // Edges are undirected, therefore source or target can be the left side
      auxF.insert(vs);

    else
      auxF.insert(ts);
  }

  wmax = 0;

  Set emptySet;

  matchedF = emptySet;
  matchedU = emptySet;

  UnordCT<Set> emptyUnordSet;
  visitedU = emptyUnordSet;

  currentF = emptyUnordSet;

  matchedE = emptySet;

  SetPath emptyPath;
  Pmax = emptyPath;
}

SetPath MatchingStruct::waspf(Set ftilde)
{
  SetPath pmax;

  if (!ftilde.empty()) {
    UnordCT<Set> etilde = split(ftilde);

    BOOST_FOREACH (Set eh, etilde) {
      Set uh = mapU.image(eh);
      Set uhn = uh.diff(matchedU);

      int sz = uhn.size();
      if (sz > wmax) {
        wmax = sz;

        SetPath auxp;

        PWLMap mapUaux = mapU.restrictMap(eh);
        PWLMap auxinv = minInv(mapUaux, uhn);
        Set auxinvim = auxinv.image(uhn);

        auxp.insert(auxp.begin(), auxinvim);
        pmax = auxp;
      }
    }

    BOOST_FOREACH (Set eh, etilde) {
      Set uh = mapU.image(eh);
      Set uhn = uh.diff(matchedU);
      Set uhm = uh.cap(matchedU);

      bool completelyVisited = false;
      BOOST_FOREACH (Set svisited, visitedU) {
        if (uhm.subseteq(svisited)) completelyVisited = true;
      }

      if (!completelyVisited) {
        if (uhm.size() > wmax) {
          visitedU.insert(uhm);
          SetPath phat = waspu(uhm);

          if (!phat.empty()) {
            Set p1 = *(phat.begin());

            if (p1.size() > wmax) {
              wmax = uhm.size();
              Set imp1 = mapU.image(p1);
              PWLMap mapUaux = mapU.restrictMap(eh);
              PWLMap auxinv = minInv(mapUaux, imp1);
              Set auxinvim = auxinv.image(imp1);

              phat.insert(phat.begin(), auxinvim);
              pmax = phat;
            }
          }
        }
      }
    }
  }

  return pmax;
}

SetPath MatchingStruct::waspu(Set utilde)
{
  SetPath pmax;

  Set auxs1 = mapU.preImage(utilde);
  Set auxs2 = auxs1.cap(matchedE);
  Set Ftilde = mapF.image(auxs2);

  BOOST_FOREACH (Set sf, auxF) {
    Set auxFi = sf.cap(Ftilde);

    if (auxFi.size() > wmax) {
      bool notinpath = true;
      BOOST_FOREACH (Set auxs, currentF) {
        if (auxs == auxFi) {
          notinpath = false;
          break;
        }
      }

      if (notinpath) {
        currentF.insert(auxFi);
        SetPath phat = waspf(auxFi);
        currentF.erase(auxFi);

        if (!phat.empty()) {
          Set p1 = *(phat.begin());
          int sz = p1.size();
          if (sz > wmax) {
            wmax = sz;
            Set imp1 = mapF.image(p1);
            Set predom = mapF.preImage(imp1);
            Set matchedom = predom.cap(matchedE);

            phat.insert(phat.begin(), matchedom);
            pmax = phat;
          }
        }
      }
    }
  }

  return pmax;
}

Set MatchingStruct::SBGMatching()
{
  VertexIt vi_start, vi_end;
  boost::tie(vi_start, vi_end) = vertices(g);

  if (vi_start != vi_end) {
    SetPath P;

    UnordCT<Set> F = auxF;

    do {
      Set ftilde = *(auxF.begin());  // TODO: hacer bien la heurística

      UnordCT<Set> emptyUnordSet;
      visitedU = emptyUnordSet;

      wmax = 0;

      emptyUnordSet.insert(ftilde);
      currentF = emptyUnordSet;

      P = waspf(ftilde);

      if (!P.empty()) {
        Set p1 = *(P.begin());
        Set newFm = mapF.image(p1);
        Set Fm = matchedF.cup(newFm);
        matchedF = Fm;

        OrdCT<Set>::iterator Pend = P.end();
        Pend--;
        Set pn = *Pend;
        Set newUm = mapU.image(pn);
        Set Um = matchedU.cup(newUm);
        matchedU = Um;

        int l = 1;
        BOOST_FOREACH (Set Pl, P) {
          if ((l % 2) == 1) {
            Set Em = matchedE.cup(Pl);
            matchedE = Em;

            Set currentf = mapF.image(Pl);
            UnordCT<Set> auxFaux = auxF;
            BOOST_FOREACH (Set auxFi, auxFaux) {
              if (currentf.subset(auxFi)) {
                auxF.erase(auxFi);
                Set fiDiff = auxFi.diff(currentf);
                auxF.insert(fiDiff);
              }

              else
                auxF.erase(currentf);
            }
          }

          else {
            Set Em = matchedE.diff(Pl);
            matchedE = Em;
          }

          ++l;
        }
      }
    } while (!P.empty());
  }

  return matchedE;
}
