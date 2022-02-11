/*****************************************************************************
 
    This file is part of the Set-Based Graph library.

******************************************************************************/

#include <utility>
#include <omp.h>

#include <boost/foreach.hpp>

#include <util/graph/sbg/pw_map.h>

namespace SBG {

// Piecewise linear maps --------------------------------------------------------------------------

#define SETS_TYPE                  \
   typename PW_TEMP_TYPE::Sets

#define LMAPS_TYPE                 \
   typename PW_TEMP_TYPE::LMaps

PW_TEMPLATE
PW_TEMP_TYPE::PWLMapImp1() : ndim_(0) {}

PW_TEMPLATE
PW_TEMP_TYPE::PWLMapImp1(SETS_TYPE dom, LMAPS_TYPE lmap) : ndim_(0)
{
  LMapsIt itl = lmap.begin();
  int dim1 = (*(dom.begin())).ndim();
  bool different = false;

  if (dom.size() == lmap.size()) {
    BOOST_FOREACH (SET_IMP set, dom) {
      BOOST_FOREACH (MI_IMP as, set.asets()) {
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
  ndim_ = auxpw.ndim();
}

PW_TEMPLATE
void PW_TEMP_TYPE::addLMSet(LM_IMP lm, SET_IMP s)
{
  dom_.insert(dom_ref().begin(), s);
  lmap_.insert(lmap_ref().begin(), lm);
  PWLMapImp1 auxpw(dom_, lmap_);

  dom_ = auxpw.dom();
  lmap_ = auxpw.lmap();
  ndim_ = auxpw.ndim();
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

    BOOST_FOREACH (MI_IMP as, aux1.asets()) {
      APW_IMP auxMap(as, *itl);
      partialRes.addAtomSet(auxMap.image(as));
    }

    res = res.cup(partialRes);

    ++itl;
  }

  return res;
}

PW_TEMPLATE
SET_IMP PW_TEMP_TYPE::image()
{
  return image(wholeDom());
}

PW_TEMPLATE
SET_IMP PW_TEMP_TYPE::preImage(SET_IMP s)
{
  LMapsIt itl = lmap_ref().begin();

  SET_IMP res;

  BOOST_FOREACH (SET_IMP ss, dom()) {
    SET_IMP partialRes;

    BOOST_FOREACH (MI_IMP as1, ss.asets()) {
      APW_IMP auxMap(as1, *itl);

      BOOST_FOREACH (MI_IMP as2, s.asets())
        partialRes.addAtomSet(auxMap.preImage(as2));
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
  PWLMapImp1 aux = *this;
  aux = aux.normalize();
  pw2 = pw2.normalize();

  LMapsIt itlm1 = aux.lmap_ref().begin();
  LMapsIt itlm2 = pw2.lmap_ref().begin();

  Sets ress;
  SetsIt itress = ress.begin();
  LMaps reslm;
  LMapsIt itreslm = reslm.begin();

  SET_IMP auxDom;
  SET_IMP newDom;

  if (aux.equivalentPW(pw2)) {
    PWLMap notId;

    for (SetsIt itdom1 = aux.dom_ref().begin(); itdom1 != aux.dom_ref().end(); ++itdom1) {
      if ((*itlm1).isId()) {
        itress = ress.insert(itress, *itdom1);
        ++itress;
        itreslm = reslm.insert(itreslm, *itlm1);
        ++itreslm;

        SET_IMP pre1 = aux.preImage(*itdom1).diff(*itdom1);

        if (!pre1.empty()) {
          itlm2 = aux.lmap_ref().begin();
          for (SetsIt itdom2 = aux.dom_ref().begin(); itdom2 != aux.dom_ref().end(); ++itdom2) {
            SET_IMP inter = pre1.cap(*itdom2);

            if (!inter.empty()) {
              itress = ress.insert(itress, inter);
              ++itress;
              itreslm = reslm.insert(itreslm, *itlm2);
              ++itreslm;
            }

            ++itlm2;
          }
        }
      }

      else {
        Set dom1;

        BOOST_FOREACH (MI_IMP mi, (*itdom1).asets()) {
          APW_IMP atom(mi, *itlm1);

          if (atom.isId()) {
            SET_IMP atomDom;
            atomDom.addAtomSet(mi);
            itress = ress.insert(itress, atomDom);
            ++itress;
            itreslm = reslm.insert(itreslm, *itlm1);
            ++itreslm;

            SET_IMP pre1 = aux.preImage(atomDom);

            if (!pre1.empty()) {
              itlm2 = aux.lmap_ref().begin();
              for (SetsIt itdom2 = aux.dom_ref().begin(); itdom2 != aux.dom_ref().end(); ++itdom2) {
                SET_IMP inter = pre1.cap(*itdom2);

                if (!inter.empty()) {
                  itress = ress.insert(itress, inter);
                  ++itress;
                  itreslm = reslm.insert(itreslm, *itlm2);
                  ++itreslm;
                }

                ++itlm2;
              }
            }
          }
     
          else 
            dom1.addAtomSet(mi);
        }

        if (!dom1.empty())
          notId.addSetLM(dom1, *itlm1);
      }

      ++itlm1;
    }

    itlm1 = notId.lmap_ref().begin();
    for (SetsIt itdom1 = notId.dom_ref().begin(); itdom1 != notId.dom_ref().end(); ++itdom1) {
      itlm2 = aux.lmap_ref().begin();
      SET_IMP pre1 = preImage(*itdom1);

      if (!pre1.empty()) {
        for (SetsIt itdom2 = aux.dom_ref().begin(); itdom2 != aux.dom_ref().end(); ++itdom2) {
          SET_IMP inter = pre1.cap(*itdom2);

          if (!inter.empty()) {
            LM_IMP newLM((*itlm1).compose(*itlm2));

            itress = ress.insert(itress, inter);
            ++itress;
            itreslm = reslm.insert(itreslm, newLM);
            ++itreslm;
          }

          ++itlm2;
        }
      }

      ++itlm1;
    }
  }

  else {

    itlm1 = aux.lmap_ref().begin();

    BOOST_FOREACH (SET_IMP d1, aux.dom()) {
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
  }

  PWLMapImp1 res(ress, reslm);
  /*
  std::cout << aux << "\n\n";
  std::cout << pw2 << "\n\n";
  std::cout << "res: "<< res << "\n\n";
  */
  return res;
}

PW_TEMPLATE
PW_TEMP_TYPE PW_TEMP_TYPE::compPW(int n)
{
  PWLMapImp1 res = *this;
  PWLMapImp1 original = *this;

  for (int i = 1; i < n; i++)
    res = res.compPW(original);

  return res;
}

// Minimum inverse of a compact PWLMap. Is minimum, because PWLMaps aren't always injective,
// in which case the minimum element of the dom is selected
PW_TEMPLATE
PW_TEMP_TYPE PW_TEMP_TYPE::minInvCompact(SET_IMP s)
{
  Sets domRes;
  LMaps lmRes;

  if (dom_ref().size() == 1) {
    ORD_CT<INT_IMP> min = wholeDom().minElem();
    typename ORD_CT<INT_IMP>::iterator itmin = min.begin();
    SET_IMP domInv = image().cap(s);

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

PW_TEMPLATE
PW_TEMP_TYPE PW_TEMP_TYPE::minInv(SET_IMP s)
{
  if (!empty()) {
    // Initialization
    SetsIt itdom = dom_ref().begin();
    LMapsIt itlm = lmap_ref().begin();

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

    for (; itdom != dom_ref().end(); ++itdom) {
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

        if (!domcap.empty()) {
          PWLMapImp1 auxpw1;
          auxpw1.addSetLM(domcap, *itlm1);
          PWLMapImp1 auxpw2;
          auxpw2.addSetLM(domcap, *itlm2);

          SET_IMP im1 = auxpw1.image(domcap);
          SET_IMP im2 = auxpw2.image(domcap);

          if (im1 != im2)
            return false;
        }

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
  if (!newdom.empty()) {
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

    return PWLMapImp1(resdom, reslm);
  }

  return PWLMapImp1();
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

// Given two maps pw1 (this map) and pw2, return a map pw3
// such that dom(pw3) = dom(pw1) U dom(pw2), and
// pw3(x) = pw1(x) if x in dom(pw1); else pw3(x) = pw2(x)
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

PW_TEMPLATE
PW_TEMP_TYPE PW_TEMP_TYPE::filterMap(bool (*f)(SET_IMP dom, LM_IMP lm))
{
  PWLMapImp1 res;

  SetsIt itdom = dom_ref().begin();
  LMapsIt itlm = lmap_ref().begin();

  for(unsigned int i = 0; i < dom_ref().size(); i++) {
    if (f(*itdom, *itlm))
      res.addSetLM(*itdom, *itlm);

    ++itdom;
    ++itlm;
  }

  return res;
}

// Given a map pw1 (this map) and a bijective map pw2, return a map 
// pw3 such that dom(pw3) = {pw2(x) : x in dom(pw1)}, and given
// y = pw2(x) in dom(pw3), pw3(y) = pw1(x) where x in dom(pw1)
// This operation updates the values of the dom, without changing
// the mapping of the elements
PW_TEMPLATE
PW_TEMP_TYPE PW_TEMP_TYPE::offsetDomMap(PW_TEMP_TYPE pw2)
{
  Sets domRes;
  SetsIt itDomRes = domRes.begin();

  BOOST_FOREACH (Set d, dom()) {
    itDomRes = domRes.insert(itDomRes, pw2.image(d));
    ++itDomRes;
  }

  PWLMapImp1 res(domRes, lmap());
  return res;
}

// Given a map pw1 (this map) and a bijective map pw2, return a map 
// pw3 such that dom(pw3) = dom(pw1), and given
// y = pw2(x) in dom(pw3), pw3(y) = pw1(x) where x in dom(pw1)
// This operation updates the linear maps without changing the doms
PW_TEMPLATE
PW_TEMP_TYPE PW_TEMP_TYPE::offsetImageMap(ORD_CT<INT_IMP> offElem) 
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
// the sum of both linear maps
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
    BOOST_FOREACH (MI_IMP as, d.asets()) {
      SET_IMP aux;
      aux.addAtomSet(as);

      itdres = dres.insert(itdres, aux);
      ++itdres;
      itlres = lres.insert(itlres, *itlm);
      ++itlres;
    }

    ++itlm;
  }

  return PWLMapImp1(dres, lres);
}

PW_TEMPLATE
PW_TEMP_TYPE PW_TEMP_TYPE::normalize()
{
  Sets domres;
  LMaps lmres;

  SetsIt itdom1 = dom_ref().begin(), itdom2, itdomres = domres.begin();
  LMapsIt itlm1 = lmap_ref().begin(), itlm2, itlmres = lmres.begin();

  Set notRepeat;

  while (itdom1 != dom_ref().end()) {
    itdom2 = itdom1;
    ++itdom2;
    itlm2 = itlm1;
    ++itlm2;

    Set newDom;

    if ((*itdom1).cap(notRepeat).empty()) {
      newDom = *itdom1;

      while (itdom2 != dom_ref().end()) {
        if (*itlm1 == *itlm2) 
            newDom = newDom.cup(*itdom2);

        ++itdom2;
        ++itlm2;
      }
    }

    notRepeat = notRepeat.cup(newDom);

    if (!newDom.empty()) {
      itdomres = domres.insert(itdomres, newDom.normalize());
      itlmres = lmres.insert(itlmres, *itlm1);

      ++itdomres;
      ++itlmres;
    }

    ++itdom1;
    ++itlm1;
  }

  return PWLMapImp1(domres, lmres);
}

// Return a PWLMap, with dom partitioned accordingly to return lm1 if
// lm3.compose(lm1) < lm4.compose(lm2)
// If lm3.compose(lm1) == lm4.compose(lm2), choose the minimum between lm1
// and lm2
// Else, return lm2
PW_TEMPLATE
void PW_TEMP_TYPE::minMapAtomSet(MI_IMP dom, LM_IMP lm1, LM_IMP lm2, LM_IMP lm3, LM_IMP lm4)
{
  LM_IMP lm31 = lm3.compose(lm1);
  LM_IMP lm42 = lm4.compose(lm2);

  ORD_CT<REAL_IMP> g31 = lm31.gain();
  ORD_CT<REAL_IMP> o31 = lm31.offset();
  typename ORD_CT<REAL_IMP>::iterator ito31 = o31.begin();
  ORD_CT<REAL_IMP> g42 = lm42.gain();
  ORD_CT<REAL_IMP> o42 = lm42.offset();
  typename ORD_CT<REAL_IMP>::iterator itg42 = g42.begin();
  typename ORD_CT<REAL_IMP>::iterator ito42 = o42.begin();
  ORD_CT<INTER_IMP> ints = dom.inters();
  typename ORD_CT<INTER_IMP>::iterator itints = ints.begin();

  MI_IMP asAux = dom;
  SET_IMP sAux;
  sAux.addAtomSet(asAux);
  LM_IMP lmAux = lm31;
  ORD_CT<REAL_IMP> resg = g31;
  ORD_CT<REAL_IMP> reso = o31;
  int count = 1;

  Sets domRes;
  LMaps lmRes;
  
  // Base case
  if (lm31 == lm42 && lm1 == lm2) {
    domRes.insert(domRes.begin(), sAux);
    lmRes.insert(lmRes.begin(), lm1);
    PWLMapImp1 res(domRes, lmRes);
    set_dom(res.dom());
    set_lmap(res.lmap());
    return;
  }

  // Need to analyze gains and offsets of lm31 and lm42
  if (lm31.ndim() == lm42.ndim() && lm1.ndim() == lm2.ndim()) {
    BOOST_FOREACH (REAL_IMP g31i, g31) {
      lmAux = lm1;

      // Different gains, there's intersection
      if (g31i != *itg42) {
        REAL_IMP xinter = (*ito42 - *ito31) / (g31i - *itg42);

        // Intersection before domain
        if (xinter <= (*itints).lo()) {
          if (*itg42 < g31i) lmAux = lm2;

          domRes.insert(domRes.begin(), sAux);
          lmRes.insert(lmRes.begin(), lmAux);
        }

        // Intersection after domain
        else if (xinter >= (*itints).hi()) {
          if (*itg42 > g31i) lmAux = lm2;

          domRes.insert(domRes.begin(), sAux);
          lmRes.insert(lmRes.begin(), lmAux);
        }

        // Intersection in domain
        else {
          INTER_IMP i1((*itints).lo(), (*itints).step(), floor(xinter));
          INTER_IMP i2(i1.hi() + i1.step(), (*itints).step(), (*itints).hi());

          MI_IMP as1 = asAux.replace(i1, count);
          MI_IMP as2 = asAux.replace(i2, count);

          SET_IMP d1;
          d1.addAtomSet(as1);
          SET_IMP d2;
          d2.addAtomSet(as2);

          domRes.insert(domRes.end(), d1);
          domRes.insert(domRes.end(), d2);

          if (g31i > *itg42) {
            lmRes.insert(lmRes.end(), lm1);
            lmRes.insert(lmRes.end(), lm2);
          }

          else {
            lmRes.insert(lmRes.end(), lm2);
            lmRes.insert(lmRes.end(), lm1);
          }
        }

        PWLMapImp1 res(domRes, lmRes);
        set_dom(res.dom());
        set_lmap(res.lmap());
        return;
      }

      // Same gain and different offset, no intersection
      else if (*ito31 != *ito42) {
        if (*ito42 < *ito31) lmAux = lm2;

        domRes.insert(domRes.begin(), sAux);
        lmRes.insert(lmRes.begin(), lmAux);

        PWLMapImp1 res(domRes, lmRes);
        set_dom(res.dom());
        set_lmap(res.lmap());
        return;
      }

      ++ito31;
      ++itg42;
      ++ito42;
      ++itints;
      ++count;
    }
  }

  // Same gain and offset, get the minimum: lm1 or lm2
  LM_IMP id(lm1.ndim());
  minMapAtomSet(dom, lm1, lm2, id, id);
  return;
}

PW_TEMPLATE
void PW_TEMP_TYPE::minMapAtomSet(MI_IMP dom, LM_IMP lm1, LM_IMP lm2)
{
  PWLMapImp1 res;

  if (lm1.ndim() == lm2.ndim()) {
    LM_IMP idlm(lm1.ndim());
    res.minMapAtomSet(dom, lm1, lm2, idlm, idlm); 
  }

  set_dom(res.dom());
  set_lmap(res.lmap());
}

// Return a PWLMap, with dom partitioned accordingly to return lm1 if
// lm3.compose(lm1) < lm4.compose(lm2); if not, lm2 is returned
PW_TEMPLATE
void PW_TEMP_TYPE::minMapSet(SET_IMP dom, LM_IMP lm1, LM_IMP lm2, LM_IMP lm3, LM_IMP lm4)
{
  ORD_CT<Set> sres;
  ORD_CT<LMap> lres;

  SET_IMP sres1;
  SET_IMP sres2;
  LM_IMP lres1;
  LM_IMP lres2;

  UNORD_CT<MI_IMP> asets = dom.asets();
  typename UNORD_CT<MI_IMP>::iterator itas = asets.begin();

  if (!dom.empty()) {
    PWLMapImp1 aux;
    MI_IMP asAux = *itas;
    aux.minMapAtomSet(asAux, lm1, lm2, lm3, lm4);
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
        aux.minMapAtomSet(asAux, lm1, lm2, lm3, lm4);
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

  PWLMapImp1 res(sres, lres);
  set_dom(res.dom());
  set_lmap(res.lmap());
}

PW_TEMPLATE
void PW_TEMP_TYPE::minMapSet(SET_IMP dom, LM_IMP lm1, LM_IMP lm2, PWLMapImp1 pw3)
{
  PWLMapImp1 res;

  Sets doms;
  doms.insert(doms.begin(), dom);
  LMaps lms1;
  lms1.insert(lms1.begin(), lm1);
  LMaps lms2;
  lms2.insert(lms2.begin(), lm2);
  PWLMapImp1 pw1(doms, lms1);
  PWLMapImp1 pw2(doms, lms2);

  LMapsIt itlm31 = pw3.lmap_ref().begin();
  LMapsIt itlm32 = pw3.lmap_ref().begin();

  SET_IMP im1 = pw1.image(dom);
  SET_IMP im2 = pw2.image(dom);

  BOOST_FOREACH (SET_IMP d1, pw3.dom()) {
    itlm32 = pw3.lmap_ref().begin();

    d1 = d1.cap(im1);
    if (!d1.empty()) {
      SET_IMP pre1 = pw1.preImage(d1);

      BOOST_FOREACH (SET_IMP d2, pw3.dom()) {
        d2 = d2.cap(im2);
        if (!d2.empty()) { 
          SET_IMP pre2 = pw2.preImage(d2);

          SET_IMP d = pre1.cap(pre2).cap(dom);
          if (!d.empty()) {
            minMapSet(d, lm1, lm2, *itlm31, *itlm32); 
            res = (*this).combine(res);
          }
        }

        ++itlm32;
      }
    }

    ++itlm31;
  }

  set_dom(res.dom());
  set_lmap(res.lmap());
}

PW_TEMPLATE
void PW_TEMP_TYPE::minMapSet(SET_IMP dom, LM_IMP lm1, LM_IMP lm2)
{
  PWLMapImp1 res;

  if (lm1.ndim() == lm2.ndim()) {
    LM_IMP idlm(lm1.ndim());
    res.minMapSet(dom, lm1, lm2, idlm, idlm); 
  }

  set_dom(res.dom());
  set_lmap(res.lmap());
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
          PWLMapImp1 aux;
          aux.minMapSet(dom, *itl1, *itl2);

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

PW_TEMPLATE
PW_TEMP_TYPE PW_TEMP_TYPE::minMap(PW_TEMP_TYPE pw2, PW_TEMP_TYPE pw1)
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
          PWLMapImp1 aux;
          aux.minMapSet(dom, *itl1, *itl2, pw1);

          if (res.empty()) { 
            res.set_dom(aux.dom());
            res.set_lmap(aux.lmap());
          }

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
        MI_IMP ascomp(micomp);
        SET_IMP scomp;
        scomp.addAtomSet(ascomp);
        SET_IMP mins2 = pw1.preImage(scomp);
        mins2 = mins2.cap(im2);

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
      MI_IMP ascomp(micomp);
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
      minM = res.minMap(minAdj, pw1);
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

// PWLMap reduction
PW_TEMPLATE
PW_TEMP_TYPE PW_TEMP_TYPE::reduceMapN(int dim)
{
  PWLMap res;

  LMaps lm = lmap();
  LMapsIt itlm = lm.begin();

  unsigned int i = 1;
  // Traverse dom. Reduce all possible intervals
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

    if (*itg == 1 && *ito != 0) {
      REAL off = std::abs(*ito);

      // Traverse dom
      BOOST_FOREACH (MI_IMP mi, di.asets()) {
        ORD_CT<INTER_IMP> inters = mi.inters();
        typename ORD_CT<INTER_IMP>::iterator itints = inters.begin();

        int count2 = 1;
        // Get the dim-th interval
        while (count2 < dim) {
          ++itints;
          ++count2;
        }

        INT loint = (*itints).lo();
        INT stint = (*itints).step();
        INT hiint = (*itints).hi();

        // Map's image is in adom, reduction is plausible
        if (INT_IMP(off) % stint == 0) {
        // Is convenient to partition the interval?
          if (((hiint - loint) / stint) > (off * off)) {
            Sets news;
            SetsIt itnews = news.begin();
            LMaps newl;
            LMapsIt itnewl = newl.begin();

            // Partition of the interval
            for (int k = 1; k <= off; k++) {
              INTER_IMP newinter(loint + k - 1, off, hiint);
              MI_IMP auxas = mi.replace(newinter, dim);
              SET_IMP newset;
              newset.addAtomSet(auxas);

              REAL_IMP newoff = newinter.lo() + *ito;
              if (*ito > 0)
                newoff = newinter.hi() + *ito;

              LM_IMP newlmap = (*itlm).replace(0, newoff, dim);

              itnews = news.insert(itnews, newset);
              ++itnews;
              itnewl = newl.insert(itnewl, newlmap);
              ++itnewl;
            }

            PWLMapImp1 newmap(news, newl);
            if (!newmap.empty())
              res = res.concat(newmap);
          }
        }
      }
    }

    ++itlm;
    ++i;
  }

  // Add intervals that weren't reduced
  SET_IMP reducedDom = res.wholeDom();
  LMapsIt itlmNR = lmap_ref().begin();
  BOOST_FOREACH (SET_IMP di, dom()) {
    SET_IMP notReduced = di.diff(reducedDom);

    if (!notReduced.empty()) {
      PWLMapImp1 aux;
      aux.addSetLM(notReduced, *itlmNR);
      res.concat(aux);
    }

    ++itlmNR;
  }

  res.set_ndim(ndim());
  return res;
}

// PWLMap infinite composition
PW_TEMPLATE
PW_TEMP_TYPE PW_TEMP_TYPE::mapInf(int n)
{
  PWLMapImp1 res = *this;

  if (!empty()) {
    int i = 1;
    PWLMap originalRes = res, oldRes;
    while (!oldRes.equivalentPW(res) && i < n) {
      oldRes = res;
      res = res.compPW(originalRes);
      i++;
    }

    if (oldRes.equivalentPW(res))
      return res;

    else {
      int den = 0;
      for (int j = 1; j <= res.ndim(); ++j) res = res.reduceMapN(j);
      
      std::cout << "\n";
      do {
        oldRes = res;
        res = res.compPW(res);

        for (int j = 1; j <= res.ndim(); ++j) res = res.reduceMapN(j);
        den++;
      }
      while (!oldRes.equivalentPW(res));
      std::cout << "den: " << den << "\n\n";
    }
  }

  return res;
}

PW_TEMPLATE
bool PW_TEMP_TYPE::operator==(const PW_TEMP_TYPE &other) const
{
  return dom() == other.dom() && lmap() == other.lmap();
}

template struct PWLMapImp1<OrdCT, UnordCT, AtomPWLMap, LMap, Set, MultiInterval, Interval, INT, REAL>;

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
