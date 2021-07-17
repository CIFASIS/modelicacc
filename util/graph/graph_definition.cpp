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

#include <util/graph/graph_definition.h>

namespace SBG {
size_t hash_value(SetVertex v) { return v.hash(); }

/*-----------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
// Printing instances
/*-----------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/

std::ostream &operator<<(std::ostream &out, SetVertex &V)
{
  std::string Vnm = V.name_();
  Set Vvs = V.vs_();

  out << Vnm << ": " << Vvs << "\n";

  return out;
}

std::ostream &operator<<(std::ostream &out, SetEdge &E)
{
  std::string Enm = E.name_();
  OrdCT<Set> dom = E.es1_().dom();
  OrdCT<Set>::iterator itdom = dom.begin();

  out << Enm << " dom: ";
  out << "[";
  for (; next(itdom, 1) != dom.end(); ++itdom)
    out << *itdom << ", ";
  out << *itdom << "]\n";

  OrdCT<LMap> lmleft = E.es1_().lmap();
  OrdCT<LMap>::iterator itleft = lmleft.begin();
  OrdCT<LMap> lmright = E.es2_().lmap();
  OrdCT<LMap>::iterator itright = lmright.begin();

  out << Enm << " left | right: ";
  out << "[";
  for (; next(itleft, 1) != lmleft.end(); ++itleft)
    out << *itleft << ", ";
  out << *itleft << "] | ";

  out << "[";
  for (; next(itright, 1) != lmright.end(); ++itright)
    out << *itright << ", ";
  out << *itright << "]\n";

  return out;
}

/*-----------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
// Map operations
/*-----------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/

// This operations are implemented here to simplify the implementation.

PWLMap offsetMap(OrdCT<INT> &offElem, PWLMap &pw) 
{
  OrdCT<LMap> reslm;
  OrdCT<LMap>::iterator itreslm = reslm.begin();

  OrdCT<INT>::iterator itelem;

  if ((int) offElem.size() == pw.ndim()) {
    itelem = offElem.begin();

    BOOST_FOREACH (LMap lm, pw.lmap()) {
      OrdCT<REAL> resg;
      OrdCT<REAL>::iterator itresg = resg.begin();
      OrdCT<REAL> reso;
      OrdCT<REAL>::iterator itreso = reso.begin();

      BOOST_FOREACH (REAL gi, lm.gain()) {
        itresg = resg.insert(itresg, gi);

        ++itresg;
      }

      BOOST_FOREACH (REAL oi, lm.offset()) {
        itreso = reso.insert(itreso, oi + (REAL) *itelem);

        ++itreso; 
        ++itelem;
      }

      itreslm = reslm.insert(itreslm, LMap(resg, reso));
    }
  }

  return PWLMap(pw.dom(), reslm);
}

// This function calculates if pw1 and pw2 are equivalent
// (have the same whole dom, and the image of each dom is
// the same)
// e.g. [({1:1:10}, [1 * x + 0])] and 
//      [({1:1:3}, [1 * x + 0]), ({4:1:10}, [1 * x + 0])]
//      are equivalent
// but [({1:1:10}, [1 * x + 0])] and
//     [({1:1:10}, [0 * x + 1])]
//     aren't equivalent
bool equivalentPW(PWLMap pw1, PWLMap pw2)
{
  Set dom1 = pw1.wholeDom();
  Set dom2 = pw2.wholeDom();

  if (dom1 == dom2) {
    OrdCT<LMap> lm1 = pw1.lmap();
    OrdCT<LMap>::iterator itlm1 = lm1.begin();

    BOOST_FOREACH (Set d1, pw1.dom()) {
      OrdCT<LMap> lm2 = pw2.lmap();
      OrdCT<LMap>::iterator itlm2 = lm2.begin();

      BOOST_FOREACH (Set d2, pw2.dom()) { 
        Set domcap = d1.cap(d2);

        PWLMap auxpw1;
        auxpw1.addSetLM(domcap, *itlm1);
        PWLMap auxpw2;
        auxpw2.addSetLM(domcap, *itlm2);

        Set im1 = auxpw1.image(domcap);
        Set im2 = auxpw2.image(domcap);

        if (im1 != im2)
          return false;

        ++itlm2;
      }

      ++itlm1;
    }

    return true;
  }

  return false;
}

PWLMap minAtomPW(AtomSet &dom, LMap &lm1, LMap &lm2)
{
  OrdCT<REAL> g1 = lm1.gain();
  OrdCT<REAL> o1 = lm1.offset();
  OrdCT<REAL>::iterator ito1 = o1.begin();
  OrdCT<REAL> g2 = lm2.gain();
  OrdCT<REAL>::iterator itg2 = g2.begin();
  OrdCT<REAL> o2 = lm2.offset();
  OrdCT<REAL>::iterator ito2 = o2.begin();
  OrdCT<Interval> ints = dom.aset_ref().inters();
  OrdCT<Interval>::iterator itints = ints.begin();

  AtomSet asAux = dom;
  LMap lmAux = lm1;
  OrdCT<REAL> resg = g1;
  OrdCT<REAL> reso = o1;
  int count = 1;

  OrdCT<Set> domRes;
  OrdCT<LMap> lmRes;

  if (lm1.ndim() == lm2.ndim()) {
    BOOST_FOREACH (REAL g1i, g1) {
      if (g1i != *itg2) {
        REAL xinter = (*ito2 - *ito1) / (g1i - *itg2);

        // Intersection before domain
        if (xinter <= (*itints).lo()) {
          if (*itg2 < g1i) lmAux = lm2;

          Set sAux;
          sAux.addAtomSet(asAux);

          domRes.insert(domRes.begin(), sAux);
          lmRes.insert(lmRes.begin(), lmAux);
        }

        // Intersection after domain
        else if (xinter >= (*itints).hi()) {
          if (*itg2 > g1i) lmAux = lm2;

          Set sAux;
          sAux.addAtomSet(asAux);

          domRes.insert(domRes.begin(), sAux);
          lmRes.insert(lmRes.begin(), lmAux);
        }

        // Intersection in domain
        else {
          Interval i1((*itints).lo(), (*itints).step(), floor(xinter));
          Interval i2(i1.hi() + i1.step(), (*itints).step(), (*itints).hi());

          AtomSet as1 = asAux.replace(i1, count);
          AtomSet as2 = asAux.replace(i2, count);

          Set d1;
          d1.addAtomSet(as1);

          Set d2;
          d2.addAtomSet(as2);

          domRes.insert(domRes.end(), d1);
          domRes.insert(domRes.end(), d2);

          if (g1i > *itg2) {
            lmRes.insert(lmRes.end(), lm1);
            lmRes.insert(lmRes.end(), lm2);
          }

          else {
            lmRes.insert(lmRes.end(), lm2);
            lmRes.insert(lmRes.end(), lm1);
          }
        }

        PWLMap auxRes(domRes, lmRes);
        return auxRes;
      }

      else if (*ito1 != *ito2) {
        if (*ito2 < *ito1) lmAux = lm2;

        Set sAux;
        sAux.addAtomSet(asAux);
        domRes.insert(domRes.begin(), sAux);
        lmRes.insert(lmRes.begin(), lmAux);

        PWLMap auxRes(domRes, lmRes);
        return auxRes;
      }

      ++ito1;
      ++itg2;
      ++ito2;
      ++itints;
      ++count;
    }
  }

  Set sAux;
  sAux.addAtomSet(dom);
  domRes.insert(domRes.begin(), sAux);
  lmRes.insert(lmRes.begin(), lm1);
  PWLMap auxRes(domRes, lmRes);
  return auxRes;
}

PWLMap minPW(Set &dom, LMap &lm1, LMap &lm2)
{
  OrdCT<Set> sres;
  OrdCT<LMap> lres;

  Set sres1;
  Set sres2;
  LMap lres1;
  LMap lres2;

  UnordCT<AtomSet> asets = dom.asets();
  UnordCT<AtomSet>::iterator itas = asets.begin();

  if (!dom.empty()) {
    PWLMap aux;
    AtomSet asAux = *itas;
    aux = minAtomPW(asAux, lm1, lm2);
    if (!aux.empty()) {
      sres1 = *(aux.dom_ref().begin());
      lres1 = *(aux.lmap_ref().begin());
      ++itas;

      OrdCT<Set> d;
      OrdCT<Set>::iterator itd;
      OrdCT<LMap> l;
      OrdCT<LMap>::iterator itl;
      while (itas != asets.end()) {
        asAux = *itas;
        aux = minAtomPW(asAux, lm1, lm2);
        d = aux.dom();
        itd = d.begin();
        l = aux.lmap();
        itl = l.begin();

        while (itd != d.end()) {
          if (*itl == lres1)
            sres1 = sres1.cup(*itd);

          else {
            if (sres2.empty()) {
              sres2 = *itd;
              lres2 = *itl;
            }

            else
              sres2 = sres2.cup(*itd);
          }

          ++itd;
          ++itl;
        }

        ++itas;
      }
    }
  }

  if (!sres1.empty() && !lres1.empty()) {
    sres.insert(sres.end(), sres1);
    lres.insert(lres.end(), lres1);
  }

  if (!sres2.empty() && !lres2.empty()) {
    sres.insert(sres.end(), sres2);
    lres.insert(lres.end(), lres2);
  }

  PWLMap res(sres, lres);
  return res;
}

PWLMap minMap(PWLMap &pw1, PWLMap &pw2)
{
  PWLMap res;

  OrdCT<LMap> lm1 = pw1.lmap();
  OrdCT<LMap>::iterator itl1 = lm1.begin();
  OrdCT<LMap> lm2 = pw2.lmap();

  if (!pw1.empty() && !pw2.empty()) {
    BOOST_FOREACH (Set s1i, pw1.dom()) {
      OrdCT<LMap>::iterator itl2 = lm2.begin();

      BOOST_FOREACH (Set s2j, pw2.dom()) {
        Set dom = s1i.cap(s2j);

        if (!dom.empty()) {
          PWLMap aux = minPW(dom, *itl1, *itl2);

          if (res.empty())
            res = aux;

          else
            res = aux.combine(res);
        }

        ++itl2;
      }

      ++itl1;
    }
  }

  return res;
}

PWLMap minInv(PWLMap &pw, Set &s)
{
  if (!pw.empty()) {
    // Initialization
    OrdCT<Set> auxdom = pw.dom();
    OrdCT<Set>::iterator itdom = auxdom.begin();
    OrdCT<LMap> auxlm = pw.lmap();
    OrdCT<LMap>::iterator itlm = auxlm.begin();

    Set dom1 = *itdom;
    ++itdom;
    LMap lm1 = *itlm;
    ++itlm;

    OrdCT<Set> pw2dom;
    pw2dom.insert(pw2dom.begin(), dom1);
    OrdCT<LMap> pw2lm;
    pw2lm.insert(pw2lm.begin(), lm1);
    PWLMap pw2(pw2dom, pw2lm);

    PWLMap invpw2 = pw2.minInvCompact(s);
    pw2 = invpw2;

    for (; itdom != auxdom.end(); ++itdom) {
      OrdCT<Set> pwidom;
      pwidom.insert(pwidom.begin(), *itdom);
      OrdCT<LMap> pwilm;
      pwilm.insert(pwilm.begin(), *itlm);
      PWLMap pwi(pwidom, pwilm);

      PWLMap invpwi = pwi.minInvCompact(s);

      PWLMap minmap = minMap(pw2, invpwi);
      PWLMap combmap = invpwi.combine(pw2);
      pw2 = combmap;

      if (!minmap.empty()) {
        PWLMap combmap2 = minmap.combine(pw2);
        pw2 = combmap2;
      }

      ++itlm;
    }

    return pw2;
  }

  PWLMap res;
  return res;
}

PWLMap reduceMapN(PWLMap pw, int dim)
{
  OrdCT<Set> sres = pw.dom();
  OrdCT<Set>::iterator itsres = sres.end();
  OrdCT<LMap> lres = pw.lmap();
  OrdCT<LMap>::iterator itlres = lres.end();

  OrdCT<LMap> lm = pw.lmap();
  OrdCT<LMap>::iterator itlm = lm.begin();

  unsigned int i = 1;
  BOOST_FOREACH (Set di, pw.dom()) {
    int count1 = 1;

    OrdCT<REAL> g = (*itlm).gain();
    OrdCT<REAL>::iterator itg = g.begin();
    OrdCT<REAL> o = (*itlm).offset();
    OrdCT<REAL>::iterator ito = o.begin();
    // Get the dim-th gain and offset
    while (count1 < dim) {
      ++itg;
      ++ito;
      ++count1;
    }

    if (*itg == 1 && *ito < 0) {
      REAL off = -(*ito);

      BOOST_FOREACH (AtomSet adom, di.asets()) {
        MultiInterval mi = adom.aset();
        OrdCT<Interval> inters = mi.inters();
        OrdCT<Interval>::iterator itints = inters.begin();

        int count2 = 1;
        while (count2 < dim) {
          ++itints;
          ++count2;
        }

        INT loint = (*itints).lo();
        INT hiint = (*itints).hi();

        if ((hiint - loint) > (off * off)) {
          OrdCT<Set> news;
          OrdCT<Set>::iterator itnews = news.begin();
          OrdCT<LMap> newl;
          OrdCT<LMap>::iterator itnewl = newl.begin();

          for (int k = 1; k <= off; k++) {
            OrdCT<REAL> newo = (*itlm).offset();
            OrdCT<REAL>::iterator itnewo = newo.begin();

            OrdCT<REAL> resg;
            OrdCT<REAL>::iterator itresg = resg.begin();
            OrdCT<REAL> reso;
            OrdCT<REAL>::iterator itreso = reso.begin();

            int count3 = 1;
            BOOST_FOREACH (REAL gi, (*itlm).gain()) {
              if (count3 == dim) {
                itresg = resg.insert(itresg, 0);
                itreso = reso.insert(itreso, loint + k - off - 1);
              }

              else {
                itresg = resg.insert(itresg, gi);
                itreso = reso.insert(itreso, *itnewo);
              }

              ++itresg;
              ++itreso;
              ++itnewo;
              ++count3;
            }

            LMap newlmap(resg, reso);
            Interval newinter(loint + k - 1, off, hiint);
            AtomSet auxas = adom.replace(newinter, dim);
            Set newset;
            newset.addAtomSet(auxas);

            itnews = news.insert(itnews, newset);
            ++itnews;
            itnewl = newl.insert(itnewl, newlmap);
            ++itnewl;
          }

          PWLMap newmap(news, newl);

          UnordCT<AtomSet> auxnewd;
          BOOST_FOREACH (AtomSet auxasi, di.asets()) {
            if (auxasi != adom) auxnewd.insert(auxasi);
          }

          Set newdomi(auxnewd);

          if (newdomi.empty()) {
            itlres = lres.begin();

            if (i < sres.size()) {
              OrdCT<Set> auxs;
              OrdCT<Set>::iterator itauxs = auxs.begin();
              OrdCT<LMap> auxl;
              OrdCT<LMap>::iterator itauxl = auxl.begin();

              unsigned int count4 = 1;
              BOOST_FOREACH (Set si, sres) {
                if (count4 != i) {
                  itauxs = auxs.insert(itauxs, si);
                  ++itauxs;
                  itauxl = auxl.insert(itauxl, *itlres);
                  ++itauxl;
                }

                ++count4;
                ++itlres;
              }

              sres = auxs;
              lres = auxl;
            }

            else {
              OrdCT<Set> auxs;
              OrdCT<Set>::iterator itauxs = auxs.begin();
              OrdCT<LMap> auxl;
              OrdCT<LMap>::iterator itauxl = auxl.begin();

              unsigned int count4 = 1;
              BOOST_FOREACH (Set si, sres) {
                if (count4 < i) {
                  itauxs = auxs.insert(itauxs, si);
                  ++itauxs;
                  itauxl = auxl.insert(itauxl, *itlres);
                  ++itauxl;
                }

                ++count4;
                ++itlres;
              }

              sres = auxs;
              lres = auxl;
            }
          }

          else {
            OrdCT<Set> auxs;
            OrdCT<Set>::iterator itauxs = auxs.begin();
            OrdCT<Set>::iterator itauxsres = sres.begin();
            unsigned int count5 = 1;
            while (itauxsres != sres.end()) {
              if (count5 == i)
                itauxs = auxs.insert(itauxs, newdomi);

              else
                itauxs = auxs.insert(itauxs, *itauxsres);

              ++itauxs;
              ++itauxsres;
              ++count5;
            }

            sres = auxs;
          }

          BOOST_FOREACH (Set newi, newmap.dom()) {
            itsres = sres.insert(itsres, newi);
            ++itsres;
          }

          BOOST_FOREACH (LMap newi, newmap.lmap()) {
            itlres = lres.insert(itlres, newi);
            ++itlres;
          }
        }
      }
    }

    ++itlm;
    ++i;
  }

  PWLMap res(sres, lres);
  return res;
}

PWLMap mapInf(PWLMap pw)
{
  PWLMap res;
  if (!pw.empty()) {
    res = reduceMapN(pw, 1);

    for (int i = 2; i <= res.ndim(); ++i) res = reduceMapN(res, i);

    int maxit = 0;

    OrdCT<Set> doms = res.dom();
    OrdCT<Set>::iterator itdoms = doms.begin();
    BOOST_FOREACH (LMap lm, res.lmap()) {
      OrdCT<REAL> o = lm.offset();
      OrdCT<REAL>::iterator ito = o.begin();

      REAL a = 0;
      REAL b = *(lm.gain().begin());

      BOOST_FOREACH (REAL gi, lm.gain()) {
        a = std::max(a, gi * abs(*ito));
        b = std::min(b, gi);

        ++ito;
      }

      ito = o.begin();
      if (a > 0) {
        REAL its = 0;

        OrdCT<REAL> g = lm.gain();
        OrdCT<REAL>::iterator itg = g.begin();
        // For intervals in which size <= off ^ 2 (check reduceMapN, this intervals are not "reduced")
        for (int dim = 0; dim < res.ndim(); ++dim) {
          if (*itg == 1 && *ito != 0) {
            BOOST_FOREACH (AtomSet asi, (*itdoms).asets()) {
              MultiInterval mii = asi.aset();
              OrdCT<Interval> ii = mii.inters();
              OrdCT<Interval>::iterator itii = ii.begin();

              for (int count = 0; count < dim; ++count) ++itii;

              its = std::max(its, ceil(((*itii).hi() - (*itii).lo()) / abs(*ito)));
            }
          }

          // else
          //  ++maxit;

          ++itg;
          ++ito;
        }

        maxit += its;
      }

      else if (b == 0)
        ++maxit;

      ++itdoms;
    }

    if (maxit == 0)
      return res;

    else
      maxit = floor(log2(maxit)) + 1;

    for (int j = 0; j < maxit; ++j) res = res.compPW(res);
  }

  return res;
}

// pw3, pw2, pw1 are such that:
//   pw3 : A -> B, and is a compact map
//   pw2 : A -> C
//   pw1 : C -> D
PWLMap minAdjCompMap(PWLMap pw3, PWLMap pw2, PWLMap pw1)
{
  PWLMap res;

  OrdCT<Set> auxd = pw3.dom();
  int auxsize = auxd.size();
  if (auxsize == 1) {
    Set dom = *(pw3.dom_ref().begin());
    LMap lm = *(pw3.lmap_ref().begin());

    Set dominv = pw3.image(dom);
    LMap lminv = lm.invLMap();
    OrdCT<REAL> lminvo = lminv.offset();

    PWLMap invpw;
    invpw.addSetLM(dominv, lminv);

    REAL maxg = *(lminv.gain().begin());
    REAL ming = maxg;
    BOOST_FOREACH (REAL gi, lminv.gain()) {
      maxg = std::max(maxg, gi);
      ming = std::min(ming, gi);
    }

    // Bijective map, therefore, it's invertible
    if (maxg < Inf) {
      res = pw2.compPW(invpw);
    }

    // Constant map
    else if (ming == Inf) {
      if (!pw3.empty()) {
        Set im2 = pw2.image(dom);
        Set compim12 = pw1.image(im2);

        // Get vertices in image of pw2 with minimum image in pw1
        OrdCT<INT> mincomp = compim12.minElem();
        MultiInterval micomp;
        BOOST_FOREACH (INT mincompi, mincomp) {
          Interval i(mincompi, 1, mincompi);
          micomp.addInter(i);
        }
        AtomSet ascomp(micomp);
        Set scomp;
        scomp.addAtomSet(ascomp);
        Set mins2 = pw1.preImage(scomp);

        // Choose minimum in mins2, and assign dom(pw1) this element as image
        OrdCT<INT> min2 = mins2.minElem();
        OrdCT<INT>::iterator itmin2 = min2.begin();

        OrdCT<REAL> reso;
        OrdCT<REAL>::iterator itreso = reso.begin();
        OrdCT<REAL> resg;
        OrdCT<REAL>::iterator itresg = resg.begin();
        for (int i = 0; i < dominv.ndim(); ++i) {
          itresg = resg.insert(itresg, 0);
          ++itresg;
          itreso = reso.insert(itreso, (REAL)(*itmin2));
          ++itreso;

          ++itmin2;
        }

        res.addSetLM(dominv, LMap(resg, reso));
      }
    }

    // Bijective in some dimensions, and constant in others
    else {
      OrdCT<REAL>::iterator itinvo = lminvo.begin();
      OrdCT<REAL> newinvg;
      OrdCT<REAL>::iterator itnewinvg = newinvg.begin();
      OrdCT<REAL> newinvo;
      OrdCT<REAL>::iterator itnewinvo = newinvo.begin();
      OrdCT<INT> mindom = dom.minElem();
      OrdCT<INT>::iterator itmindom = mindom.begin();

      // Replace inverse of constant maps for composition (with a dummy value)
      BOOST_FOREACH (REAL invgi, lminv.gain()) {
        if (invgi >= Inf) {
          itnewinvg = newinvg.insert(itnewinvg, 0);
          itnewinvo = newinvo.insert(itnewinvo, (REAL) (*itmindom));
        }

        else {
          itnewinvg = newinvg.insert(itnewinvg, invgi);
          itnewinvo = newinvo.insert(itnewinvo, *itinvo);
        }

        ++itinvo;
        ++itnewinvg;
        ++itnewinvo;
        ++itmindom;
      }

      LMap newinvlm(newinvg, newinvo);
      PWLMap newinvpw;
      newinvpw.addSetLM(dominv, newinvlm);

      // Compose
      PWLMap auxres = pw2.compPW(newinvpw);

      OrdCT<Set> domaux = auxres.dom();
      OrdCT<LMap> lmaux = auxres.lmap();
      OrdCT<LMap>::iterator itlmaux = lmaux.begin();

      OrdCT<LMap> lmres;
      OrdCT<LMap>::iterator itlmres = lmres.begin();

      // Replace values of constant maps with the desired value
      Set im2 = pw2.image(dom);
      Set compim12 = pw1.image(im2);

      // Get vertices in image of pw2 with minimum image in pw1
      OrdCT<INT> mincomp = compim12.minElem();
      MultiInterval micomp;
      BOOST_FOREACH (INT mincompi, mincomp) {
        Interval i(mincompi, 1, mincompi);
        micomp.addInter(i);
      }
      AtomSet ascomp(micomp);
      Set scomp;
      scomp.addAtomSet(ascomp);
      Set mins2 = pw1.preImage(scomp);

      // Choose minimum in min2
      // Assign dom(pw1) this element as image
      OrdCT<INT> min2 = mins2.minElem();
      OrdCT<INT>::iterator itmin2 = min2.begin();

      BOOST_FOREACH (Set domauxi, domaux) {
        OrdCT<REAL> replaceg;
        OrdCT<REAL>::iterator itrepg = replaceg.begin();
        OrdCT<REAL> replaceo;
        OrdCT<REAL>::iterator itrepo = replaceo.begin();

        OrdCT<REAL> auxg = (*itlmaux).gain();
        OrdCT<REAL>::iterator itauxg = auxg.begin();
        OrdCT<REAL> auxo = (*itlmaux).offset();
        OrdCT<REAL>::iterator itauxo = auxo.begin();

        BOOST_FOREACH (REAL invgi, lminv.gain()) {
          // i-th dimension constant
          if (invgi >= Inf) {
            itrepg = replaceg.insert(itrepg, 0);
            itrepo = replaceo.insert(itrepo, (REAL)(*itmin2));
          }

          // i-th dimension bijective
          else {
            itrepg = replaceg.insert(itrepg, *itauxg);
            itrepo = replaceo.insert(itrepo, *itauxo);
          }

          ++itauxg;
          ++itauxo;
          ++itrepg;
          ++itrepo;
          ++itmin2;
        }

        itlmres = lmres.insert(itlmres, LMap(replaceg, replaceo));
        LMap den(replaceg, replaceo);

        ++itlmaux;
        ++itlmres;
      }

      PWLMap replacedpw(auxres.dom(), lmres);
      res = replacedpw;
    }
  }

  return res;
}

// pw2, pw1 are such that:
//   pw2 : A -> B, and is a compact map
//   pw1 : A -> C
PWLMap minAdjCompMap(PWLMap pw2, PWLMap pw1)
{
  Set dom1 = pw1.wholeDom();
  Set im1 = pw1.image(dom1);
  PWLMap idmap(im1);

  return minAdjCompMap(pw2, pw1, idmap);
}

PWLMap minAdjMap(PWLMap pw3, PWLMap pw2, PWLMap pw1)
{
  PWLMap res;

  if (!pw3.empty()) {
    OrdCT<Set> dom = pw3.dom();
    OrdCT<Set>::iterator itdom = dom.begin();
    OrdCT<LMap> lm = pw3.lmap();
    OrdCT<LMap>::iterator itlm = lm.begin();

    Set auxdom = *itdom;
    LMap auxlm = *itlm;

    PWLMap map31;
    map31.addSetLM(auxdom, auxlm);

    res = minAdjCompMap(map31, pw2, pw1);
    ++itdom;
    ++itlm;

    PWLMap minAdj;
    PWLMap minM;
    while (itdom != dom.end()) {
      PWLMap map3i;
      map3i.addSetLM(*itdom, *itlm);
      minAdj = minAdjCompMap(map3i, pw2, pw1);
      minM = minMap(res, minAdj);

      res = minAdj.combine(res);

      if (!minM.empty()) res = minM.combine(res);

      ++itdom;
      ++itlm;
    }
  }

  return res;
}

PWLMap minAdjMap(PWLMap pw2, PWLMap pw1)
{
  /*
  BOOST_FOREACH(Set domi, pw1.dom_()) {
    PWLMap pwi(domi);
    idmap = idmap.concat(pwi);
  }
  */

  Set dom1 = pw1.wholeDom();
  Set im1 = pw1.image(dom1);
  PWLMap idmap(im1);

  return minAdjMap(pw2, pw1, idmap);
}

SetEdge restrictEdge(SetEdge e, Set dom)
{
  PWLMap es1 = e.es1_();
  PWLMap es2 = e.es2_();

  PWLMap res1 = es1.restrictMap(dom);
  PWLMap res2 = es2.restrictMap(dom);

  SetEdge res(e.name, e.id_(), res1, res2, 0);
  return res;
}

}  // namespace SBG
