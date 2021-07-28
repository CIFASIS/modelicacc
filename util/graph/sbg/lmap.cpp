/*****************************************************************************
 
    This file is part of the Set-Based Graph library.

******************************************************************************/

#include <boost/foreach.hpp>

#include <util/graph/sbg/defs.h>
#include <util/graph/sbg/lmap.h>

namespace SBG {

// Linear maps ------------------------------------------------------------------------------------

#define ORD_NUMS_TYPE                  \
   typename LM_TEMP_TYPE::OrdNumeric

LM_TEMPLATE
LM_TEMP_TYPE::LMapImp1() : ndim_(0), gain_(), offset_() {}

LM_TEMPLATE
LM_TEMP_TYPE::LMapImp1(ORD_NUMS_TYPE gain, ORD_NUMS_TYPE offset) : ndim_(0)
{
  OrdNumeric emptyNum;

  if (gain.size() == offset.size()) {
    gain_ = gain;
    offset_ = offset;
    ndim_ = gain.size();
  }

  else {
    gain_ = emptyNum;
    offset_ = emptyNum;
  }
}

LM_TEMPLATE
LM_TEMP_TYPE::LMapImp1(int ndim)
{
  OrdNumeric g;
  OrdNumericIt itg = g.begin();
  OrdNumeric o;
  OrdNumericIt ito = o.begin();

  for (int i = 0; i < ndim; i++) {
    itg = g.insert(itg, 1.0);
    ++itg;
    ito = o.insert(ito, 0);
    ++ito;
  }

  gain_ = g;
  offset_ = o;
  ndim_ = ndim;
}

member_imp_temp(LM_TEMPLATE, LM_TEMP_TYPE, ORD_NUMS_TYPE, gain);
member_imp_temp(LM_TEMPLATE, LM_TEMP_TYPE, ORD_NUMS_TYPE, offset);
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

template std::string mapOper<REAL>(REAL &cte);

LM_TEMPLATE
std::ostream &operator<<(std::ostream &out, const LM_TEMP_TYPE &lm)
{
  ORD_NUMS_TYPE g = lm.gain();
  ORD_NUMS_TYPE::iterator itg = g.begin();
  ORD_NUMS_TYPE o = lm.offset();
  ORD_NUMS_TYPE::iterator ito = o.begin();

  if (g.size() == 0) return out;

  if (g.size() == 1) {
    out << "[" << *itg << " * x " << mapOper(*ito) << *ito << "]";
    return out;
  }

  out << "[" << *itg << " * x " << mapOper(*ito) << *ito;
  ++itg;
  ++ito;
  while (next(itg, 1) != g.end()) {
    out << ", " << *itg << " * x " << mapOper(*ito) << *ito;

    ++itg;
    ++ito;
  }
  out << ", " << *itg << " * x " << mapOper(*ito) << *ito << "]";

  return out;
}

template std::ostream &operator<<(std::ostream &out, const LMap &lm);

} // namespace SBG
