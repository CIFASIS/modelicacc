/*****************************************************************************
 
    This file is part of the Set-Based Graph library.

******************************************************************************/

#include <boost/foreach.hpp>

#include <util/graph/sbg/atom_pw_map.h>

namespace SBG {

// Atomic piecewise linear maps -------------------------------------------------------------------

APW_TEMPLATE
APW_TEMP_TYPE::AtomPWLMapImp1() {}

APW_TEMPLATE
APW_TEMP_TYPE::AtomPWLMapImp1(MI_IMP dom, LM_IMP lmap)
{
  MI_IMP emptyAS;
  LM_IMP emptyLM;

  if (dom.ndim() != lmap.ndim()) {
    dom_ = emptyAS;
    lmap_ = emptyLM;
  }

  else {
    ORD_CT<INTER_IMP> inters = dom.inters();
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

APW_TEMPLATE
APW_TEMP_TYPE::AtomPWLMapImp1(MI_IMP dom, MI_IMP image)
{
  dom_ = dom;
  lmap_ = LM_IMP();

  if (!dom.empty()) {
    typename ORD_CT<INTER_IMP>::iterator itdom = dom.inters_ref().begin(); 
    typename ORD_CT<INTER_IMP>::iterator itim = image.inters_ref().begin(); 

    ORD_CT<REAL_IMP> g;
    typename ORD_CT<REAL_IMP>::iterator itg = g.begin();
    ORD_CT<REAL_IMP> o;
    typename ORD_CT<REAL_IMP>::iterator ito = o.begin();

    while (itdom != dom.inters_ref().end()) {
      if ((*itdom).card() == (*itim).card()) {
        REAL_IMP transformGain = (*itim).step() / (*itdom).step(); 
        REAL_IMP transformOff = (*itim).lo() - transformGain * (*itdom).lo(); 

        itg = g.insert(itg, transformGain);
        ++itg;
        ito = o.insert(ito, transformOff);
        ++ito;
      } 

      else 
        break;
 
      ++itdom; 
      ++itim;
    }

    lmap_ = LM_IMP(g, o);
  }
}

member_imp_temp(APW_TEMPLATE, APW_TEMP_TYPE, MI_IMP, dom);
member_imp_temp(APW_TEMPLATE, APW_TEMP_TYPE, LM_IMP, lmap);

APW_TEMPLATE
bool APW_TEMP_TYPE::empty() { return dom_ref().empty() && lmap_ref().empty(); }

APW_TEMPLATE
MI_IMP APW_TEMP_TYPE::image(MI_IMP as)
{
  ORD_CT<INTER_IMP> inters = (as.cap(dom())).inters();
  ORD_CT<REAL_IMP> g = lmap_ref().gain();
  typename ORD_CT<REAL_IMP>::iterator itg = g.begin();
  ORD_CT<REAL_IMP> o = lmap_ref().offset();
  typename ORD_CT<REAL_IMP>::iterator ito = o.begin();

  ORD_CT<INTER_IMP> res;
  typename ORD_CT<INTER_IMP>::iterator itres = res.begin();

  if (dom_ref().empty()) return MI_IMP();

  BOOST_FOREACH (INTER_IMP capi, inters) {
    INT_IMP newLo;
    INT_IMP newStep;
    INT_IMP newHi;

    REAL_IMP auxLo = capi.lo() * (*itg) + (*ito);
    REAL_IMP auxStep = capi.step() * (*itg);
    REAL_IMP auxHi = capi.hi() * (*itg) + (*ito);
  
    if (auxLo == auxHi)
      auxStep = 1;

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

  return MI_IMP(res);
}

APW_TEMPLATE
MI_IMP APW_TEMP_TYPE::preImage(MI_IMP as)
{
  MI_IMP fullIm = image(dom());
  MI_IMP actualIm = fullIm.cap(as);

  if (!actualIm.empty()) {
    AtomPWLMapImp1 inv(actualIm, lmap_ref().invLMap());

    MI_IMP aux = inv.image(actualIm);
    return dom_ref().cap(aux);
  }

  else
    return MI_IMP();
}

APW_TEMPLATE
bool APW_TEMP_TYPE::operator==(const APW_TEMP_TYPE &other) const
{
  return dom() == other.dom() && lmap() == other.lmap();
}

template struct AtomPWLMapImp1<OrdCT, LMap, MultiInterval, Interval, INT, REAL>;

APW_TEMPLATE
std::ostream &operator<<(std::ostream &out, const APW_TEMP_TYPE &atompw)
{
  out << "(" << atompw.dom() << ", " << atompw.lmap() << ")";
  return out;
}

template std::ostream &operator<<(std::ostream &out, const AtomPWLMap &atompw);

} // namespace SBG
