/*****************************************************************************
 
    This file is part of the Set-Based Graph library.

******************************************************************************/

#include <iostream>

#include <boost/foreach.hpp>

#include <util/graph/sbg/set.h>

namespace SBG {

// Sets -------------------------------------------------------------------------------------------

#define AS_TYPE                  \
   typename SET_TEMP_TYPE::AtomSets

SET_TEMPLATE
SET_TEMP_TYPE::SetImp1() : ndim_(0), asets_() {}

SET_TEMPLATE
SET_TEMP_TYPE::SetImp1(MI_IMP as) : ndim_(as.ndim()) { asets_ref().insert(as); }

SET_TEMPLATE
SET_TEMP_TYPE::SetImp1(AtomSets asets)
{
  if (!asets.empty()) {
    int dim1 = (*(asets.begin())).ndim();
    bool equalDims = true;
    // Check if all atomic sets have the same dimension
    BOOST_FOREACH (MI_IMP as, asets) 
      if (dim1 != as.ndim()) equalDims = false;

    if (equalDims && dim1 != 0) {
      asets_ = asets;
      ndim_ = dim1;
    }

    else {
      AtomSets emptyASets;
      asets_ = emptyASets;
      ndim_ = 0;
    }
  }

  else {
    asets_ = asets;
    ndim_ = 0;
  }
}

member_imp_temp(SET_TEMPLATE, SET_TEMP_TYPE, AS_TYPE, asets);
member_imp_temp(SET_TEMPLATE, SET_TEMP_TYPE, int, ndim);

SET_TEMPLATE
void SET_TEMP_TYPE::addAtomSet(MI_IMP aset2)
{
  if (!aset2.empty() && aset2.ndim() == ndim() && !empty())
    asets_.insert(aset2);

  else if (!aset2.empty() && empty()) {
    asets_.insert(aset2);
    ndim_ = aset2.ndim();
  }
}

SET_TEMPLATE
void SET_TEMP_TYPE::addAtomSets(AS_TYPE sets2)
{
  BOOST_FOREACH (MI_IMP as, sets2)
    addAtomSet(as);
}

SET_TEMPLATE
bool SET_TEMP_TYPE::empty()
{
  if (asets_ref().empty()) return true;

  bool res = true;
  BOOST_FOREACH (MI_IMP as, asets()) {
    if (!as.empty()) res = false;
  }

  return res;
}

SET_TEMPLATE
bool SET_TEMP_TYPE::isIn(ORD_CT<INT_IMP> elem)
{
  BOOST_FOREACH (MI_IMP as, asets()) 
    if (as.isIn(elem)) return true;

  return false;
}

SET_TEMPLATE
int SET_TEMP_TYPE::card()
{
  int res = 0;

  BOOST_FOREACH (MI_IMP as, asets())
    res += as.card();

  return res;
}

SET_TEMPLATE 
bool SET_TEMP_TYPE::subseteq(SET_TEMP_TYPE set2)
{
  SetImp1 sdiff = (*this).diff(set2);

  if (sdiff.empty()) return true;

  return false;
}

SET_TEMPLATE
bool SET_TEMP_TYPE::subset(SET_TEMP_TYPE set2)
{
  SetImp1 sdiff1 = (*this).diff(set2);
  SetImp1 sdiff2 = set2.diff(*this);

  if (sdiff1.empty() && !sdiff2.empty()) return true;

  return false;
}

SET_TEMPLATE
SET_TEMP_TYPE SET_TEMP_TYPE::cap(SET_TEMP_TYPE set2)
{
  MI_IMP aux1, aux2;

  if (empty() || set2.empty()) {
    SetImp1 emptyRes;
    return emptyRes;
  }

  AtomSets res;

  BOOST_FOREACH (MI_IMP as1, asets()) {
    BOOST_FOREACH (MI_IMP as2, set2.asets()) {
      MI_IMP capres = as1.cap(as2);

      if (!capres.empty()) res.insert(capres);
    }
  }

  return SetImp1(res);
}

SET_TEMPLATE
SET_TEMP_TYPE SET_TEMP_TYPE::diff(SET_TEMP_TYPE set2)
{
  SetImp1 res;
  AtomSets capres = cap(set2).asets();

  if (!capres.empty()) {
    BOOST_FOREACH (MI_IMP as1, asets()) {
      AtomSets aux;
      aux.insert(as1);

      BOOST_FOREACH (MI_IMP as2, capres) {
        SetImp1 newSets;

        BOOST_FOREACH (MI_IMP as3, aux) {
          AtomSets diffres = as3.diff(as2);
          newSets.addAtomSets(diffres);
        }

        aux = newSets.asets();
      }

      res.addAtomSets(aux);
    }
  }

  else
    res.addAtomSets(asets());

  return res;
}

SET_TEMPLATE
SET_TEMP_TYPE SET_TEMP_TYPE::cup(SET_TEMP_TYPE set2)
{
  SetImp1 res = *this;
  SetImp1 aux = set2.diff(*this);

  if (!aux.empty()) res.addAtomSets(aux.asets());

  return res;
}

SET_TEMPLATE
ORD_CT<INT_IMP> SET_TEMP_TYPE::minElem()
{
  ORD_CT<INT_IMP> res;

  if (empty()) return res;

  MI_IMP min = *(asets_ref().begin());

  BOOST_FOREACH (MI_IMP as1, asets()) 
    if (as1.minElem() < min.minElem()) min = as1;

  return min.minElem();
} 

SET_TEMPLATE
ORD_CT<INT_IMP> SET_TEMP_TYPE::maxElem()
{
  ORD_CT<INT_IMP> res;

  if (empty()) return res;

  MI_IMP max = *(asets_ref().begin());

  BOOST_FOREACH (MI_IMP as1, asets()) 
    if (max.maxElem() < as1.maxElem()) max = as1;

  return max.maxElem();
} 

SET_TEMPLATE
SET_TEMP_TYPE SET_TEMP_TYPE::normalize()
{
  UNORD_CT<MI_IMP> res = asets();
  UNORD_CT<MI_IMP> toInsert, toDelete;

  UNORD_CT<MI_IMP> empty;
  
  do {
    bool first = true;
    toInsert = empty;
    toDelete = empty;
 
    SetImp1 aux(res);
    BOOST_FOREACH (MI_IMP as1, res) {
      BOOST_FOREACH (MI_IMP as2, res) {
        MI_IMP normalized = as1.normalize(as2);

        if (!normalized.empty() && first && as1 != as2) {
          toInsert.insert(normalized);
          toDelete.insert(as1);
          toDelete.insert(as2);
 
          first = false;
        }
      }
    }

    BOOST_FOREACH (MI_IMP ins, toInsert) 
      res.insert(ins);

    BOOST_FOREACH (MI_IMP del, toDelete) 
      res.erase(del);
  }
  while (!toDelete.empty());

  return SetImp1(res);
}

SET_TEMPLATE
SET_TEMP_TYPE SET_TEMP_TYPE::crossProd(SET_TEMP_TYPE set2)
{
  AtomSets res;

  BOOST_FOREACH (MI_IMP as1, asets()) {
    BOOST_FOREACH (MI_IMP as2, set2.asets()) {
      MI_IMP auxres = as1.crossProd(as2);
      res.insert(auxres);
    }
  }

  return SetImp1(res);
}

SET_TEMPLATE
bool SET_TEMP_TYPE::operator==(const SET_TEMP_TYPE &other) const
{
  SetImp1 aux1 = *this;
  SetImp1 aux2 = other;
  SetImp1 diff1 = aux1.diff(aux2);
  SetImp1 diff2 = aux2.diff(aux1);

  if (diff1.empty() && diff2.empty())
    return true;

  return false; 
}

SET_TEMPLATE
bool SET_TEMP_TYPE::operator!=(const SET_TEMP_TYPE &other) const
{
  return !(*this == other); 
}

SET_TEMPLATE
bool SET_TEMP_TYPE::operator<(const SET_TEMP_TYPE &other) const 
{
  SetImp1 aux1 = *this;
  SetImp1 aux2 = other;

  if (!aux1.empty() && !aux2.empty()) {
    MI_IMP minAs1 = *(aux1.asets_ref().begin());

    BOOST_FOREACH (MI_IMP as, aux1.asets())
      if (as < minAs1) minAs1 = as;

    MI_IMP minAs2 = *(aux2.asets_ref().begin());

    BOOST_FOREACH (MI_IMP as, aux2.asets())
      if (as < minAs2) minAs2 = as;

    return minAs1 < minAs2;
  }

  return false;
}

SET_TEMPLATE
size_t SET_TEMP_TYPE::hash()
{
  size_t seed = 0;
  boost::hash_combine(seed, asets_ref().size());
  return seed; 
}

template struct SetImp1<OrdCT, UnordCT, MultiInterval, INT>;

SET_TEMPLATE
std::ostream &operator<<(std::ostream &out, const SET_TEMP_TYPE &set)
{
  UNORD_CT<MI_IMP> asets = set.asets();
  typename UNORD_CT<MI_IMP>::iterator it = asets.begin();
  MI_IMP aux;

  if (asets.size() == 0) {
    out << "{}";
    return out;
  }

  if (asets.size() == 1) {
    aux = *it;
    out << "{" << aux << "}";
    return out;
  }

  aux = *it;
  out << "{" << aux;
  ++it;
  while (std::next(it, 1) != asets.end()) {
    aux = *it;
    out << ", " << aux;
    ++it;
  }
  aux = *it;
  out << ", " << aux << "}";

  return out;
}

template std::ostream &operator<<(std::ostream &out, const Set &set);

size_t hash_value(const Set &set) { 
  Set aux = set;
  return aux.hash(); 
}

Set createSet(Interval i)
{
  MultiInterval mi;
  mi.addInter(i);
  Set s;
  s.addAtomSet(mi);

  return s;
}

Set createSet(MultiInterval mi)
{
  Set s;
  s.addAtomSet(mi);

  return s;
}

} // namespace SBG
