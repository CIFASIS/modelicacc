/*****************************************************************************
 
    This file is part of the Set-Based Graph library.

******************************************************************************/

#pragma once

#include <util/graph/sbg/defs.h>
#include <util/graph/sbg/interval.h>
#include <util/graph/sbg/multi_interval.h>
#include <util/graph/sbg/atomic_set.h>
#include <util/graph/sbg/lmap.h>

namespace SBG {

// Piecewise atomic linear maps -----------------------------------------------------------------

#define APW_TEMPLATE                                                               \
  template <template<typename T, typename = std::allocator<T>> class ORD_CT,       \
            typename LM_IMP, typename AS_IMP, typename MI_IMP, typename INTER_IMP, \
            typename INT_IMP, typename REAL_IMP>

#define APW_TEMP_TYPE                                                    \
  AtomPWLMapImp1<ORD_CT, LM_IMP, AS_IMP, MI_IMP, INTER_IMP, INT_IMP, REAL_IMP>

APW_TEMPLATE
struct AtomPWLMapImp1 {
  member_class(AS_IMP, dom);
  member_class(LM_IMP, lmap);

  AtomPWLMapImp1(); 
  AtomPWLMapImp1(AS_IMP d, LM_IMP l);
  AtomPWLMapImp1(AS_IMP dom, AS_IMP image);

  bool empty();

  AS_IMP image(AS_IMP as);
  AS_IMP preImage(AS_IMP as);

  eq_class(AtomPWLMapImp1);
};

typedef AtomPWLMapImp1<OrdCT, LMap, AtomSet, MultiInterval, Interval, INT, REAL> AtomPWLMap;

printable_temp(APW_TEMPLATE, APW_TEMP_TYPE);

} // namespace SBG
