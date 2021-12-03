/*****************************************************************************
 
    This file is part of the Set-Based Graph library.

******************************************************************************/

#pragma once

#include <util/graph/sbg/defs.h>
#include <util/graph/sbg/interval.h>
#include <util/graph/sbg/multi_interval.h>
#include <util/graph/sbg/lmap.h>

namespace SBG {

// Piecewise atomic linear maps -----------------------------------------------------------------

#define APW_TEMPLATE                                                               \
  template <template<typename T, typename = std::allocator<T>> class ORD_CT,       \
            typename LM_IMP, typename MI_IMP, typename INTER_IMP,                  \
            typename INT_IMP, typename REAL_IMP>

#define APW_TEMP_TYPE                                                    \
  AtomPWLMapImp1<ORD_CT, LM_IMP, MI_IMP, INTER_IMP, INT_IMP, REAL_IMP>

APW_TEMPLATE
struct AtomPWLMapImp1 {
  member_class(MI_IMP, dom);
  member_class(LM_IMP, lmap);

  AtomPWLMapImp1(); 
  AtomPWLMapImp1(MI_IMP d, LM_IMP l);
  AtomPWLMapImp1(MI_IMP dom, MI_IMP image);

  bool empty();

  MI_IMP image(MI_IMP as);
  MI_IMP image();
  MI_IMP preImage(MI_IMP as);

  eq_class(AtomPWLMapImp1);
};

typedef AtomPWLMapImp1<OrdCT, LMap, MultiInterval, Interval, INT, REAL> AtomPWLMap;

printable_temp(APW_TEMPLATE, APW_TEMP_TYPE);

} // namespace SBG
