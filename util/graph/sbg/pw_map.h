/*****************************************************************************
 
    This file is part of the Set-Based Graph library.

******************************************************************************/

#pragma once

#include <iostream>

#include <util/graph/sbg/defs.h>
#include <util/graph/sbg/interval.h>
#include <util/graph/sbg/multi_interval.h>
#include <util/graph/sbg/atomic_set.h>
#include <util/graph/sbg/set.h>
#include <util/graph/sbg/lmap.h>
#include <util/graph/sbg/atom_pw_map.h>

namespace SBG {

// Piecewise linear maps ------------------------------------------------------------------------
// A compact piecewise linear map is a piecewise linear map which has only one Set and one LMap

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
  SET_IMP image();
  SET_IMP preImage(SET_IMP s);
  PWLMapImp1 compPW(PWLMapImp1 pw2);
  PWLMapImp1 compPW(int n);
  PWLMapImp1 minInvCompact(SET_IMP s);
  PWLMapImp1 minInv(SET_IMP s);

  bool equivalentPW(PWLMapImp1 pw2);

  PWLMapImp1 restrictMap(SET_IMP newdom);

  PWLMapImp1 concat(PWLMapImp1 pw2);
  PWLMapImp1 combine(PWLMapImp1 pw2);

  PWLMapImp1 filterMap(bool (*f)(SET_IMP dom, LM_IMP lm));

  PWLMapImp1 offsetDomMap(PWLMapImp1 pw2);
  PWLMapImp1 offsetImageMap(ORD_CT<INT_IMP> off);
  PWLMapImp1 addMap(PWLMapImp1 pw2);
  PWLMapImp1 diffMap(PWLMapImp1 pw2);

  PWLMapImp1 atomize();
  PWLMapImp1 normalize();

  void minMapAtomSet(AS_IMP dom, LM_IMP lm1, LM_IMP lm2, LM_IMP lm3, LM_IMP lm4); 
  void minMapAtomSet(AS_IMP dom, LM_IMP lm1, LM_IMP lm2); 
  void minMapSet(SET_IMP dom, LM_IMP lm1, LM_IMP lm2, LM_IMP lm3, LM_IMP lm4);
  void minMapSet(SET_IMP dom, LM_IMP lm1, LM_IMP lm2, PWLMapImp1 pw3); 
  void minMapSet(SET_IMP dom, LM_IMP lm1, LM_IMP lm2);
  PWLMapImp1 minMap(PWLMapImp1 pw2, PWLMapImp1 pw1); 
  PWLMapImp1 minMap(PWLMapImp1 pw2); 

  PWLMapImp1 minAdjCompMap(PWLMapImp1 pw2, PWLMapImp1 pw1);
  PWLMapImp1 minAdjCompMap(PWLMapImp1 pw1);
  PWLMapImp1 minAdjMap(PWLMapImp1 pw2, PWLMapImp1 pw1);
  PWLMapImp1 minAdjMap(PWLMapImp1 pw1);

  PWLMapImp1 reduceMapN(int dim);
  PWLMapImp1 mapInf();

  eq_class(PWLMapImp1);
};

typedef PWLMapImp1<OrdCT, UnordCT, AtomPWLMap, LMap, Set, AtomSet, MultiInterval, Interval, INT, REAL> PWLMap;

printable_temp(PW_TEMPLATE, PW_TEMP_TYPE);

} // namespace SBG
