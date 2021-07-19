/*****************************************************************************
 
    This file is part of the Set-Based Graph library.

******************************************************************************/

#include <boost/foreach.hpp>

#include <util/graph/sbg/defs.h>
#include <util/graph/sbg/pw_map.h>

namespace SBG {

// Piecewise linear maps --------------------------------------------------------------------------

#define SET_TYPE                  \
   typename PW_TEMP_TYPE::Sets

#define LMAP_TYPE                  \
   typename PW_TEMP_TYPE::LMaps

PW_TEMPLATE
PW_TEMP_TYPE::PWLMapImp1() {}

PW_TEMPLATE
PW_TEMP_TYPE::PWLMapImp1(SET_TYPE d, LMAP_TYPE l)
{
  LMapsIt itl = l.begin();
  int auxndim = (*(d.begin())).ndim();
  bool different = false;

  if (d.size() == l.size()) {
    BOOST_FOREACH (SET_IMP sd, d) {
      BOOST_FOREACH (AS_IMP as, sd.asets()) {
        APW_IMP pwatom(as, *itl);

        if (pwatom.empty()) different = true;
      }

      ++itl;
    }

    if (different) {
      // WARNING("Sets and maps should have the same dimension");

      Sets aux1;
      LMaps aux2;
      dom_ = aux1;
      lmap_ = aux2;
      ndim_ = 0;
    }

    else {
      dom_ = d;
      lmap_ = l;
      ndim_ = auxndim;
    }
  }

  else {
    // WARNING("Domain size should be equal to map size");

    Sets aux1;
    LMaps aux2;
    dom_ = aux1;
    lmap_ = aux2;
    ndim_ = 0;
  }
}

PW_TEMPLATE
PW_TEMP_TYPE::PWLMapImp1(SET_IMP s)
{
  Sets d;
  LMaps lm;

  LM_IMP aux(s.ndim());

  d.insert(d.begin(), s);
  lm.insert(lm.begin(), aux);

  dom_ = d;
  lmap_ = lm;
  ndim_ = 1;
}

member_imp_temp(PW_TEMPLATE, PW_TEMP_TYPE, SET_TYPE, dom);
member_imp_temp(PW_TEMPLATE, PW_TEMP_TYPE, LMAP_TYPE, lmap);
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

PW_TEMPLATE
bool PW_TEMP_TYPE::operator==(const PW_TEMP_TYPE &other) const
{
  return dom() == other.dom() && lmap() == other.lmap();
}

template struct PWLMapImp1<OrdCT, UnordCT, AtomPWLMap, LMap, Set, AtomSet, INT, REAL>;

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
