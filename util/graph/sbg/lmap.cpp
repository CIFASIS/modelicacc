/*****************************************************************************
 
    This file is part of the Set-Based Graph library.

******************************************************************************/

#include <boost/foreach.hpp>

#include <util/graph/sbg/defs.h>
#include <util/graph/sbg/lmap.h>

namespace SBG {

// Linear maps ------------------------------------------------------------------------------------

#define NUM_TYPE                  \
   typename LM_TEMP_TYPE::OrdNumeric

LM_TEMPLATE
LM_TEMP_TYPE::LMapImp1()
{
  OrdNumeric empty;
  gain_ = empty;
  offset_ = empty;
  ndim_ = 0;
}

LM_TEMPLATE
LM_TEMP_TYPE::LMapImp1(NUM_TYPE g, NUM_TYPE o)
{
  OrdNumeric empty;

  if (g.size() == o.size()) {
    gain_ = g;
    offset_ = o;
    ndim_ = g.size();
  }

  else {
    gain_ = empty;
    offset_ = empty;
    ndim_ = 0;
  }
}

LM_TEMPLATE
LM_TEMP_TYPE::LMapImp1(int dim)
{
  OrdNumeric g;
  OrdNumericIt itg = g.begin();
  OrdNumeric o;
  OrdNumericIt ito = o.begin();

  for (int i = 0; i < dim; i++) {
    itg = g.insert(itg, 1.0);
    ++itg;
    ito = o.insert(ito, 0);
    ++ito;
  }

  gain_ = g;
  offset_ = o;
  ndim_ = dim;
}

member_imp_temp(LM_TEMPLATE, LM_TEMP_TYPE, NUM_TYPE, gain);
member_imp_temp(LM_TEMPLATE, LM_TEMP_TYPE, NUM_TYPE, offset);
member_imp_temp(LM_TEMPLATE, LM_TEMP_TYPE, int, ndim);

LM_TEMPLATE
void LM_TEMP_TYPE::addGO(REAL_IMP g, REAL_IMP o)
{
  gain_.insert(gain_.end(), g);
  offset_.insert(offset_.end(), o);
  ++ndim_;
}

LM_TEMPLATE
bool LM_TEMP_TYPE::empty()
{
  if (gain_ref().empty() && offset_ref().empty()) return true;

  return false;
}

LM_TEMPLATE
LM_TEMP_TYPE LM_TEMP_TYPE::compose(LM_TEMP_TYPE lm2)
{
  OrdNumeric resg;
  OrdNumericIt itresg = resg.begin();
  OrdNumeric reso;
  OrdNumericIt itreso = reso.begin();

  OrdNumericIt ito1 = offset_ref().begin();
  OrdNumericIt itg2 = lm2.gain_ref().begin();
  OrdNumericIt ito2 = lm2.offset_ref().begin();

  if (ndim() == lm2.ndim()) {
    BOOST_FOREACH (REAL_IMP g1i, gain()) {
      itresg = resg.insert(itresg, g1i * (*itg2));
      ++itresg;
      itreso = reso.insert(itreso, (*ito2) * g1i + (*ito1));
      ++itreso;

      ++ito1;
      ++itg2;
      ++ito2;
    }
  }

  else {
    LMapImp1 aux;
    return aux;
  }

  return LMapImp1(resg, reso);
}

LM_TEMPLATE
LM_TEMP_TYPE LM_TEMP_TYPE::invLMap()
{
  OrdNumeric resg;
  OrdNumericIt itresg = resg.begin();
  OrdNumeric reso;
  OrdNumericIt itreso = reso.begin();

  OrdNumericIt ito1 = offset_ref().begin();

  BOOST_FOREACH (REAL_IMP g1i, gain()) {
    if (g1i != 0) {
      itresg = resg.insert(itresg, 1 / g1i);
      ++itresg;

      itreso = reso.insert(itreso, -(*ito1) / g1i);
      ++itreso;
    }

    else {
      itresg = resg.insert(itresg, Inf);
      ++itresg;

      itreso = reso.insert(itreso, -Inf);
      ++itreso;
    }

    ++ito1;
  }

  return LMapImp1(resg, reso);
}

LM_TEMPLATE
bool LM_TEMP_TYPE::operator==(const LM_TEMP_TYPE &other) const
{
  return gain() == other.gain() && offset() == other.offset();
}

template struct LMapImp1<OrdCT, REAL>;

template<typename REAL_IMP>
std::string mapOper(REAL_IMP &cte) { return (cte >= 0) ? "+ " : ""; }

LM_TEMPLATE
std::ostream &operator<<(std::ostream &out, const LM_TEMP_TYPE &lm)
{
  NUM_TYPE g = lm.gain();
  NUM_TYPE::iterator itg = g.begin();
  NUM_TYPE o = lm.offset();
  NUM_TYPE::iterator ito = o.begin();

  if (g.size() == 0) return out;

  if (g.size() == 1) {
    out << "[" << *itg << " * x " << mapOper<REAL_IMP>(*ito) << *ito << "]";
    return out;
  }

  out << "[" << *itg << " * x " << mapOper<REAL_IMP>(*ito) << *ito;
  ++itg;
  ++ito;
  while (next(itg, 1) != g.end()) {
    out << ", " << *itg << " * x " << mapOper<REAL_IMP>(*ito) << *ito;

    ++itg;
    ++ito;
  }
  out << ", " << *itg << " * x " << mapOper<REAL_IMP>(*ito) << *ito << "]";

  return out;
}

template std::ostream &operator<<(std::ostream &out, const LMap &lm);

} // namespace SBG
