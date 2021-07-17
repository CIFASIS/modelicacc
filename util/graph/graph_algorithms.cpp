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

    PWLMap oldres;

    while (!equivalentPW(oldres, res)) {
      PWLMap ermap1 = res.compPW(emap1);
      PWLMap ermap2 = res.compPW(emap2);

      PWLMap rmap1 = minAdjMap(ermap1, ermap2);
      PWLMap rmap2 = minAdjMap(ermap2, ermap1);
      rmap1 = rmap1.combine(res);
      rmap2 = rmap2.combine(res);

      PWLMap newRes = minMap(rmap1, rmap2);
      oldres = res;
      res = minMap(newRes, res);

      if (!equivalentPW(oldres, res)) {
        res = newRes;
        res = mapInf(res);
      }
    }
  }

  return res;
}

// Matching ---------------------------------------------------------------------------------------

VertexIt MatchingStruct::findSetVertex(Set matched)
{
  // Find the set-vertex where matched subset is included
  VertexIt vi_start, vi_end;
  boost::tie(vi_start, vi_end) = vertices(g);

  for (; vi_start != vi_end; ++vi_start) {
    SetVertex v = g[*vi_start];
    Set vs = v.vs_();
    Set inter = vs.cap(matched);
    if (!inter.empty()) {
      return vi_start;
    }
  }
  // A given subset should be included in one of the graph vertex, this should never happen.
  assert(false);
  return vi_start;
}

Set MatchingStruct::wholeVertex(Set matched_subset)
{
  Set whole_vertex;
  // Find the set-vertex where the matched subset is included
  VertexIt matched_vertex = findSetVertex(matched_subset);
  SetVertex v = g[*matched_vertex];
  return v.vs_();
}

Set MatchingStruct::wholeEdge(Set matched_subset)
{
  Set whole_edge_split;
  // Find set-edges connected to the set-vertex
  VertexIt vi_start, vi_end;
  boost::tie(vi_start, vi_end) = vertices(g);

  for (; vi_start != vi_end; ++vi_start) {
    OutEdgeIt ei_start, ei_end;
    boost::tie(ei_start, ei_end) = boost::out_edges(*vi_start, g);
    for (; ei_start != ei_end; ++ei_start) {
      SetEdge e = g[*ei_start];
      PWLMap map_u = e.es2_();
      OrdCT<Set> dom = map_u.dom();

      for (Set edge_dom : dom) {
        Set inter = edge_dom.cap(matched_subset);
        if (!inter.empty()) {
          whole_edge_split = whole_edge_split.cup(edge_dom);
        }
      }
      if (!whole_edge_split.empty()) {
        return whole_edge_split;
      }
    }
  }
  return whole_edge_split;
}

Set MatchingStruct::matchedUVar(Set var)
{
  Set matched_u;
  Set var_edges = mapU.preImage(var);
  VertexIt matched_var = findSetVertex(var);

  // Find set-edges connected to the set-vertex
  OutEdgeIt ei_start, ei_end;
  boost::tie(ei_start, ei_end) = boost::out_edges(*matched_var, g);
  for (; ei_start != ei_end; ++ei_start) {
    SetEdge e = g[*ei_start];
    PWLMap map_u = e.es2_();
    OrdCT<Set> map_u_dom = map_u.dom();
    for (Set edge_dom : map_u_dom) {
      Set matched_dom_edges = edge_dom.cap(matchedE);
      Set map_u_image = map_u.image(matched_dom_edges); 
      matched_u = matched_u.cup(map_u_image);
    }
  }
  return matched_u;
}

bool MatchingStruct::checkRecursion(Set candidate_u)
{
  int path_item = 1;
  Set whole_u = wholeVertex(candidate_u);
  for (Set edge_set : Pmax) {
    if ((path_item % 2) == 1) {
      Set visited_node = wholeVertex(mapU.image(edge_set));
      if (visited_node.subseteq(whole_u)) {
        return true;  
      }
    }  
    ++path_item;
  }
  return false;
}

int MatchingStruct::pathWidth()
{
  return Pmax.front().card();
}

bool MatchingStruct::matchingLookAhead(Set matched_f, Set matched_u)
{
  Set possible_match = wholeEdge(matched_f); 
  Set whole_matched_u = matchedUVar(wholeVertex(matched_u));
  return (possible_match.card() + pathWidth()) > whole_matched_u.card();
}

SetPath MatchingStruct::pathTo(Set var)
{
  SetPath path_to;
  int path_item = 1;
  Set whole_u = wholeVertex(var);
  for (Set edge_set : Pmax) {
    path_to.insert(path_to.end(), edge_set);
    if ((path_item % 2) == 1) {
      Set visited_node = wholeVertex(mapU.image(edge_set));
      if (visited_node.subseteq(whole_u)) {
        break;  
      }
    }  
    ++path_item;
  }
  return path_to;
}

void MatchingStruct::updatePath(SetPath path)
{
  for(Set edges: path) {
    Pmax.insert(Pmax.end(), edges);
  }
}

UnordCT<Set> MatchingStruct::split(Set ftilde)
{
  UnordCT<Set> res;

  // Find the set-vertex where ftilde is included
  VertexIt vi_start = findSetVertex(ftilde);

  // Find set-edges connected to the set-vertex
  OutEdgeIt ei_start, ei_end;
  boost::tie(ei_start, ei_end) = boost::out_edges(*vi_start, g);

  Set aux = mapF.preImage(ftilde);
  for (; ei_start != ei_end; ++ei_start) {
    SetEdge e = g[*ei_start];

    PWLMap es2 = e.es2_();
    OrdCT<Set> dome2 = es2.dom();

    BOOST_FOREACH (Set d2, dome2) {
      Set etildei = d2.diff(matchedE);
      Set etildej = etildei.cap(aux);
      if (!etildej.empty()) res.insert(etildej);

      // int sze = etilde.size();
      // int szf = ftilde.size();

      // if(sze != szf){
      //  std::cerr << "Assumption 1 not true\n";
      //  UnordCT<Set> emptyRes;
      //  return emptyRes;
      //}
    }
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
    if (!scap.empty()) {  // Edges are undirected, therefore source or target can be the left side
      auxF.insert(vs);
    } else {
      auxF.insert(ts);
    }
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
    
    SetPath widest_path_from_split;
    BOOST_FOREACH (Set eh, etilde) {
      Set uh = mapU.image(eh);
      Set uhn = uh.diff(matchedU);

      int sz = uhn.card();
      if (sz > wmax) {
        wmax = sz;
        widest_path_from_split.clear(); 
        

        PWLMap mapUaux = mapU.restrictMap(eh);
        PWLMap auxinv = minInv(mapUaux, uhn);
        Set auxinvim = auxinv.image(uhn);

        widest_path_from_split.insert(widest_path_from_split.begin(), auxinvim);
      }
    }
    if (!widest_path_from_split.empty()) {
      return widest_path_from_split;
    }

    BOOST_FOREACH (Set eh, etilde) {
      Set uh = mapU.image(eh);
      Set uhn = uh.diff(matchedU);
      Set uhm = uh.cap(matchedU);
      bool completelyVisited = false;
      BOOST_FOREACH (Set svisited, visitedU) {
        if (uhm.subseteq(svisited)) completelyVisited = true;
      }

      if (!completelyVisited && uhm.card() > wmax) {
        Pmax.insert(Pmax.end(), eh);
        if (checkRecursion(uhm)) {
          if (matchingLookAhead(ftilde, uhm)) {
            Set whole_edge = wholeEdge(ftilde);
            Set update_matching = matchedE.diff(whole_edge);
            matchedE = update_matching;
            PWLMap map_f = mapF.restrictMap(whole_edge);
            Set matched_f = map_f.image(whole_edge);
            auxF.insert(matched_f);
            PWLMap map_u_var = mapU.restrictMap(whole_edge);
            Set remove_matched_u = map_u_var.image(whole_edge);
            Set update_matched_u = matchedU.diff(remove_matched_u);
            matchedU = update_matched_u;
            return pathTo(uhm); 
          }
        } else {
          visitedU.insert(uhm);
          SetPath phat = waspu(uhm);

          if (!phat.empty()) {
            Set p1 = *(phat.begin());

            if (p1.card() >= wmax) {
              wmax = uhm.card();
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

  VertexIt vi_start, vi_end;
  boost::tie(vi_start, vi_end) = vertices(g);

  for (; vi_start != vi_end; ++vi_start) {
    SetVertex v = g[*vi_start];
    Set vs = v.vs_();

    Set auxFi = vs.cap(Ftilde);

    if (auxFi.card() > wmax) {
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
          int sz = p1.card();
          if (sz >= wmax) {
            wmax = sz;
            Set imp1 = mapF.image(p1);
            Set predom = mapF.preImage(imp1);
            Set matchedom = predom.cap(matchedE);

            phat.insert(phat.begin(), matchedom);
            pmax = phat;
            updatePath(pmax);
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
    Set wholeF = mapF.wholeDom();

    UnordCT<Set> F = auxF;

    do {
      Set ftilde = *(auxF.begin());
      int maxcard = ftilde.card();

      BOOST_FOREACH (Set auxFi, auxF) {
        int cardi = auxFi.card();
        if (cardi > maxcard) {
          maxcard = cardi;
          ftilde = auxFi;
        }
      }

      UnordCT<Set> emptyUnordSet;
      visitedU = emptyUnordSet;

      wmax = 0;

      emptyUnordSet.insert(ftilde);
      currentF = emptyUnordSet;

      SetPath emptyPath;
      Pmax = emptyPath;
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
              } else {
                auxF.erase(currentf);
              }
            }
          } else {
            Set Em = matchedE.diff(Pl);
            matchedE = Em;
          }
          ++l;
        }
      } else {
        auxF.erase(ftilde);
      }
    } while (!auxF.empty());
  }

  return matchedE;
}
