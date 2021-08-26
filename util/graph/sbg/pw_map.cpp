/*****************************************************************************
 
    This file is part of the Set-Based Graph library.

******************************************************************************/

#include <boost/foreach.hpp>

#include <util/graph/sbg/pw_map.h>

namespace SBG {

// Piecewise linear maps --------------------------------------------------------------------------

#define SETS_TYPE                  \
   typename PW_TEMP_TYPE::Sets

#define LMAPS_TYPE                 \
   typename PW_TEMP_TYPE::LMaps

PW_TEMPLATE
PW_TEMP_TYPE::PWLMapImp1() {}

PW_TEMPLATE
PW_TEMP_TYPE::PWLMapImp1(SETS_TYPE dom, LMAPS_TYPE lmap)
{
  LMapsIt itl = lmap.begin();
  int dim1 = (*(dom.begin())).ndim();
  bool different = false;

  if (dom.size() == lmap.size()) {
    BOOST_FOREACH (SET_IMP set, dom) {
      BOOST_FOREACH (AS_IMP as, set.asets()) {
        APW_IMP pwatom(as, *itl);

        if (pwatom.empty()) different = true;
      }

      ++itl;
    }

    if (different) {
      Sets emptySets;
      LMaps emptyLMaps;
      dom_ = emptySets;
      lmap_ = emptyLMaps;
      ndim_ = 0;
    }

    else {
      dom_ = dom;
      lmap_ = lmap;
      ndim_ = dim1;
    }
  }

  else {
    Sets emptySets;
    LMaps emptyLMaps;
    dom_ = emptySets;
    lmap_ = emptyLMaps;
    ndim_ = 0;
  }
}

PW_TEMPLATE
PW_TEMP_TYPE::PWLMapImp1(SET_IMP dom) : ndim_(dom.ndim())
{
  Sets emptySets;
  LMaps emptyLMaps;

  dom_ = emptySets;
  lmap_ = emptyLMaps;

  LM_IMP idlm(dom.ndim());

  dom_ref().insert(dom_ref().begin(), dom);
  lmap_ref().insert(lmap_ref().begin(), idlm);
}

member_imp_temp(PW_TEMPLATE, PW_TEMP_TYPE, SETS_TYPE, dom);
member_imp_temp(PW_TEMPLATE, PW_TEMP_TYPE, LMAPS_TYPE, lmap);
member_imp_temp(PW_TEMPLATE, PW_TEMP_TYPE, int, ndim);

PW_TEMPLATE
void PW_TEMP_TYPE::addSetLM(SET_IMP s, LM_IMP lm)
{
  dom_.insert(dom_ref().end(), s);
  lmap_.insert(lmap_ref().end(), lm);
  PWLMapImp1 auxpw(dom_, lmap_);

  dom_ = auxpw.dom();
  lmap_ = auxpw.lmap();
}

PW_TEMPLATE
void PW_TEMP_TYPE::addLMSet(LM_IMP lm, SET_IMP s)
{
  dom_.insert(dom_ref().begin(), s);
  lmap_.insert(lmap_ref().begin(), lm);
  PWLMapImp1 auxpw(dom_, lmap_);

  dom_ = auxpw.dom();
  lmap_ = auxpw.lmap();
}

PW_TEMPLATE
bool PW_TEMP_TYPE::empty() { return dom_ref().empty() && lmap_ref().empty(); }

PW_TEMPLATE
SET_IMP PW_TEMP_TYPE::wholeDom()
{
  SET_IMP res;

  BOOST_FOREACH (SET_IMP s, dom())
    res = res.cup(s);

  return res;
}

PW_TEMPLATE
SET_IMP PW_TEMP_TYPE::image(SET_IMP s)
{
  LMapsIt itl = lmap_ref().begin();

  SET_IMP res;

  BOOST_FOREACH (SET_IMP ss, dom()) {
    SET_IMP aux1 = ss.cap(s);
    SET_IMP partialRes;

    UNORD_CT<AS_IMP> aux1as = aux1.asets();
    BOOST_FOREACH (AS_IMP as, aux1as) {
      APW_IMP auxMap(as, *itl);
      AS_IMP aux2 = auxMap.image(as);
      partialRes.addAtomSet(aux2);
    }

    res = res.cup(partialRes);

    ++itl;
  }

  return res;
}

PW_TEMPLATE
SET_IMP PW_TEMP_TYPE::preImage(SET_IMP s)
{
  LMapsIt itl = lmap_ref().begin();

  SET_IMP res;

  BOOST_FOREACH (SET_IMP ss, dom()) {
    SET_IMP partialRes;

    UNORD_CT<AS_IMP> ssas = ss.asets();
    BOOST_FOREACH (AS_IMP as1, ssas) {
     APW_IMP auxMap(as1, *itl);

      UNORD_CT<AS_IMP> sas = s.asets();
      BOOST_FOREACH (AS_IMP as2, sas) {
        AS_IMP aux2 = auxMap.preImage(as2);
        partialRes.addAtomSet(aux2);
      }
    }

    res = res.cup(partialRes);

    ++itl;
  }

  return res;
}

// Composition of PWLMaps
PW_TEMPLATE
PW_TEMP_TYPE PW_TEMP_TYPE::compPW(PW_TEMP_TYPE pw2)
{
  LMapsIt itlm1 = lmap_ref().begin();
  LMapsIt itlm2 = pw2.lmap_ref().begin();

  Sets ress;
  SetsIt itress = ress.begin();
  LMaps reslm;
  LMapsIt itreslm = reslm.begin();

  SET_IMP auxDom;
  SET_IMP newDom;

  BOOST_FOREACH (SET_IMP d1, dom()) {
    itlm2 = pw2.lmap_ref().begin();

    BOOST_FOREACH (SET_IMP d2, pw2.dom()) {
      auxDom = pw2.image(d2);
      auxDom = auxDom.cap(d1);
      auxDom = pw2.preImage(auxDom);
      newDom = auxDom.cap(d2);

      if (!newDom.empty()) {
        LM_IMP newLM((*itlm1).compose(*itlm2));

        itress = ress.insert(itress, newDom);
        ++itress;
        itreslm = reslm.insert(itreslm, newLM);
        ++itreslm;
      }

      ++itlm2;
    }

    ++itlm1;
  }

  return PWLMapImp1(ress, reslm);
}

// Minimum inverse of a compact PWLMap. Is minimum, because PWLMaps aren't always bijective
PW_TEMPLATE
PW_TEMP_TYPE PW_TEMP_TYPE::minInvCompact(SET_IMP s)
{
  Sets domRes;
  LMaps lmRes;

  if (dom_ref().size() == 1) {
    SET_IMP wDom = wholeDom();
    ORD_CT<INT_IMP> minElem = wDom.minElem();
    typename ORD_CT<INT_IMP>::iterator itmin = minElem.begin();
    SET_IMP im = image(wDom);
    SET_IMP domInv = im.cap(s);

    if (domInv.empty()) {
      PWLMapImp1 res;
      return res;
    }

    domRes.insert(domRes.begin(), domInv);

    LM_IMP lm = *(lmap_ref().begin());
    LM_IMP lmInv = lm.invLMap();
    ORD_CT<REAL_IMP> g = lmInv.gain();
    ORD_CT<REAL_IMP> o = lmInv.offset();
    typename ORD_CT<REAL_IMP>::iterator ito = o.begin();
    ORD_CT<REAL_IMP> gres;
    typename ORD_CT<REAL_IMP>::iterator itgres = gres.begin();
    ORD_CT<REAL_IMP> ores;
    typename ORD_CT<REAL_IMP>::iterator itores = ores.begin();

    BOOST_FOREACH (REAL_IMP gi, g) {
      if (gi == Inf) {
        itgres = gres.insert(itgres, 0);
        itores = ores.insert(itores, *itmin);
      }

      else {
        itgres = gres.insert(itgres, gi);
        itores = ores.insert(itores, *ito);
      }

      ++ito;
      ++itgres;
      ++itores;
      ++itmin;
    }

    LM_IMP aux(gres, ores);
    lmRes.insert(lmRes.begin(), aux);
  }

  PWLMapImp1 res(domRes, lmRes);
  return res;
}

// Minimum inverse of PWLMap. Is minimum because PWLMaps aren't always bijective
PW_TEMPLATE
PW_TEMP_TYPE PW_TEMP_TYPE::minInv(SET_IMP s)
{
  if (!empty()) {
    // Initialization
    Sets auxdom = dom();
    SetsIt itdom = auxdom.begin();
    LMaps auxlm = lmap();
    LMapsIt itlm = auxlm.begin();

    SET_IMP dom1 = *itdom;
    ++itdom;
    LM_IMP lm1 = *itlm;
    ++itlm;

    Sets pw2dom;
    pw2dom.insert(pw2dom.begin(), dom1);
    LMaps pw2lm;
    pw2lm.insert(pw2lm.begin(), lm1);
    PWLMapImp1 pw2(pw2dom, pw2lm);

    PWLMapImp1 invpw2 = pw2.minInvCompact(s);
    pw2 = invpw2;

    for (; itdom != auxdom.end(); ++itdom) {
      Sets pwidom;
      pwidom.insert(pwidom.begin(), *itdom);
      LMaps pwilm;
      pwilm.insert(pwilm.begin(), *itlm);
      PWLMapImp1 pwi(pwidom, pwilm);

      PWLMapImp1 invpwi = pwi.minInvCompact(s);

      PWLMapImp1 minmap = pw2.minMap(invpwi);
      PWLMapImp1 combmap = invpwi.combine(pw2);
      pw2 = combmap;

      if (!minmap.empty()) {
        PWLMapImp1 combmap2 = minmap.combine(pw2);
        pw2 = combmap2;
      }

      ++itlm;
    }

    return pw2;
  }

  PWLMapImp1 res;
  return res;
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
PW_TEMPLATE
bool PW_TEMP_TYPE::equivalentPW(PW_TEMP_TYPE pw2)
{
  SET_IMP dom1 = wholeDom();
  SET_IMP dom2 = pw2.wholeDom();

  if (dom1 == dom2) {
    LMaps lm1 = lmap();
    LMapsIt itlm1 = lm1.begin();

    BOOST_FOREACH (SET_IMP d1, dom()) {
      LMaps lm2 = pw2.lmap();
      LMapsIt itlm2 = lm2.begin();

      BOOST_FOREACH (SET_IMP d2, pw2.dom()) { 
        SET_IMP domcap = d1.cap(d2);

        PWLMapImp1 auxpw1;
        auxpw1.addSetLM(domcap, *itlm1);
        PWLMapImp1 auxpw2;
        auxpw2.addSetLM(domcap, *itlm2);

        SET_IMP im1 = auxpw1.image(domcap);
        SET_IMP im2 = auxpw2.image(domcap);

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

// Restrict doms (and consequently, linear maps) according to argument
PW_TEMPLATE
PW_TEMP_TYPE PW_TEMP_TYPE::restrictMap(SET_IMP newdom)
{
  Sets resdom;
  SetsIt itresdom = resdom.begin();
  LMaps reslm;
  LMapsIt itreslm = reslm.begin();

  SetsIt itdom = dom_ref().begin();
  LMapsIt itlm = lmap_ref().begin();

  for (; itdom != dom_ref().end(); ++itdom) {
    SET_IMP scap = newdom.cap(*itdom);

    if (!scap.empty()) {
      itresdom = resdom.insert(itresdom, scap);
      ++itresdom;
      itreslm = reslm.insert(itreslm, *itlm);
      ++itreslm;
    }

    ++itlm;
  }

  PWLMapImp1 res(resdom, reslm);
  return res;
}

// Append doms and maps of arguments to the current PWLMap
PW_TEMPLATE 
PW_TEMP_TYPE PW_TEMP_TYPE::concat(PW_TEMP_TYPE pw2)
{
  SetsIt itdom = dom_ref().end();
  LMapsIt itlmap = lmap_ref().end();

  BOOST_FOREACH (SET_IMP s, pw2.dom()) {
    itdom = dom_ref().insert(itdom, s);
    ++itdom;
  }

  BOOST_FOREACH (LM_IMP lm, pw2.lmap()) {
    itlmap = lmap_ref().insert(itlmap, lm);
    ++itlmap;
  }

  PWLMapImp1 res(dom(), lmap());
  return res;
}

// Use currents maps for current dom, and use maps of the argument
// for elements that are exclusive of the doms of the argument
PW_TEMPLATE
PW_TEMP_TYPE PW_TEMP_TYPE::combine(PW_TEMP_TYPE pw2)
{
  Sets sres = dom();
  SetsIt its = sres.end();
  LMaps lres = lmap();
  LMapsIt itl = lres.end();

  if (empty())
    return pw2;

  else if (pw2.empty())
    return *this;

  else {
    SET_IMP aux1 = wholeDom();
    Sets dom2 = pw2.dom();
    LMaps lm2 = pw2.lmap();
    LMapsIt itlm2 = lm2.begin();

    BOOST_FOREACH (SET_IMP s2, dom2) {
      SET_IMP newDom = s2.diff(aux1);

      if (!newDom.empty()) {
        its = sres.insert(its, newDom);
        ++its;
        itl = lres.insert(itl, *itlm2);
        ++itl;
      }

      ++itlm2;
    }
  }

  PWLMapImp1 res(sres, lres);
  return res;
}

// Return a PWLMap, with dom partitioned accordingly to express the minimum
// map between lm1 and lm2
PW_TEMPLATE
PW_TEMP_TYPE MIN_MAP_ATOM_SET(AS_IMP &dom, LM_IMP &lm1, LM_IMP &lm2)
{
  ORD_CT<REAL_IMP> g1 = lm1.gain();
  ORD_CT<REAL_IMP> o1 = lm1.offset();
  typename ORD_CT<REAL_IMP>::iterator ito1 = o1.begin();
  ORD_CT<REAL_IMP> g2 = lm2.gain();
  typename ORD_CT<REAL_IMP>::iterator itg2 = g2.begin();
  ORD_CT<REAL_IMP> o2 = lm2.offset();
  typename ORD_CT<REAL_IMP>::iterator ito2 = o2.begin();
  ORD_CT<INTER_IMP> ints = dom.aset_ref().inters();
  typename ORD_CT<INTER_IMP>::iterator itints = ints.begin();

  AS_IMP asAux = dom;
  LM_IMP lmAux = lm1;
  ORD_CT<REAL_IMP> resg = g1;
  ORD_CT<REAL_IMP> reso = o1;
  int count = 1;

  ORD_CT<SET_IMP> domRes;
  ORD_CT<LM_IMP> lmRes;

  if (lm1.ndim() == lm2.ndim()) {
    BOOST_FOREACH (REAL_IMP g1i, g1) {
      if (g1i != *itg2) {
        REAL_IMP xinter = (*ito2 - *ito1) / (g1i - *itg2);

        // Intersection before domain
        if (xinter <= (*itints).lo()) {
          if (*itg2 < g1i) lmAux = lm2;

          SET_IMP sAux;
          sAux.addAtomSet(asAux);

          domRes.insert(domRes.begin(), sAux);
          lmRes.insert(lmRes.begin(), lmAux);
        }

        // Intersection after domain
        else if (xinter >= (*itints).hi()) {
          if (*itg2 > g1i) lmAux = lm2;

          SET_IMP sAux;
          sAux.addAtomSet(asAux);

          domRes.insert(domRes.begin(), sAux);
          lmRes.insert(lmRes.begin(), lmAux);
        }

        // Intersection in domain
        else {
          INTER_IMP i1((*itints).lo(), (*itints).step(), floor(xinter));
          INTER_IMP i2(i1.hi() + i1.step(), (*itints).step(), (*itints).hi());

          AS_IMP as1 = asAux.replace(i1, count);
          AS_IMP as2 = asAux.replace(i2, count);

          SET_IMP d1;
          d1.addAtomSet(as1);

          SET_IMP d2;
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

        PW_TEMP_TYPE auxRes(domRes, lmRes);
        return auxRes;
      }

      else if (*ito1 != *ito2) {
        if (*ito2 < *ito1) lmAux = lm2;

        SET_IMP sAux;
        sAux.addAtomSet(asAux);
        domRes.insert(domRes.begin(), sAux);
        lmRes.insert(lmRes.begin(), lmAux);

        PW_TEMP_TYPE auxRes(domRes, lmRes);
        return auxRes;
      }

      ++ito1;
      ++itg2;
      ++ito2;
      ++itints;
      ++count;
    }
  }

  SET_IMP sAux;
  sAux.addAtomSet(dom);
  domRes.insert(domRes.begin(), sAux);
  lmRes.insert(lmRes.begin(), lm1);
  PW_TEMP_TYPE auxRes(domRes, lmRes);
  return auxRes;
}

// Return a PWLMap, with dom partitioned accordingly to express the minimum
// map between lm1 and lm2
PW_TEMPLATE
PW_TEMP_TYPE MIN_MAP_SET(SET_IMP &dom, LM_IMP &lm1, LM_IMP &lm2)
{
  ORD_CT<Set> sres;
  ORD_CT<LMap> lres;

  SET_IMP sres1;
  SET_IMP sres2;
  LM_IMP lres1;
  LM_IMP lres2;

  UNORD_CT<AS_IMP> asets = dom.asets();
  typename UNORD_CT<AS_IMP>::iterator itas = asets.begin();

  if (!dom.empty()) {
    PW_TEMP_TYPE aux;
    AS_IMP asAux = *itas;
    aux = minMapAtomSet(asAux, lm1, lm2);
    if (!aux.empty()) {
      sres1 = *(aux.dom_ref().begin());
      lres1 = *(aux.lmap_ref().begin());
      ++itas;

      ORD_CT<SET_IMP> d;
      typename ORD_CT<SET_IMP>::iterator itd;
      ORD_CT<LM_IMP> l;
      typename ORD_CT<LM_IMP>::iterator itl;
      while (itas != asets.end()) {
        asAux = *itas;
        aux = minMapAtomSet(asAux, lm1, lm2);
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

  PW_TEMP_TYPE res(sres, lres);
  return res;
}

// Minimum of PWLMaps
PW_TEMPLATE
PW_TEMP_TYPE PW_TEMP_TYPE::minMap(PW_TEMP_TYPE pw2)
{
  PWLMapImp1 res;

  LMaps lm1 = lmap();
  LMapsIt itl1 = lm1.begin();
  LMaps lm2 = pw2.lmap();

  if (!empty() && !pw2.empty()) {
    BOOST_FOREACH (SET_IMP s1i, dom()) {
      LMapsIt itl2 = lm2.begin();

      BOOST_FOREACH (SET_IMP s2j, pw2.dom()) {
        SET_IMP dom = s1i.cap(s2j);

        if (!dom.empty()) {
          PWLMapImp1 aux = minMapSet(dom, *itl1, *itl2);

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

// This operation applies an offset to each linear expression
// of the map.
PW_TEMPLATE
PW_TEMP_TYPE PW_TEMP_TYPE::offsetMap(ORD_CT<INT_IMP> offElem) 
{
  LMaps lmres;
  LMapsIt itlmres = lmres.begin();

  if ((int) offElem.size() == ndim()) {
    typename ORD_CT<INT_IMP>::iterator itelem = offElem.begin();
    BOOST_FOREACH (LM_IMP lmi, lmap()) {
      ORD_CT<REAL_IMP> reso;
      typename ORD_CT<REAL_IMP>::iterator itreso = reso.begin();

      BOOST_FOREACH (REAL_IMP oi, lmi.offset()) {
        itreso = reso.insert(itreso, oi + (REAL_IMP) *itelem);
        ++itreso;
      }

      itlmres = lmres.insert(itlmres, LM_IMP(lmi.gain(), reso)); 
      ++itlmres;
    }
  } 

  return PWLMapImp1(dom(), lmres);
}

// This operation adds two maps. In the
// intersection of the domains, the linear map will be 
// the substraction of both linear maps
PW_TEMPLATE
PW_TEMP_TYPE PW_TEMP_TYPE::addMap(PW_TEMP_TYPE pw2)
{
  Sets domres;
  SetsIt itdom = domres.begin();
  LMaps lmres;
  LMapsIt itlm = lmres.begin(); 

  if (ndim() == pw2.ndim()) {
    LMapsIt itlm1 = lmap_ref().begin();

    BOOST_FOREACH (SET_IMP d1, dom()) {
      LMapsIt itlm2 = pw2.lmap_ref().begin();
      BOOST_FOREACH (SET_IMP d2, pw2.dom()) {
        SET_IMP domcap = d1.cap(d2);
        LM_IMP add = (*itlm1).addLM(*itlm2);

        if (!domcap.empty()) {
          itdom = domres.insert(itdom, domcap);
          ++itdom;
          itlm = lmres.insert(itlm, add);
          ++itlm;
        }

        ++itlm2;
      }

      ++itlm1;
    }
  }

  return PWLMapImp1(domres, lmres);
}

// This operation substracts one map from another. In the
// intersection of the domains, the linear map will be 
// the substraction of both linear maps
PW_TEMPLATE
PW_TEMP_TYPE PW_TEMP_TYPE::diffMap(PW_TEMP_TYPE pw2)
{
  Sets domres;
  SetsIt itdom = domres.begin();
  LMaps lmres;
  LMapsIt itlm = lmres.begin(); 

  if (ndim() == pw2.ndim()) {
    LMapsIt itlm1 = lmap_ref().begin();

    BOOST_FOREACH (SET_IMP d1, dom()) {
      LMapsIt itlm2 = pw2.lmap_ref().begin();
      BOOST_FOREACH (SET_IMP d2, pw2.dom()) {
        SET_IMP domcap = d1.cap(d2);
        LM_IMP diff = (*itlm1).diffLM(*itlm2);

        if (!domcap.empty()) {
          itdom = domres.insert(itdom, domcap);
          ++itdom;
          itlm = lmres.insert(itlm, diff);
          ++itlm;
        }

        ++itlm2;
      }

      ++itlm1;
    }
  }

  return PWLMapImp1(domres, lmres);
}

// Transform each set in dom in a compact set (with only one atomic set)
PW_TEMPLATE
PW_TEMP_TYPE PW_TEMP_TYPE::atomize()
{
  Sets dres;
  SetsIt itdres = dres.begin();
  LMaps lres;
  LMapsIt itlres = lres.begin();

  LMapsIt itlm = lmap_ref().begin();
  BOOST_FOREACH (SET_IMP d, dom()) {
    BOOST_FOREACH (AS_IMP as, d.asets()) {
      SET_IMP aux;
      aux.addAtomSet(as);

      itdres = dres.insert(itdres, aux);
      ++itdres;
      itlres = lres.insert(itlres, *itlm);
      ++itlres;
    }

    ++itlm;
  }

  PWLMapImp1 res(dres, lres);
  return res;
}

// PWLMap reduction
PW_TEMPLATE
PW_TEMP_TYPE PW_TEMP_TYPE::reduceMapN(int dim)
{
  Sets sres = dom();
  SetsIt itsres = sres.end();
  LMaps lres = lmap();
  LMapsIt itlres = lres.end();

  LMaps lm = lmap();
  LMapsIt itlm = lm.begin();

  unsigned int i = 1;
  BOOST_FOREACH (SET_IMP di, dom()) {
    int count1 = 1;

    ORD_CT<REAL> g = (*itlm).gain();
    typename ORD_CT<REAL>::iterator itg = g.begin();
    ORD_CT<REAL> o = (*itlm).offset();
    typename ORD_CT<REAL>::iterator ito = o.begin();
    // Get the dim-th gain and offset
    while (count1 < dim) {
      ++itg;
      ++ito;
      ++count1;
    }

    if (*itg == 1 && *ito < 0) {
      REAL off = -(*ito);

      BOOST_FOREACH (AS_IMP adom, di.asets()) {
        MI_IMP mi = adom.aset();
        ORD_CT<INTER_IMP> inters = mi.inters();
        typename ORD_CT<INTER_IMP>::iterator itints = inters.begin();

        int count2 = 1;
        while (count2 < dim) {
          ++itints;
          ++count2;
        }

        INT loint = (*itints).lo();
        INT hiint = (*itints).hi();

        if ((hiint - loint) > (off * off)) {
          Sets news;
          SetsIt itnews = news.begin();
          LMaps newl;
          LMapsIt itnewl = newl.begin();

          for (int k = 1; k <= off; k++) {
            ORD_CT<REAL> newo = (*itlm).offset();
            typename ORD_CT<REAL>::iterator itnewo = newo.begin();

            ORD_CT<REAL> resg;
            typename ORD_CT<REAL>::iterator itresg = resg.begin();
            ORD_CT<REAL> reso;
            typename ORD_CT<REAL>::iterator itreso = reso.begin();

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

            LM_IMP newlmap(resg, reso);
            INTER_IMP newinter(loint + k - 1, off, hiint);
            AS_IMP auxas = adom.replace(newinter, dim);
            SET_IMP newset;
            newset.addAtomSet(auxas);

            itnews = news.insert(itnews, newset);
            ++itnews;
            itnewl = newl.insert(itnewl, newlmap);
            ++itnewl;
          }

          PWLMapImp1 newmap(news, newl);

          UNORD_CT<AS_IMP> auxnewd;
          BOOST_FOREACH (AS_IMP auxasi, di.asets()) {
            if (auxasi != adom) auxnewd.insert(auxasi);
          }

          SET_IMP newdomi(auxnewd);

          if (newdomi.empty()) {
            itlres = lres.begin();

            if (i < sres.size()) {
              Sets auxs;
              SetsIt itauxs = auxs.begin();
              LMaps auxl;
              LMapsIt itauxl = auxl.begin();

              unsigned int count4 = 1;
              BOOST_FOREACH (SET_IMP si, sres) {
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
              Sets auxs;
              SetsIt itauxs = auxs.begin();
              LMaps auxl;
              LMapsIt itauxl = auxl.begin();

              unsigned int count4 = 1;
              BOOST_FOREACH (SET_IMP si, sres) {
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
            Sets auxs;
            SetsIt itauxs = auxs.begin();
            SetsIt itauxsres = sres.begin();
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

          BOOST_FOREACH (SET_IMP newi, newmap.dom()) {
            itsres = sres.insert(itsres, newi);
            ++itsres;
          }

          BOOST_FOREACH (LM_IMP newi, newmap.lmap()) {
            itlres = lres.insert(itlres, newi);
            ++itlres;
          }
        }
      }
    }

    ++itlm;
    ++i;
  }

  PWLMapImp1 res(sres, lres);
  return res;
}

// PWLMap infinite composition
PW_TEMPLATE
PW_TEMP_TYPE PW_TEMP_TYPE::mapInf()
{
  PWLMapImp1 res;
  if (!empty()) {
    res = reduceMapN(1);

    for (int i = 2; i <= res.ndim(); ++i) res = reduceMapN(i);

    int maxit = 0;

    Sets doms = res.dom();
    SetsIt itdoms = doms.begin();
    BOOST_FOREACH (LM_IMP lm, res.lmap()) {
      ORD_CT<REAL> o = lm.offset();
      typename ORD_CT<REAL>::iterator ito = o.begin();

      REAL a = 0;
      REAL b = *(lm.gain().begin());

      BOOST_FOREACH (REAL gi, lm.gain()) {
        a = std::max(a, gi * abs(*ito));
        b = std::min(b, gi);

        ++ito;
      }

      ito = o.begin();
      if (a > 0) {
        REAL its = 1;

        ORD_CT<REAL> g = lm.gain();
        typename ORD_CT<REAL>::iterator itg = g.begin();
        // For intervals in which size <= off ^ 2 (check reduceMapN, this intervals are not "reduced")
        for (int dim = 0; dim < res.ndim(); ++dim) {
          if (*itg == 1 && *ito != 0) {
            BOOST_FOREACH (AS_IMP asi, (*itdoms).asets()) {
              MI_IMP mii = asi.aset();
              ORD_CT<INTER_IMP> ii = mii.inters();
              typename ORD_CT<INTER_IMP>::iterator itii = ii.begin();

              for (int count = 0; count < dim; ++count) ++itii;

              its = std::max(its, (REAL_IMP)(ceil(((*itii).hi() - (*itii).lo()) / abs(*ito))));
            }
          }

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

    //else
    //  maxit = floor(log2(maxit)) + 1;

    for (int j = 0; j < maxit; ++j) res = res.compPW(res);
  }

  return res;
}

// pw3 (this PWLMap), pw2, pw1 are such that:
//   pw3 : A -> B, and is a compact map
//   pw2 : A -> C
//   pw1 : C -> D
PW_TEMPLATE
PW_TEMP_TYPE PW_TEMP_TYPE::minAdjCompMap(PW_TEMP_TYPE pw2, PW_TEMP_TYPE pw1)
{
  PWLMap res;

  Sets auxd = dom();
  int auxsize = auxd.size();
  if (auxsize == 1) {
    SET_IMP dom = *(dom_ref().begin());
    LM_IMP lm = *(lmap_ref().begin());

    SET_IMP dominv = image(dom);
    LM_IMP lminv = lm.invLMap();
    ORD_CT<REAL> lminvo = lminv.offset();

    PWLMapImp1 invpw;
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
      if (!empty()) {
        SET_IMP im2 = pw2.image(dom);
        SET_IMP compim12 = pw1.image(im2);

        // Get vertices in image of pw2 with minimum image in pw1
        ORD_CT<INT> mincomp = compim12.minElem();
        MI_IMP micomp;
        BOOST_FOREACH (INT mincompi, mincomp) {
          INTER_IMP i(mincompi, 1, mincompi);
          micomp.addInter(i);
        }
        AS_IMP ascomp(micomp);
        SET_IMP scomp;
        scomp.addAtomSet(ascomp);
        SET_IMP mins2 = pw1.preImage(scomp);

        // Choose minimum in mins2, and assign dom(pw1) this element as image
        ORD_CT<INT> min2 = mins2.minElem();
        typename ORD_CT<INT>::iterator itmin2 = min2.begin();

        ORD_CT<REAL> reso;
        typename ORD_CT<REAL>::iterator itreso = reso.begin();
        ORD_CT<REAL> resg;
        typename ORD_CT<REAL>::iterator itresg = resg.begin();
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
      typename ORD_CT<REAL>::iterator itinvo = lminvo.begin();
      ORD_CT<REAL> newinvg;
      typename ORD_CT<REAL>::iterator itnewinvg = newinvg.begin();
      ORD_CT<REAL> newinvo;
      typename ORD_CT<REAL>::iterator itnewinvo = newinvo.begin();
      ORD_CT<INT> mindom = dom.minElem();
      typename ORD_CT<INT>::iterator itmindom = mindom.begin();

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

      LM_IMP newinvlm(newinvg, newinvo);
      PWLMapImp1 newinvpw;
      newinvpw.addSetLM(dominv, newinvlm);

      // Compose
      PWLMapImp1 auxres = pw2.compPW(newinvpw);

      Sets domaux = auxres.dom();
      LMaps lmaux = auxres.lmap();
      LMapsIt itlmaux = lmaux.begin();

      LMaps lmres;
      LMapsIt itlmres = lmres.begin();

      // Replace values of constant maps with the desired value
      SET_IMP im2 = pw2.image(dom);
      SET_IMP compim12 = pw1.image(im2);

      // Get vertices in image of pw2 with minimum image in pw1
      ORD_CT<INT> mincomp = compim12.minElem();
      MI_IMP micomp;
      BOOST_FOREACH (INT mincompi, mincomp) {
        INTER_IMP i(mincompi, 1, mincompi);
        micomp.addInter(i);
      }
      AS_IMP ascomp(micomp);
      SET_IMP scomp;
      scomp.addAtomSet(ascomp);
      SET_IMP mins2 = pw1.preImage(scomp);

      // Choose minimum in min2
      // Assign dom(pw3) this element as image
      ORD_CT<INT> min2 = mins2.minElem();
      typename ORD_CT<INT>::iterator itmin2 = min2.begin();

      BOOST_FOREACH (Set domauxi, domaux) {
        ORD_CT<REAL> replaceg;
        typename ORD_CT<REAL>::iterator itrepg = replaceg.begin();
        ORD_CT<REAL> replaceo;
        typename ORD_CT<REAL>::iterator itrepo = replaceo.begin();

        ORD_CT<REAL> auxg = (*itlmaux).gain();
        typename ORD_CT<REAL>::iterator itauxg = auxg.begin();
        ORD_CT<REAL> auxo = (*itlmaux).offset();
        typename ORD_CT<REAL>::iterator itauxo = auxo.begin();

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

        itlmres = lmres.insert(itlmres, LM_IMP(replaceg, replaceo));
        LM_IMP den(replaceg, replaceo);

        ++itlmaux;
        ++itlmres;
      }

      PWLMapImp1 replacedpw(auxres.dom(), lmres);
      res = replacedpw;
    }
  }

  return res;
}

// pw2 (this PWLMap), pw1 are such that:
//   pw2 : A -> B, and is a compact map
//   pw1 : A -> C
PW_TEMPLATE
PW_TEMP_TYPE PW_TEMP_TYPE::minAdjCompMap(PW_TEMP_TYPE pw1)
{
  SET_IMP dom1 = pw1.wholeDom();
  SET_IMP im1 = pw1.image(dom1);
  PWLMapImp1 idmap(im1);

  return minAdjCompMap(pw1, idmap);
}

PW_TEMPLATE
PW_TEMP_TYPE PW_TEMP_TYPE::minAdjMap(PW_TEMP_TYPE pw2, PW_TEMP_TYPE pw1)
{
  PWLMapImp1 res;

  if (!empty()) {
    SetsIt itdom = dom_ref().begin();
    LMapsIt itlm = lmap_ref().begin();

    SET_IMP auxdom = *itdom;
    LM_IMP auxlm = *itlm;

    PWLMapImp1 map31;
    map31.addSetLM(auxdom, auxlm);

    res = map31.minAdjCompMap(pw2, pw1);
    ++itdom;
    ++itlm;

    PWLMapImp1 minAdj;
    PWLMapImp1 minM;
    while (itdom != dom_ref().end()) {
      PWLMapImp1 map3i;
      map3i.addSetLM(*itdom, *itlm);
      minAdj = map3i.minAdjCompMap(pw2, pw1);
      minM = res.minMap(minAdj);

      res = minAdj.combine(res);

      if (!minM.empty()) res = minM.combine(res);

      ++itdom;
      ++itlm;
    }
  }


  return res;
}

PW_TEMPLATE
PW_TEMP_TYPE PW_TEMP_TYPE::minAdjMap(PW_TEMP_TYPE pw1)
{
  SET_IMP dom1 = pw1.wholeDom();
  SET_IMP im1 = pw1.image(dom1);
  PWLMapImp1 idmap(im1);

  PWLMap res = minAdjMap(pw1, idmap);
  return res;
}

PW_TEMPLATE
bool PW_TEMP_TYPE::operator==(const PW_TEMP_TYPE &other) const
{
  return dom() == other.dom() && lmap() == other.lmap();
}

template struct PWLMapImp1<OrdCT, UnordCT, AtomPWLMap, LMap, Set, AtomSet, MultiInterval, Interval, INT, REAL>;

PW_TEMPLATE
std::ostream &operator<<(std::ostream &out, const PW_TEMP_TYPE &pw)
{
  ORD_CT<SET_IMP> d = pw.dom();
  typename ORD_CT<SET_IMP>::iterator itd = d.begin();
  ORD_CT<LM_IMP> l = pw.lmap();
  typename ORD_CT<LM_IMP>::iterator itl = l.begin();

  if (d.size() == 0) {
    out << "[]";
    return out;
  }

  if (d.size() == 1) {
    out << "[(" << *itd << ", " << *itl << ")]";
    return out;
  }

  out << "[(" << *itd << ", " << *itl << ") ";
  ++itd;
  ++itl;
  while (next(itd, 1) != d.end()) {
    out << ", (" << *itd << ", " << *itl << ")";

    ++itd;
    ++itl;
  }
  out << ", (" << *itd << ", " << *itl << ")]";

  return out;
}

template std::ostream &operator<<(std::ostream &out, const PWLMap &pw);

} // namespace SBG
