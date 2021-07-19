/*****************************************************************************
 
    This file is part of the Set-Based Graph library.

******************************************************************************/

#include <boost/foreach.hpp>

#include <util/graph/sbg/defs.h>
#include <util/graph/sbg/atomic_set.h>

namespace SBG {

// Atomic sets ------------------------------------------------------------------------------------

AS_TEMPLATE
AS_TEMP_TYPE::AtomSetImp1() 
{
  MI_IMP emptyRes;
  aset_ = emptyRes;
  ndim_ = 0;
}

AS_TEMPLATE
AS_TEMP_TYPE::AtomSetImp1(MI_IMP as)
{
  aset_ = as;
  ndim_ = as.ndim();
}

member_imp_temp(AS_TEMPLATE, AS_TEMP_TYPE, MI_IMP, aset);
member_imp_temp(AS_TEMPLATE, AS_TEMP_TYPE, int, ndim);

AS_TEMPLATE
bool AS_TEMP_TYPE::empty() { return aset_ref().empty(); }

AS_TEMPLATE
bool AS_TEMP_TYPE::isIn(ORD_CT<INT_IMP> elem) { return aset_ref().isIn(elem); }

AS_TEMPLATE
int AS_TEMP_TYPE::card() { return aset_ref().card(); }

AS_TEMPLATE
AS_TEMP_TYPE AS_TEMP_TYPE::cap(AS_TEMP_TYPE aset2) 
{
  AS_TEMP_TYPE res(aset_ref().cap(aset2.aset()));
  return res;
}

AS_TEMPLATE
UNORD_CT<AS_TEMP_TYPE> AS_TEMP_TYPE::diff(AS_TEMP_TYPE aset2)
{
  UNORD_CT<AtomSetImp1> res;

  UNORD_CT<MI_IMP> atomicDiff = aset_ref().diff(aset2.aset());

  if (atomicDiff.empty()) {
    UNORD_CT<AtomSetImp1> emptyRes;
    return emptyRes;
  }

  else {
    BOOST_FOREACH (MI_IMP mi, atomicDiff) 
      res.insert(AtomSetImp1(mi));
  }

  return res;
}

AS_TEMPLATE
ORD_CT<INT_IMP> AS_TEMP_TYPE::minElem() { return aset_ref().minElem(); }

AS_TEMPLATE
AS_TEMP_TYPE AS_TEMP_TYPE::crossProd(AS_TEMP_TYPE aset2) 
{ 
  return AtomSetImp1(aset_ref().crossProd(aset2.aset()));
}

AS_TEMPLATE
AS_TEMP_TYPE AS_TEMP_TYPE::replace(INTER_IMP i, int dim) 
{
  return AtomSetImp1(aset_ref().replace(i, dim));
}

AS_TEMPLATE
bool AS_TEMP_TYPE::operator==(const AS_TEMP_TYPE &other) const { return aset() == other.aset(); } 

AS_TEMPLATE
bool AS_TEMP_TYPE::operator!=(const AS_TEMP_TYPE &other) const { return aset() != other.aset(); } 

AS_TEMPLATE
size_t AS_TEMP_TYPE::hash()
{
  size_t seed = 0;
  boost::hash_combine(seed, aset_ref().hash());
  return seed; 
}

template struct AtomSetImp1<OrdCT, UnordCT, MultiInterval, Interval, INT>;

AS_TEMPLATE
std::ostream &operator<<(std::ostream &out, const AS_TEMP_TYPE &as)
{
  MI_IMP mi = as.aset();

  out << "{" << mi << "}";

  return out;
}

template std::ostream &operator<<(std::ostream &out, const AtomSet &as);

size_t hash_value(const AtomSet &as) { 
  AtomSet aux = as.aset();
  return aux.hash(); 
}

} // namespace SBG
