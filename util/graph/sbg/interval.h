/*****************************************************************************
 
    This file is part of the Set-Based Graph library.

******************************************************************************/

#pragma once

#include <util/graph/sbg/defs.h>

namespace SBG {

// Intervals --------------------------------------------------------------------------------------

#define INTER_TEMPLATE                                                    \
  template <template <typename Value,                                     \
                      typename Hash = boost::hash<Value>,                 \
                      typename Pred = std::equal_to<Value>,               \
                      typename Alloc = std::allocator<Value>>             \
            class UNORD_CT>

#define INTER_TEMP_TYPE                                        \
  IntervalImp1<UNORD_CT>

INTER_TEMPLATE
struct IntervalImp1 {
  member_class(int, lo);
  member_class(int, step);
  member_class(int, hi);
  member_class(bool, empty);

  IntervalImp1();
  IntervalImp1(bool isEmpty);
  IntervalImp1(int vlo, int vstep, int vhi);

  int gcd(int a, int b);
  int lcm(int a, int b);

  bool isIn(int x);
  int card();
  IntervalImp1 cap(IntervalImp1 i2);
  UNORD_CT<IntervalImp1> diff(IntervalImp1 i2);

  IntervalImp1 offset(int off);

  int minElem();
  int maxElem();

  IntervalImp1 normalize(IntervalImp1 i2);

  eq_class(IntervalImp1);
  neq_class(IntervalImp1);
};

// >>>>> To add new implementation, add:
// struct IntervalImp2 { ... }
//
typedef IntervalImp1<UnordCT> Interval;
size_t hash_value(const Interval &inter);

// >>>>> To change implementation of Interval:
// typedef IntervalImp2 Interval;

printable_temp(INTER_TEMPLATE, INTER_TEMP_TYPE);

typedef OrdCT<Interval> ORD_INTERS;
typedef UnordCT<Interval> UNORD_INTERS;

std::ostream &operator<<(std::ostream &out, const ORD_INTERS &inters);
std::ostream &operator<<(std::ostream &out, const UNORD_INTERS &inters);

} // namespace SBG
