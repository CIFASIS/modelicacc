/*****************************************************************************
 
    This file is part of the Set-Based Graph library.

******************************************************************************/

#pragma once

#include <util/graph/sbg/defs.h>
#include <util/graph/sbg/interval.h>
#include <util/graph/sbg/multi_interval.h>

namespace SBG {

// Atomic sets ------------------------------------------------------------------------------------

#define AS_TEMPLATE                                                            \
  template <template<typename T, typename = std::allocator<T>> class ORD_CT,   \
            template <typename Value,                                          \
                      typename Hash = boost::hash<Value>,                      \
                      typename Pred = std::equal_to<Value>,                    \
                      typename Alloc = std::allocator<Value>>                  \
            class UNORD_CT,                                                    \
            typename MI_IMP, typename INTER_IMP, typename INT_IMP>

#define AS_TEMP_TYPE                                        \
  AtomSetImp1<ORD_CT, UNORD_CT, MI_IMP, INTER_IMP, INT_IMP>

AS_TEMPLATE
struct AtomSetImp1 {
  member_class(MI_IMP, aset);
  member_class(int, ndim);

  AtomSetImp1();
  AtomSetImp1(MI_IMP as);

  bool empty();

  bool isIn(ORD_CT<INT_IMP> elem);
  int card();
  AtomSetImp1 cap(AtomSetImp1 aset2);
  UNORD_CT<AtomSetImp1> diff(AtomSetImp1 aset2);

  ORD_CT<INT_IMP> minElem();

  AtomSetImp1 crossProd(AtomSetImp1 aset2);

  AtomSetImp1 replace(INTER_IMP i, int dim);

  eq_class(AtomSetImp1);
  neq_class(AtomSetImp1);

  size_t hash();
};

typedef AtomSetImp1<OrdCT, UnordCT, MultiInterval, Interval, INT> AtomSet;
size_t hash_value(const AtomSet &as);

printable_temp(AS_TEMPLATE, AS_TEMP_TYPE);

typedef UnordCT<AtomSet> UNORD_AS;

std::ostream &operator<<(std::ostream &out, const UNORD_AS &ass);

} // namespace SBG
