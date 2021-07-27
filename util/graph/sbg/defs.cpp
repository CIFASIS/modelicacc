/*****************************************************************************
 
    This file is part of the Set-Based Graph library.

******************************************************************************/

#include <iostream>

#include <util/graph/sbg/defs.h>

namespace SBG {

std::ostream &operator<<(std::ostream &out, const ORD_INTS &nums)
{
  out << "[";
  if (nums.size() == 1)
    out << *(nums.begin());

  else if (nums.size() > 1) {
    for (auto it = nums.begin(); std::next(it) != nums.end(); it++) 
      out << *it << ", ";
    out << *(nums.end());
  }
  out << "]";

  return out;
}

std::ostream &operator<<(std::ostream &out, const ORD_REALS &nums)
{
  out << "[";
  if (nums.size() == 1)
    out << *(nums.begin());

  else if (nums.size() > 1) {
    for (auto it = nums.begin(); std::next(it) != nums.end(); it++) 
      out << *it << ", ";
    out << *(nums.end());
  }
  out << "]";

  return out;
}

} // namespace SBG
