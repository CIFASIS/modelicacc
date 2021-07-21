/*****************************************************************************
 
    This file is part of the Set-Based Graph library.

******************************************************************************/

#pragma once

#include <util/graph/sbg/defs.h>
#include <util/graph/sbg/atomic_set.h>

namespace SBG {

// Sets --------------------------------------------------------------------------------------------

#define SET_TEMPLATE                                                           \
  template <template<typename T, typename = std::allocator<T>> class ORD_CT,   \
            template <typename Value,                                          \
                      typename Hash = boost::hash<Value>,                      \
                      typename Pred = std::equal_to<Value>,                    \
                      typename Alloc = std::allocator<Value>>                  \
            class UNORD_CT,                                                    \
            typename AS_IMP, typename INT_IMP>

#define SET_TEMP_TYPE                                        \
  SetImp1<ORD_CT, UNORD_CT, AS_IMP, INT_IMP>

SET_TEMPLATE
struct SetImp1 {
  typedef UNORD_CT<AS_IMP> AtomSets;
  typedef typename AtomSets::iterator AtomSetsIt;

  member_class(AtomSets, asets);
  member_class(int, ndim);

  SetImp1();
  SetImp1(AS_IMP as);
  SetImp1(AtomSets ss);

  void addAtomSet(AS_IMP aset2);
  void addAtomSets(AtomSets sets2);

  bool empty();

  bool isIn(ORD_CT<INT_IMP> elem);
  int card();
  bool subseteq(SetImp1 set2);
  bool subset(SetImp1 set2);
  SetImp1 cap(SetImp1 set2);
  SetImp1 diff(SetImp1 set2);
  SetImp1 cup(SetImp1 set2);

  ORD_CT<INT_IMP> minElem();

  SetImp1 crossProd(SetImp1 set2);

  eq_class(SetImp1);
  neq_class(SetImp1);

  size_t hash();
};

typedef SetImp1<OrdCT, UnordCT, AtomSet, INT> Set;
size_t hash_value(const Set &set);

printable_temp(SET_TEMPLATE, SET_TEMP_TYPE);

} // namespace SBG
