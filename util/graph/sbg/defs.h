/*****************************************************************************
 
    This file is part of the Set-Based Graph library.

******************************************************************************/

#pragma once

#include <boost/config.hpp>
#include <boost/unordered_set.hpp>

namespace SBG {

#define Inf std::numeric_limits<int>::max()

typedef int INT;
typedef float REAL;

template <typename T, class = std::allocator<T>>
using OrdCT = std::list<T>; // Ord stands for ordered container

template <typename Value, typename Hash = boost::hash<Value>, typename Pred = std::equal_to<Value>, typename Alloc = std::allocator<Value>>
using UnordCT = boost::unordered_set<Value>; // Unord stands for unordered container

typedef OrdCT<INT> ORD_INTS;
typedef OrdCT<REAL> ORD_REALS;

std::ostream &operator<<(std::ostream &out, const ORD_INTS &nums);
std::ostream &operator<<(std::ostream &out, const ORD_REALS &nums);

// Helpful macros ---------------------------------------------------------------------------------

#define member_class(X, Y) \
  X Y##_;                  \
  X Y() const;             \
  void set_##Y(X x);       \
  X &Y##_ref();

#define member_imp_temp(T, C, X, Y)  \
  T                                  \
  X C::Y() const { return Y##_; }    \
  T                                  \
  void C::set_##Y(X x) { Y##_ = x; } \
  T                                  \
  X &C::Y##_ref() { return Y##_; }

#define eq_class(X) bool operator==(const X &other) const;
#define neq_class(X) bool operator!=(const X &other) const;
#define lt_class(X) bool operator<(const X &other) const;
#define gt_class(X) bool operator>(const X &other) const;

#define printable_temp(T, X)                                    \
  T                                                             \
  std::ostream &operator<<(std::ostream &out, const X &);

} // namespace SBG
