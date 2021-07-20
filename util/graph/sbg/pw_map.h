/*****************************************************************************
 
    This file is part of the Set-Based Graph library.

******************************************************************************/

#pragma once

#include <util/graph/sbg/defs.h>
#include <util/graph/sbg/interval.h>
#include <util/graph/sbg/multi_interval.h>
#include <util/graph/sbg/atomic_set.h>
#include <util/graph/sbg/set.h>
#include <util/graph/sbg/lmap.h>
#include <util/graph/sbg/atom_pw_map.h>

namespace SBG {

// Piecewise linear maps ------------------------------------------------------------------------

#define PW_TEMPLATE                                                          \
  template <template<typename T, typename = std::allocator<T>> class ORD_CT, \
            template <typename Value,                                        \
                      typename Hash = boost::hash<Value>,                    \
                      typename Pred = std::equal_to<Value>,                  \
                      typename Alloc = std::allocator<Value>>                \
            class UNORD_CT,                                                  \
            typename APW_IMP, typename LM_IMP, typename SET_IMP,             \
            typename AS_IMP, typename MI_IMP, typename INTER_IMP,            \
            typename INT_IMP, typename REAL_IMP>

#define PW_TEMP_TYPE                                        \
  PWLMapImp1<ORD_CT, UNORD_CT, APW_IMP, LM_IMP, SET_IMP, AS_IMP, MI_IMP, INTER_IMP, INT_IMP, REAL_IMP>

PW_TEMPLATE
struct PWLMapImp1 {
  typedef ORD_CT<SET_IMP> Sets;
  typedef typename ORD_CT<SET_IMP>::iterator SetsIt;
  typedef ORD_CT<LM_IMP> LMaps;
  typedef typename ORD_CT<LM_IMP>::iterator LMapsIt;

  member_class(Sets, dom);
  member_class(LMaps, lmap);
  member_class(int, ndim);

  PWLMapImp1(); 
  PWLMapImp1(Sets d, LMaps l);
  PWLMapImp1(SET_IMP s); // Dom is s, the map is the id map

  void addSetLM(SET_IMP s, LM_IMP lm);
  void addLMSet(LM_IMP lm, SET_IMP s);

  bool empty();

  SET_IMP wholeDom();
  SET_IMP image(SET_IMP s);
  SET_IMP preImage(SET_IMP s);
  PWLMapImp1 compPW(PWLMapImp1 pw2);
  PWLMapImp1 minInvCompact(SET_IMP s);
  PWLMapImp1 minInv(SET_IMP set);

  bool equivalentPW(PWLMapImp1 pw2);

  PWLMapImp1 restrictMap(SET_IMP newdom);

  PWLMapImp1 concat(PWLMapImp1 pw2);
  PWLMapImp1 combine(PWLMapImp1 pw2);
 
  PWLMapImp1 minMap(PWLMapImp1 pw2); 

  PWLMapImp1 atomize();

  PWLMapImp1 reduceMapN(int dim);
  PWLMapImp1 mapInf();

  PWLMapImp1 minAdjCompMap(PWLMapImp1 pw2, PWLMapImp1 pw1);
  PWLMapImp1 minAdjCompMap(PWLMapImp1 pw1);
  PWLMapImp1 minAdjMap(PWLMapImp1 pw2, PWLMapImp1 pw1);
  PWLMapImp1 minAdjMap(PWLMapImp1 pw1);

  eq_class(PWLMapImp1);
};

typedef PWLMapImp1<OrdCT, UnordCT, AtomPWLMap, LMap, Set, AtomSet, MultiInterval, Interval, INT, REAL> PWLMap;

PWLMap minMapAtomSet(AtomSet dom, LMap lm1, LMap lm2);
PWLMap minMapSet(Set dom, LMap lm1, LMap lm2);

printable_temp(PW_TEMPLATE, PW_TEMP_TYPE);

} // namespace SBG
