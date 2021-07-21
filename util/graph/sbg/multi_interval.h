/*****************************************************************************
 
    This file is part of the Set-Based Graph library.

******************************************************************************/

#pragma once

#include <util/graph/sbg/defs.h>
#include <util/graph/sbg/interval.h>

namespace SBG {

// MultiIntervals ---------------------------------------------------------------------------------

#define MI_TEMPLATE                                                            \
  template <template<typename T, typename = std::allocator<T>> class ORD_CT,   \
            template <typename Value,                                          \
                      typename Hash = boost::hash<Value>,                      \
                      typename Pred = std::equal_to<Value>,                    \
                      typename Alloc = std::allocator<Value>>                  \
            class UNORD_CT,                                                    \
            typename INTER_IMP, typename INT_IMP>

#define MI_TEMP_TYPE                                        \
  MultiInterImp1<ORD_CT, UNORD_CT, INTER_IMP, INT_IMP>

MI_TEMPLATE
struct MultiInterImp1 {
  typedef ORD_CT<INTER_IMP> Intervals;
  typedef typename ORD_CT<INTER_IMP>::iterator IntervalsIt;

  member_class(Intervals, inters);
  member_class(int, ndim);

  MultiInterImp1();
  MultiInterImp1(Intervals is);

  void addInter(INTER_IMP i);
  bool empty();

  bool isIn(ORD_CT<INT_IMP> elem);
  int card();
  MultiInterImp1 cap(MultiInterImp1 mi2);
  UNORD_CT<MultiInterImp1> diff(MultiInterImp1 mi2);

  ORD_CT<INT_IMP> minElem();

  MultiInterImp1 crossProd(MultiInterImp1 mi2);

  MultiInterImp1 replace(INTER_IMP i, int dim);

  eq_class(MultiInterImp1);
  neq_class(MultiInterImp1);
  lt_class(MultiInterImp1);

  size_t hash();
};

typedef MultiInterImp1<OrdCT, UnordCT, Interval, INT> MultiInterval;
size_t hash_value(const MultiInterval &mi);

printable_temp(MI_TEMPLATE, MI_TEMP_TYPE);

typedef UnordCT<MultiInterval> UNORD_MI;

} // namespace SBG
