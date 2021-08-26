/*****************************************************************************
 
    This file is part of the Set-Based Graph library.

******************************************************************************/

#pragma once

#include <iostream>

#include <util/graph/sbg/defs.h>

namespace SBG {

// LinearMaps ---------------------------------------------------------------------------------------

#define LM_TEMPLATE                                                            \
  template <template<typename T, typename = std::allocator<T>> class ORD_CT,   \
            typename REAL_IMP>

#define LM_TEMP_TYPE                                        \
  LMapImp1<ORD_CT, REAL_IMP>

LM_TEMPLATE
struct LMapImp1 {
  typedef ORD_CT<REAL_IMP> OrdNumeric;
  typedef typename OrdNumeric::iterator OrdNumericIt;

  member_class(OrdNumeric, gain);
  member_class(OrdNumeric, offset);
  member_class(int, ndim);

  LMapImp1();
  LMapImp1(OrdNumeric g, OrdNumeric o);
  LMapImp1(int dim); // Constructs the id of LMaps

  void addGO(REAL_IMP g, REAL_IMP o);

  bool empty();

  LMapImp1 compose(LMapImp1 lm2);
  LMapImp1 invLMap();
  LMapImp1 addLM(LMapImp1 lm2);
  LMapImp1 diffLM(LMapImp1 lm2);

  eq_class(LMapImp1);
};

typedef LMapImp1<OrdCT, REAL> LMap;

template<typename REAL_IMP>
std::string mapOper(REAL_IMP &cte);

printable_temp(LM_TEMPLATE, LM_TEMP_TYPE);

} // namespace SBG
