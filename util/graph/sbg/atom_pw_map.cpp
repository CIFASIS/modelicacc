/*****************************************************************************
 
    This file is part of the Set-Based Graph library.

******************************************************************************/

#include <boost/foreach.hpp>

#include <util/graph/sbg/defs.h>
#include <util/graph/sbg/atom_pw_map.h>

namespace SBG {

// Atomic piecewise linear maps -------------------------------------------------------------------

APW_TEMPLATE
APW_TEMP_TYPE::AtomPWLMapImp1() {}

APW_TEMPLATE
APW_TEMP_TYPE::AtomPWLMapImp1(AS_IMP dom, LM_IMP lmap)
{
  AS_IMP emptyAS;
  LM_IMP emptyLM;

  if (dom.ndim() != lmap.ndim()) {
    dom_ = emptyAS;
    lmap_ = emptyLM;
  }

  else {
    ORD_CT<INTER_IMP> inters = dom.aset_ref().inters();
    ORD_CT<REAL_IMP> g = lmap.gain();
    typename ORD_CT<REAL_IMP>::iterator itg = g.begin();
    ORD_CT<REAL_IMP> o = lmap.offset();
    typename ORD_CT<REAL_IMP>::iterator ito = o.begin();
    bool incompatible = false;

    BOOST_FOREACH (INTER_IMP i, inters) {
      REAL_IMP auxLo = i.lo() * (*itg) + (*ito);
      REAL_IMP auxStep = i.step() * (*itg);
      REAL_IMP auxHi = i.hi() * (*itg) + (*ito);

      if (*itg < Inf) {
        if (auxLo != (int)auxLo && i.lo()) 
          incompatible = true;

        if (auxStep != (int)auxStep && i.step()) 
          incompatible = true;

        if (auxHi != (int)auxHi && i.hi()) 
          incompatible = true;

        ++itg;
        ++ito;
      }
    }

    if (incompatible) {
      dom_ = emptyAS;
      lmap_ = emptyLM;
    }

    else {
      dom_ = dom;
      lmap_ = lmap;
    }
  }
}

member_imp_temp(APW_TEMPLATE, APW_TEMP_TYPE, AS_IMP, dom);
member_imp_temp(APW_TEMPLATE, APW_TEMP_TYPE, LM_IMP, lmap);

APW_TEMPLATE
bool APW_TEMP_TYPE::empty() { return dom_ref().empty() && lmap_ref().empty(); }

APW_TEMPLATE
AS_IMP APW_TEMP_TYPE::image(AS_IMP as)
{
  ORD_CT<INTER_IMP> inters = (as.cap(dom())).aset_ref().inters();
  ORD_CT<REAL_IMP> g = lmap_ref().gain();
  typename ORD_CT<REAL_IMP>::iterator itg = g.begin();
  ORD_CT<REAL_IMP> o = lmap_ref().offset();
  typename ORD_CT<REAL_IMP>::iterator ito = o.begin();

  ORD_CT<INTER_IMP> res;
  typename ORD_CT<INTER_IMP>::iterator itres = res.begin();

  if (dom_ref().empty()) {
    AS_IMP aux2;
    return aux2;
  }

  BOOST_FOREACH (INTER_IMP capi, inters) {
    INT_IMP newLo;
    INT_IMP newStep;
    INT_IMP newHi;

    REAL_IMP auxLo = capi.lo() * (*itg) + (*ito);
    REAL_IMP auxStep = capi.step() * (*itg);
    REAL_IMP auxHi = capi.hi() * (*itg) + (*ito);

    if (*itg < Inf) {
      if (auxLo >= Inf)
        newLo = Inf;
      else
        newLo = (INT_IMP)auxLo;

      if (auxStep >= Inf)
        newStep = Inf;
      else
        newStep = (INT_IMP)auxStep;

      if (auxHi >= Inf)
        newHi = Inf;
      else
        newHi = (INT_IMP)auxHi;
    }

    else {
      newLo = 1;
      newStep = 1;
      newHi = Inf;
    }

    INTER_IMP aux1(newLo, newStep, newHi);
    itres = res.insert(itres, aux1);
    ++itres;

    ++itg;
    ++ito;
  }

  MI_IMP aux2(res);
  return AS_IMP(aux2);
}

APW_TEMPLATE
AS_IMP APW_TEMP_TYPE::preImage(AS_IMP as)
{
  AS_IMP fullIm = image(dom());
  AS_IMP actualIm = fullIm.cap(as);
  AtomPWLMapImp1 inv(actualIm, lmap_ref().invLMap());

  AS_IMP aux = inv.image(actualIm);
  return dom_ref().cap(aux);
}

APW_TEMPLATE
bool APW_TEMP_TYPE::operator==(const APW_TEMP_TYPE &other) const
{
  return dom() == other.dom() && lmap() == other.lmap();
}

template struct AtomPWLMapImp1<OrdCT, LMap, AtomSet, MultiInterval, Interval, INT, REAL>;

APW_TEMPLATE
std::ostream &operator<<(std::ostream &out, const APW_TEMP_TYPE &atompw)
{
  AS_IMP d = atompw.dom();
  LM_IMP lm = atompw.lmap();

  out << "(" << d << ", " << lm << ")";
  return out;
}

template std::ostream &operator<<(std::ostream &out, const AtomPWLMap &atompw);

} // namespace SBG
