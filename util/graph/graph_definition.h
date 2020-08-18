/*****************************************************************************

    This file is part of Modelica C Compiler.

    Modelica C Compiler is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Modelica C Compiler is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Modelica C Compiler.  If not, see <http://www.gnu.org/licenses/>.

******************************************************************************/

/*! \file graph_definition.h
*   General library to work with set based graphs.
*   The main goal of this module was to abstract
*   classes that describe mathematical entities,
*   such as sets, graphs, etc.
*   If implementation has to change, a new concrete
*   class can be defined, but the client modules
*   of graph_definition will call abstract classes,
*   so no change is needed in the rest of the compiler.
*   Through typename template the abstract class uses
*   the new concrete implementation, therefore, requiring
*   little change appart from concrete class implementation.
*/

#ifndef GRAPH_DEFINITION_
#define GRAPH_DEFINITION_

#include <iostream>
#include <list>
#include <map>
#include <math.h>
#include <utility>

#include <boost/config.hpp>
#include <boost/foreach.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/unordered_set.hpp>

#include <util/debug.h>
#include <util/table.h>

using namespace std;

#define Inf numeric_limits<int>::max()

/*-----------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
// Abstract classes
/*-----------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/

template<template<typename Value, typename Hash = boost::hash<Value>, 
                  typename Pred = std::equal_to<Value>, 
                  typename Alloc = std::allocator<Value>> class CT>
struct IntervalImp1{
  int lo;
  int step;
  int hi;
  bool empty;

  int gcd(int a, int b){
    int c;

    do{
      c = a % b;
      if(c > 0){
        a = b;
        b = c;
      }
    } while (c != 0);

    return b;
  }

  int lcm(int a, int b){
    if(a < 0 || b < 0)
      return -1;

    return (a * b) / gcd(a, b);
  }

  IntervalImp1(){};
  IntervalImp1(bool isEmpty){ 
    lo = -1;
    step = -1;
    hi = -1;
    empty = isEmpty;
  };
  IntervalImp1(int vlo, int vstep, int vhi){ 
    if(vlo >= 0 && vstep > 0 && vhi >= 0){
      empty = false;
      lo = vlo;
      step = vstep;

      if(vlo <= vhi && vhi < Inf){
        int rem = std::fmod(vhi - vlo, vstep); 
        hi = vhi - rem; 
      }

      else if(vlo <= vhi && vhi == Inf){
        hi = Inf;
      }

      else{
        //WARNING("Wrong values for subscript (check low <= hi)");
        empty = true;
      }
    }

    else if(vlo >= 0 && vstep == 0 && vhi == vlo){
      empty = false;
      lo = vlo;
      hi = vhi;
      step = 1; 
    }

    else{ 
      //WARNING("Subscripts should be positive");
      lo = -1;
      step = -1;
      hi = -1;
      empty = true;
    }
  }

  int lo_(){
    return lo;
  }

  int step_(){
    return step;
  }

  int hi_(){
    return hi;
  }

  bool empty_() const{
    return empty;
  }

  bool isIn(int x){
    if(x < lo || x > hi || empty)
      return false;

    float aux = fmod(x - lo, step);
    if(aux == 0)
      return true;

    return false;
  } 

  IntervalImp1 cap(IntervalImp1 &inter2){
    int maxLo = max(lo, inter2.lo), newLo = -1;
    int newStep = lcm(step, inter2.step);
    int newEnd = min(hi, inter2.hi);

    if(!empty && !inter2.empty)
      for(int i = 0; i < newStep; i++){
        int res1 = maxLo + i;

        if(isIn(res1) && inter2.isIn(res1)){
          newLo = res1;
          break;
        }
      }

    else
      return IntervalImp1(true);


    if(newLo < 0)
      return IntervalImp1(true);

    return IntervalImp1(newLo, newStep, newEnd);
  }

  CT<IntervalImp1> diff(IntervalImp1 &i2){
    CT<IntervalImp1> res;
    IntervalImp1 capres = cap(i2);

    if(capres.empty){
      res.insert(*this);
      return res;
    }

    if(capres == *this)
      return res;

    // "Before" intersection
    if(lo < capres.lo){
      IntervalImp1 aux = IntervalImp1(lo, 1, capres.lo - 1);
      IntervalImp1 left = cap(aux);
      res.insert(left);
    }

    // "During" intersection
    if(capres.step <= (capres.hi - capres.lo)){
      int nInters = capres.step / step;
      for(int i = 1; i < nInters; i++){
        IntervalImp1 aux = IntervalImp1(capres.lo + i * step, capres.step, capres.hi);
        res.insert(aux);
      }  
    }

    // "After" intersection
    if(hi > capres.hi){
      IntervalImp1 aux = IntervalImp1(capres.hi + 1, 1, hi);
      IntervalImp1 right = cap(aux);
      res.insert(right);
    }
  
    return res;
  }

  int minElem(){
    return lo;
  }
  
  int size(){
    int res = (hi - lo) / step + 1;
    return res;
  }

  bool operator==(const IntervalImp1 &other) const{
    return (lo == other.lo) && (step == other.step) && (hi == other.hi) &&
           (empty == other.empty);
  }

  bool operator!=(const IntervalImp1 &other) const{
    return (lo != other.lo) || (step != other.step) || (hi != other.hi) ||
           (empty != other.empty);
  }

  size_t hash(){
    return lo;
  }
};

template<template<typename Value, typename Hash = boost::hash<Value>, 
                  typename Pred = std::equal_to<Value>, 
                  typename Alloc = std::allocator<Value>> class CT>
size_t hash_value(IntervalImp1<CT> inter){
  return inter.hash();
}

template<template<typename Value, typename Hash = boost::hash<Value>, 
                  typename Pred = std::equal_to<Value>, 
                  typename Alloc = std::allocator<Value>> class CT,
         typename IntervalImp, typename NumImp>
struct IntervalAbs{
  IntervalAbs(){}; 
  IntervalAbs(IntervalImp inter){
    i = inter;
  }
  IntervalAbs(NumImp lo, NumImp step, NumImp hi){
    i = IntervalImp(lo, step, hi);
  }
  IntervalAbs(bool emp){
    i = IntervalImp(emp);
  }
  
  NumImp lo_(){
    return i.lo_();
  }

  NumImp step_(){
    return i.step_();
  }
 
  NumImp hi_(){
    return i.hi_();
  }

  bool empty_(){
    return i.empty_();
  }

  bool isIn(NumImp x){
    return i.isIn(x);
  }

  IntervalAbs cap(IntervalAbs i2){
    return IntervalAbs(i.cap(i2.i));
  }

  CT<IntervalAbs> diff(IntervalAbs i2){
    CT<IntervalAbs> res;
    CT<IntervalImp> diffres = i.diff(i2.i);

    for(IntervalImp inter : diffres){
      IntervalAbs aux(inter);
      res.insert(aux);
    }

    return res;
  }

  NumImp minElem(){
    return i.minElem();
  }

  NumImp size(){
    return i.size();
  }

  bool operator==(const IntervalAbs &other) const{
    return i == other.i;
  }
  
  bool operator!=(const IntervalAbs &other) const{
    return i != other.i;
  }

  size_t hash(){
    return i.lo_(); 
  }
 
  private:
  IntervalImp i; 
};

template<template<typename Value, typename Hash = boost::hash<Value>, 
                  typename Pred = std::equal_to<Value>, 
                  typename Alloc = std::allocator<Value>> class CT,
         typename IntervalImp, typename NumImp>
inline size_t hash_value(IntervalAbs<CT, IntervalImp, NumImp> inter){
  return inter.hash();
}

// MultiIntervals ---------------------------------------------------------------------------------

template<template<typename T, typename = allocator<T>> class CT1,
         template<typename Value, typename Hash = boost::hash<Value>, 
                  typename Pred = std::equal_to<Value>, 
                  typename Alloc = std::allocator<Value>> class CT2,
         typename IntervalImp, typename NumImp>
struct MultiInterImp1{
  typedef typename CT1<IntervalImp>::iterator IntImpIt;

  CT1<IntervalImp> inters;
  int ndim;

  MultiInterImp1(){
    CT1<IntervalImp> emptyRes;
    inters = emptyRes;  
    ndim = 0;
  }
  MultiInterImp1(CT1<IntervalImp> is){
    IntImpIt it = is.begin();
    bool areEmptys = false;

    while(it != is.end()){
      if((*it).empty_())
        areEmptys = true;

      ++it;
    } 
 
    if(areEmptys){
      //WARNING("Empty dimension"); 

      CT1<IntervalImp> aux;
      inters = aux;
      ndim = 0;
    }

    else{
      inters = is;
      ndim = is.size();
    }
  }

  CT1<IntervalImp> inters_(){
    return inters;
  }

  int ndim_(){
    return ndim;
  }

  void addInter(IntervalImp i){
    if(!i.empty_()){
      inters.insert(inters.end(), i);
      ++ndim;
    }
  }

  bool empty(){
    if(inters.empty())
      return true;
  
    return false;
  }

  bool isIn(CT1<NumImp> elem){
    IntImpIt it2 = inters.begin();

    if((int) elem.size() != ndim)
      return false;

    BOOST_FOREACH(NumImp n, elem){
      if(!(*it2).isIn(n))
        return false;      

      ++it2;
    }

    return true;
  }

  MultiInterImp1 cap(MultiInterImp1 &mi2){
    CT1<IntervalImp> res;
    IntImpIt itres = res.begin();

    IntImpIt it2 = mi2.inters.begin();
    if(ndim == mi2.ndim){
      BOOST_FOREACH(IntervalImp i1, inters){
        IntervalImp capres = i1.cap(*it2);
     
        if(capres.empty_()){
          CT1<IntervalImp> aux;
          return MultiInterImp1(aux);
        }

        itres = res.insert(itres, capres);
        ++itres;

        ++it2;    
      }
    }

    return MultiInterImp1(res);
  }

  CT2<MultiInterImp1> diff(MultiInterImp1 &mi2){
    MultiInterImp1 capres = cap(mi2);

    CT2<MultiInterImp1> resmi;
    typename CT2<MultiInterImp1>::iterator itresmi = resmi.begin();

    if(inters.empty())
      return resmi;

    if(ndim != mi2.ndim){
      return resmi;
    }

    if(capres.empty()){
      resmi.insert(*this);
      return resmi;
    }

    if(inters == capres.inters)
      return resmi;

    IntImpIt itcap = capres.inters.begin();
    CT1<CT2<IntervalImp>> diffs;
    typename CT1<CT2<IntervalImp>>::iterator itdiffs = diffs.begin();

    BOOST_FOREACH(IntervalImp i, inters){
      itdiffs = diffs.insert(itdiffs, i.diff(*itcap));
 
      ++itcap;
      ++itdiffs;
    }

    IntImpIt it1 = inters.begin();
    ++it1;
    itdiffs = diffs.begin();
    
    int count = 0;
    BOOST_FOREACH(CT2<IntervalImp> vdiff, diffs){
      BOOST_FOREACH(IntervalImp i, vdiff){
        if(!i.empty_()){
          CT1<IntervalImp> resi;
          IntImpIt itresi = resi.begin();
  
          itcap = capres.inters.begin();

          if(count > 0){
            for(int j = 0; j < count; j++){
              itresi = resi.insert(itresi, *itcap);         
              ++itresi; 

              ++itcap;
            }
          }

          itresi = resi.insert(itresi, i);
          ++itresi; 

          IntImpIt auxit1 = it1;
          while(auxit1 != inters.end()){
            itresi = resi.insert(itresi, *auxit1);
            ++itresi;
        
            ++auxit1;
          }

          itresmi = resmi.insert(itresmi, MultiInterImp1(resi));
          ++itresmi;
        }
      }

      ++count;
      ++it1;
    }

    return resmi;
  }

  MultiInterImp1 crossProd(MultiInterImp1 mi2){
    CT1<IntervalImp> res; 
    IntImpIt itres = res.begin();

    BOOST_FOREACH(IntervalImp i, inters){
      itres = res.insert(itres, i);
      ++itres;
    }

    BOOST_FOREACH(IntervalImp i, mi2.inters){
      itres = res.insert(itres, i);
      ++itres;
    }

    return MultiInterImp1(res);
  }

  CT1<NumImp> minElem(){
    CT1<NumImp> res;
    typename CT1<NumImp>::iterator itres = res.begin();

    BOOST_FOREACH(IntervalImp i, inters){
      if(i.empty_()){
        CT1<NumImp> aux;
        return aux;
      }

      itres = res.insert(itres, i.minElem());
      ++itres;
    }    

    return res;
  }

  MultiInterImp1 replace(IntervalImp &i, int dim){
    CT1<IntervalImp> auxRes;
    IntImpIt itAux = auxRes.begin();
    int count = 1;

    BOOST_FOREACH(IntervalImp ii, inters){
      if(dim == count)
        itAux = auxRes.insert(itAux, i);
      else
        itAux = auxRes.insert(itAux, ii);

      ++itAux;

      ++count;
    }

    return MultiInterImp1(auxRes);
  }

  int size(){
    int res = 1;
    BOOST_FOREACH(IntervalImp i, inters){
      res *= i.size();
    }

    return res; 
  }

  bool operator<(const MultiInterImp1 &other) const{
    typename CT1<IntervalImp>::const_iterator iti2 = other.inters.begin();

    BOOST_FOREACH(IntervalImp i1, inters){
      IntervalImp aux = *iti2; 
      if(i1.minElem() < aux.minElem())
        return true;      

      ++iti2;
    }

    return false;
  }

  bool operator==(const MultiInterImp1 &other) const{
    return inters == other.inters;
  }

  bool operator!=(const MultiInterImp1 &other) const{
    return inters != other.inters;
  }

  size_t hash(){
    return inters.size(); 
  }
};

template<template<typename T, typename = allocator<T>> class CT1,
         template<typename Value, typename Hash = boost::hash<Value>, 
                  typename Pred = std::equal_to<Value>, 
                  typename Alloc = std::allocator<Value>> class CT2,
         typename IntervalImp, typename NumImp>
size_t hash_value(MultiInterImp1<CT1, CT2, IntervalImp, NumImp> mi){
  return mi.hash();
}

template<template<typename T, typename = allocator<T>> class CT1,
         template<typename Value, typename Hash = boost::hash<Value>, 
                  typename Pred = std::equal_to<Value>, 
                  typename Alloc = std::allocator<Value>> class CT2,
          typename MultiInterImp, typename IntervalImp, typename NumImp>
struct MultiInterAbs{
  MultiInterAbs(){
    CT1<IntervalImp> aux;
    multiInterImp = aux;
  }
  MultiInterAbs(MultiInterImp mi){
    multiInterImp = mi;
  }
  MultiInterAbs(CT1<IntervalImp> ints){
    multiInterImp = MultiInterImp(ints);
  }

  CT1<IntervalImp> inters_(){
    return multiInterImp.inters_();
  }

  int ndim_(){
    return multiInterImp.ndim_();
  }

  bool empty(){
    return multiInterImp.empty();
  }

  bool isIn(CT1<NumImp> elem){
    return multiInterImp.isIn(elem);
  }

  void addInter(IntervalImp i){
    multiInterImp.addInter(i);
  }

  MultiInterAbs cap(MultiInterAbs &mi2){
    return MultiInterAbs(multiInterImp.cap(mi2.multiInterImp));
  }

  CT2<MultiInterAbs> diff(MultiInterAbs &mi2){
    CT2<MultiInterImp> diffRes = multiInterImp.diff(mi2.multiInterImp);

    CT2<MultiInterAbs> res;

    BOOST_FOREACH(MultiInterImp mi, diffRes){
      MultiInterAbs aux(mi);
      res.insert(aux);
    }

    return res;
  }

  MultiInterAbs crossProd(MultiInterAbs &mi2){
    return MultiInterAbs(multiInterImp.crossProd(mi2.multiInterImp));
  }

  CT1<NumImp> minElem(){
    return multiInterImp.minElem();
  }

  MultiInterAbs replace(IntervalImp &i, int dim){
    return MultiInterAbs(multiInterImp.replace(i, dim));
  }

  int size(){
    return multiInterImp.size();
  }

  bool operator<(const MultiInterAbs &other) const{
    return multiInterImp < other.multiInterImp;
  }

  bool operator==(const MultiInterAbs &other) const{
    return multiInterImp == other.multiInterImp;
  }

  bool operator!=(const MultiInterAbs &other) const{
    return multiInterImp != other.multiInterImp;
  }

  size_t hash(){
    return multiInterImp.hash();
  }

  private:
  MultiInterImp multiInterImp;
  int ndim;
};

template<template<typename T, typename = allocator<T>> class CT1,
         template<typename Value, typename Hash = boost::hash<Value>, 
                  typename Pred = std::equal_to<Value>, 
                  typename Alloc = std::allocator<Value>> class CT2,
         typename MultiInterImp, typename IntervalImp, typename NumImp>
size_t hash_value(MultiInterAbs<CT1, CT2, MultiInterImp, IntervalImp, NumImp> mi){
  return mi.hash();
}

// Atomic sets ------------------------------------------------------------------------------------

template<template<typename T, typename = allocator<T>> class CT1, 
         template<typename Value, typename Hash = boost::hash<Value>, 
                  typename Pred = std::equal_to<Value>, 
                  typename Alloc = std::allocator<Value>> class CT2,
         typename MultiInterImp, typename IntervalImp, typename NumImp>
struct AtomSetImp1{
  MultiInterImp aset;
  int ndim;

  AtomSetImp1(){
    MultiInterImp emptyRes;
    aset = emptyRes;
    ndim = 0;
  }
  AtomSetImp1(MultiInterImp as){
    aset = as;
    ndim = as.ndim_();
  }

  MultiInterImp aset_(){
    return aset;
  }

  int ndim_(){
    return ndim;
  }

  bool empty(){
    return aset.empty();  
  }

  bool isIn(CT1<NumImp> elem){
    return aset.isIn(elem);
  }

  AtomSetImp1 cap(AtomSetImp1 &aset2){
    AtomSetImp1 aux(aset.cap(aset2.aset));
    return aux;
  }

  CT2<AtomSetImp1> diff(AtomSetImp1 &aset2){
    CT2<AtomSetImp1> res;
    typename CT2<AtomSetImp1>::iterator itres = res.begin();

    CT2<MultiInterImp> atomicDiff = aset.diff(aset2.aset);

    if(atomicDiff.empty()){
      CT2<AtomSetImp1> emptyRes;
      return emptyRes;
    } 

    else{
      BOOST_FOREACH(MultiInterImp mi, atomicDiff){
        itres = res.insert(itres, AtomSetImp1(mi)); 
        ++itres;
      }
    }

    return res;
  }

  AtomSetImp1 crossProd(AtomSetImp1 &aset2){
    return AtomSetImp1(aset.crossProd(aset2.aset));
  }

  CT1<NumImp> minElem(){
    return aset.minElem();
  }

  AtomSetImp1 replace(IntervalImp &i, int dim){
    return AtomSetImp1(aset.replace(i, dim));
  }

  bool operator==(const AtomSetImp1 &other) const{
    return aset == other.aset;
  }

  bool operator!=(const AtomSetImp1 &other) const{
    return aset != other.aset;
  }

  size_t hash(){
    return aset.hash();
  }
};

template<template<typename T, typename = allocator<T>> class CT1, 
         template<typename Value, typename Hash = boost::hash<Value>, 
                  typename Pred = std::equal_to<Value>, 
                  typename Alloc = std::allocator<Value>> class CT2,
         typename MultiInterImp, typename IntervalImp, typename NumImp>
size_t hash_value(AtomSetImp1<CT1, CT2, MultiInterImp, IntervalImp, NumImp> as){
  return as.hash();
}

template<template<typename T, typename = allocator<T>> class CT1,
         template<typename Value, typename Hash = boost::hash<Value>, 
                  typename Pred = std::equal_to<Value>, 
                  typename Alloc = std::allocator<Value>> class CT2,
          typename ASetImp, typename MultiInterImp, typename IntervalImp, typename NumImp>
struct AtomSetAbs{
  AtomSetAbs(){
    ASetImp aux;
    as = aux;
  };
  AtomSetAbs(ASetImp ass){
    as = ass;
  }
  AtomSetAbs(MultiInterImp mi){
    ASetImp aux(mi);
    as = aux;
  }

  MultiInterImp aset_(){
    return as.aset_();
  }

  int ndim_(){
    return as.ndim_();
  }

  bool empty(){
    return as.empty();  
  }

  bool isIn(CT1<NumImp> elem){
    return as.isIn(elem);
  }

  AtomSetAbs cap(AtomSetAbs &aset2){
    AtomSetAbs aux(as.cap(aset2.as));
    return aux;
  }

  CT2<AtomSetAbs> diff(AtomSetAbs &aset2){
    CT2<ASetImp> diffRes = as.diff(aset2.as);

    CT2<AtomSetAbs> res;
    typename CT2<AtomSetAbs>::iterator itRes = res.begin();

    BOOST_FOREACH(ASetImp as, diffRes){
      itRes = res.insert(itRes, AtomSetAbs(as));
      ++itRes;
    }

    return res;
  }

  AtomSetAbs crossProd(AtomSetAbs &aset2){
    return AtomSetAbs(as.crossProd(aset2.as));
  }
 
  CT1<NumImp> minElem(){
    return as.minElem();
  }

  AtomSetAbs replace(IntervalImp &i, int dim){
    return AtomSetAbs(as.replace(i, dim));
  }

  bool operator==(const AtomSetAbs &other) const{
    return as == other.as;
  }

  bool operator!=(const AtomSetAbs &other) const{
    return as != other.as;
  }

  size_t hash(){
    return as.hash();
  }

  private:
  ASetImp as;
  int ndim;
};

template<template<typename T, typename = allocator<T>> class CT1,
         template<typename Value, typename Hash = boost::hash<Value>, 
                  typename Pred = std::equal_to<Value>, 
                  typename Alloc = std::allocator<Value>> class CT2,
         typename ASetImp, typename MultiInterImp, typename IntervalImp, typename NumImp>
size_t hash_value(AtomSetAbs<CT1, CT2, ASetImp, MultiInterImp, IntervalImp, NumImp> as){
  return as.hash();
}

// Sets --------------------------------------------------------------------------------------------

template<template<typename T, typename = allocator<T>> class CT1,
         template<typename Value, typename Hash = boost::hash<Value>, 
                  typename Pred = std::equal_to<Value>, 
                  typename Alloc = std::allocator<Value>> class CT2,
         typename ASetImp, typename NumImp>
struct SetImp1{
  typedef CT2<ASetImp> SetType;
  typedef typename SetType::iterator SetIt;

  SetType asets;
  int ndim;
 
  SetImp1(){
    SetType aux;
    asets = aux;
    ndim = 0;
  }
  SetImp1(SetType ss){
    ASetImp aux2;

    if(!ss.empty()){
      aux2 = *(ss.begin());
      int aux1 = aux2.ndim_();
      bool equalDims = true;
      // Check if all atomic sets have the same dimension
      BOOST_FOREACH(ASetImp as, ss){
        if(aux1 != as.ndim_())
          equalDims = false;
      }

      if(equalDims && aux1 != 0){
        asets = ss;
        ndim = aux1; 
      }
    
      else{
        //WARNING("Using atomics sets of different sizes");

        SetType aux3;
        asets = aux3;
        ndim = 0;
      }
    }

    else{
      asets = ss;
      ndim = 0;
    }
  }

  SetType asets_(){
    return asets;
  }

  int ndim_(){
    return ndim;
  }

  bool empty(){
    if(asets.empty())
      return true;
  
    return false;
  }

  bool isIn(CT1<NumImp> elem){
    BOOST_FOREACH(ASetImp as, asets){
      if(as.isIn(elem))
        return true;
    }

    return false;
  }

  void addAtomSet(ASetImp &aset2){
    if(!aset2.empty() && aset2.ndim_() == ndim && !asets.empty())
      asets.insert(aset2);

    else if(!aset2.empty() && asets.empty()){
      asets.insert(aset2);
      ndim = aset2.ndim_();
    }
 
    //else
      //WARNING("Atomic sets should have the same dimension");
  }

  void addAtomSets(SetType &sets2){
    ASetImp aux;

    SetIt it = sets2.begin();

    while(it != sets2.end()){
      aux = *it;
      addAtomSet(aux);

      ++it;
    }
  }

  SetImp1 cap(SetImp1 &set2){
    ASetImp aux1, aux2;

    if(asets.empty() || set2.asets.empty()){
      SetImp1 emptyRes;
      return emptyRes; 
    }
    
    SetType res;

    BOOST_FOREACH(ASetImp as1, asets){
      BOOST_FOREACH(ASetImp as2, set2.asets){
        ASetImp capres = as1.cap(as2);
      
        if(!capres.empty())
          res.insert(capres);
      }
    }

    return SetImp1(res);
  }

  SetImp1 diff(SetImp1 &set2){
    SetImp1 res;
    SetType capres = cap(set2).asets; 

    if(!capres.empty()){

      BOOST_FOREACH(ASetImp as1, asets){
        SetType aux;
        aux.insert(as1);
 
        BOOST_FOREACH(ASetImp as2, capres){
          SetImp1 newSets;

          BOOST_FOREACH(ASetImp as3, aux){
            SetType diffres = as3.diff(as2);
            newSets.addAtomSets(diffres);
          }

          aux = newSets.asets;
        }

        res.addAtomSets(aux);
      }
    }

    else
      res.addAtomSets(asets);

    return res; 
  }

  SetImp1 cup(SetImp1 &set2){
    SetImp1 res = *this;
    SetImp1 aux = set2.diff(*this);
 
    if(!aux.empty()) 
      res.addAtomSets(aux.asets);

    return res;
  }

  SetImp1 crossProd(SetImp1 &set2){
    SetType res;

    BOOST_FOREACH(ASetImp as1, asets.end){
      BOOST_FOREACH(ASetImp as2, set2.asets){
        ASetImp auxres = as1.crossProd(as2);
        res.addAtomSet(auxres);
      }
    }

    return SetImp1(res);
  }

  CT1<NumImp> minElem(){
    CT2<CT1<NumImp>> mins;
    typename CT2<CT1<NumImp>>::iterator itmins = mins.begin();

    // Get each min element of each atomic set
    BOOST_FOREACH(ASetImp as1, asets){
      itmins = mins.insert(itmins, as1.minElem());
      ++itmins;
    }

    bool hasValue = false;
    CT1<NumImp> res;
  
    itmins = mins.begin();
    BOOST_FOREACH(CT1<NumImp> n2, mins){
      // Set initial minimum
      if(!hasValue && !n2.empty()){
        res = n2;
        hasValue = true;
      }

      // Check if there is another element lower than our current min
      if(hasValue && !n2.empty()){
        typename CT1<NumImp>::iterator it2 = n2.begin();

        // Find the first component in which they differ. It determines
        // which one is lower.
        BOOST_FOREACH(NumImp n1, res){
          if(*it2 < n1)
            res = n2;

          else if(n1 < *it2)
            break;

          ++it2;
        }
      }      
    }

    return res;
  }

  bool operator==(const SetImp1 &other) const{
    return asets == other.asets;
  }

  bool operator!=(const SetImp1 &other) const{
    return asets != other.asets;
  }

  size_t hash(){
    return asets.size();
  }
};

template<template<typename T, typename = allocator<T>> class CT1, 
         template<typename Value, typename Hash = boost::hash<Value>, 
                  typename Pred = std::equal_to<Value>, 
                  typename Alloc = std::allocator<Value>> class CT2,
         typename ASetImp, typename NumImp>
size_t hash_value(SetImp1<CT1, CT2, ASetImp, NumImp> s){
  return s.hash();
}

template<template<typename T, typename = allocator<T>> class CT1,
         template<typename Value, typename Hash = boost::hash<Value>, 
                  typename Pred = std::equal_to<Value>, 
                  typename Alloc = std::allocator<Value>> class CT2,
          typename SetImp, typename ASetImp, typename NumImp>
struct SetAbs{
  SetAbs(){
    SetImp aux;
    set = aux;
  }
  SetAbs(SetImp ss){
    set = ss;
  }
  SetAbs(CT2<ASetImp> ass){
    SetImp aux(ass);
    set = aux;
  }

  bool empty(){
    return set.empty(); 
  }
 
  bool isIn(CT1<NumImp> elem){
    return set.isIn(elem);
  }

  CT2<ASetImp> asets_(){
    return set.asets_();
  }

  int ndim_(){
    return set.ndim_();
  }

  void addAtomSet(ASetImp &aset2){
    set.addAtomSet(aset2); 
  }

  void addAtomSets(CT2<ASetImp> &sets2){
    set.addAtomSets(sets2);
  }

  SetAbs cap(SetAbs &set2){
    return SetAbs(set.cap(set2.set));
  }

  SetAbs diff(SetAbs &set2){
    return SetAbs(set.diff(set2.set)); 
  }

  SetAbs cup(SetAbs &set2){
    return SetAbs(set.cup(set2.set));
  }

  SetAbs crossProd(SetAbs &set2){
    return SetAbs(set.crossProd(set2.set));
  }

  CT1<NumImp> minElem(){
    return set.minElem();
  }

  bool operator==(const SetAbs &other) const{
    return set == other.set;
  }

  bool operator!=(const SetAbs &other) const{
    return set != other.set;
  }

  size_t hash(){
    return set.hash();
  }

  private:
  SetImp set;
  int ndim;
};

template<template<typename T, typename = allocator<T>> class CT1,
         template<typename Value, typename Hash = boost::hash<Value>, 
                  typename Pred = std::equal_to<Value>, 
                  typename Alloc = std::allocator<Value>> class CT2,
          typename SetImp, typename ASetImp, typename NumImp>
size_t hash_value(SetAbs<CT1, CT2, SetImp, ASetImp, NumImp> s){
  return s.hash();
}

// LinearMaps ---------------------------------------------------------------------------------------

template <template<typename T, typename = std::allocator<T>> class CT,
          typename NumImp>
struct LMapImp1{
  typedef CT<NumImp> CTNum;
  typedef typename CTNum::iterator CTNumIt;

  CTNum gain;
  CTNum offset;
  int ndim;

  LMapImp1(){
    CTNum aux1;
    CTNum aux2;
    gain = aux2;
    offset = aux1;
    ndim = 0;
  }
  LMapImp1(CTNum g, CTNum o){
    bool negative = false;

    BOOST_FOREACH(NumImp gi, g){
      if(gi < 0)
        negative = true;
    }

    if(!negative){
      if(g.size() == o.size()){
        gain = g;
        offset = o;
        ndim = g.size();
      }

      else{
        //WARNING("Offset and gain should be of the same size");

        CTNum aux1;
        CTNum aux2;
        gain = aux2;
        offset = aux1;
        ndim = 0;
      }
    }

    else{
      //WARNING("All gains should be positive");

      CTNum aux1;
      CTNum aux2;
      gain = aux2;
      offset = aux1;
      ndim = 0;
    }
  }
  // Constructs the id of LMaps
  LMapImp1(int dim){
    CTNum g;
    CTNumIt itg = g.begin();
    CTNum o;
    CTNumIt ito = o.begin();

    for(int i = 0; i < dim; i++){
      itg = g.insert(itg, 1.0);
      ++itg;
      ito = o.insert(ito, 0);
      ++ito;
    }

    gain = g;
    offset = o;
    ndim = dim;
  }

  CTNum gain_(){
    return gain;
  }

  CTNum off_(){
    return offset;
  }

  int ndim_(){
    return ndim;
  }

  bool empty(){
    if(gain.empty() && offset.empty())
      return true;

    return false;
  }

  void addGO(NumImp g, NumImp o){
    if(g >= 0){
      gain.insert(gain.end(), g);
      offset.insert(offset.end(), o);
      ++ndim;
    }

    //else
      //WARNING("Gain should be positive");
  }

  LMapImp1 compose(LMapImp1 &lm2){
    CTNum resg;
    CTNumIt itresg = resg.begin();
    CTNum reso;
    CTNumIt itreso = reso.begin();

    CTNumIt ito1 = offset.begin();
    CTNumIt itg2 = lm2.gain.begin();
    CTNumIt ito2 = lm2.offset.begin();

    if(ndim == lm2.ndim){
      BOOST_FOREACH(NumImp g1i, gain){
        itresg = resg.insert(itresg, g1i * (*itg2));
        ++itresg;
        itreso = reso.insert(itreso, (*ito2) * g1i + (*ito1));
        ++itreso;

        ++ito1;
        ++itg2;
        ++ito2;
      } 
    }

    else{
      //WARNING("Linear maps should be of the same size");
      LMapImp1 aux;
      return aux;
    }

    return LMapImp1(resg, reso);
  }

  LMapImp1 invLMap(){
    CTNum resg;
    CTNumIt itresg = resg.begin();
    CTNum reso;
    CTNumIt itreso = reso.begin();

    CTNumIt ito1 = offset.begin();

    BOOST_FOREACH(NumImp g1i, gain){
      if(g1i != 0){
        itresg = resg.insert(itresg, 1 / g1i);
        ++itresg;

        itreso = reso.insert(itreso, -(*ito1) / g1i);
        ++itreso;
      }

      else{
        itresg = resg.insert(itresg, Inf);
        ++itresg;
      
        itreso = reso.insert(itreso, -Inf);
        ++itreso;
      }

      ++ito1;
    }

    return LMapImp1(resg, reso);
  }
 
  bool operator==(const LMapImp1 &other) const{
    return gain == other.gain && offset == other.offset;
  }
};

template<template<typename T, typename = std::allocator<T>> class CT,
         typename LMapImp, typename NumImp>
struct LMapAbs{
  typedef CT<NumImp> CTNum;
  typedef typename CTNum::iterator CTNumIt;

  LMapAbs(){
    LMapImp aux;
    lm = aux;
  }
  LMapAbs(LMapImp lmap){
    lm = lmap;
  }
  LMapAbs(CTNum g, CTNum o){
    lm = LMapImp(g, o);
  }
  LMapAbs(int dim){
    lm = LMapImp(dim); 
  }
  
  CTNum gain_(){
    return lm.gain_();
  }

  CTNum off_(){
    return lm.off_();
  }

  int ndim_(){
    return lm.ndim_();
  }

  bool empty(){
    return lm.empty();
  }

  void addGO(NumImp g, NumImp o){
    lm.addGO(g, o);
  }

  LMapAbs compose(LMapAbs &lm2){
    return LMapAbs(lm.compose(lm2.lm));
  }

  LMapAbs invLMap(){
    return LMapAbs(lm.invLMap());
  } 

  bool operator==(const LMapAbs &other) const{
    return lm == other.lm;
  }

  private:
  LMapImp lm;
  int ndim;
};

// Piecewise atomic linear maps -----------------------------------------------------------------

template<template<typename T, typename = allocator<T>> class CT,
         typename LMapImp, typename ASetImp, typename MultiInterImp, typename IntervalImp, 
         typename NumImp1, typename NumImp2>
struct PWAtomLMapImp1{
  ASetImp dom;
  LMapImp lmap;

  PWAtomLMapImp1(){}
  PWAtomLMapImp1(ASetImp d, LMapImp l){
    ASetImp aux1;
    LMapImp aux2;

    if(d.ndim_() != l.ndim_()){
      //WARNING("Atomic set and map should be of the same dimension");

      dom = aux1;
      lmap = aux2;
    }

    else{
      CT<IntervalImp> ints = d.aset_().inters_(); 
      CT<NumImp2> g = l.gain_();
      typename CT<NumImp2>::iterator itg = g.begin();
      CT<NumImp2> o = l.off_();
      typename CT<NumImp2>::iterator ito = o.begin();
      bool incompatible = false;

      CT<IntervalImp> auxdom;

      BOOST_FOREACH(IntervalImp i, ints){
        NumImp2 auxLo = i.lo_() * (*itg) + (*ito); 
        NumImp2 auxStep = i.step_() * (*itg);
        NumImp2 auxHi = i.hi_() * (*itg) + (*ito);

        if(*itg < Inf){
          if(auxLo != (int) auxLo && i.lo_()){
            //WARNING("Incompatible map");
            incompatible = true;
          }

          if(auxStep != (int) auxStep && i.step_()){
            //WARNING("Incompatible map");
            incompatible = true;
          }

          if(auxHi != (int) auxHi && i.hi_()){
            //WARNING("Incompatible map");
            incompatible = true;
          }

          ++itg;
          ++ito;
        }
      }

      if(incompatible){
        dom = aux1;
        lmap = aux2;
      }

      else{
        dom = d; 
        lmap = l;
      }
    }
  }

  ASetImp dom_(){
    return dom;
  }

  LMapImp lmap_(){
    return lmap;
  }

  bool empty(){
    return dom.empty() && lmap.empty();
  }
 
  ASetImp image(ASetImp &s){
    CT<IntervalImp> inters = (s.cap(dom)).aset_().inters_(); 
    CT<NumImp2> g = lmap.gain_();
    typename CT<NumImp2>::iterator itg = g.begin();
    CT<NumImp2> o = lmap.off_();
    typename CT<NumImp2>::iterator ito = o.begin();

    CT<IntervalImp> res;
    typename CT<IntervalImp>::iterator itres = res.begin();

    if(dom.empty()){
      ASetImp aux2;
      return aux2;
    }

    BOOST_FOREACH(IntervalImp capi, inters){
      NumImp1 newLo;
      NumImp1 newStep;
      NumImp1 newHi;

      NumImp2 auxLo = capi.lo_() * (*itg) + (*ito);
      NumImp2 auxStep = capi.step_() * (*itg);
      NumImp2 auxHi = capi.hi_() * (*itg) + (*ito);

      if(*itg < Inf){
        if(auxLo >= Inf)
          newLo = Inf;
        else 
          newLo = (NumImp1) auxLo;

        if(auxStep >= Inf)
          newStep = Inf;
        else
          newStep = (NumImp1) auxStep;

        if(auxHi >= Inf)
          newHi = Inf;
        else 
          newHi = (NumImp1) auxHi;
      }

      else{
        newLo = 1;
        newStep = 1;
        newHi = Inf;
      }

      IntervalImp aux1(newLo, newStep, newHi); 
      itres = res.insert(itres, aux1);
      ++itres;

      ++itg;
      ++ito;
    }

    MultiInterImp aux2(res);
    return ASetImp(aux2);
  } 

  ASetImp preImage(ASetImp &s){
    ASetImp fullIm = image(dom);
    ASetImp actualIm = fullIm.cap(s);
    PWAtomLMapImp1 inv(actualIm, lmap.invLMap());  

    ASetImp aux = inv.image(actualIm);
    return dom.cap(aux);
  }

  bool operator==(const PWAtomLMapImp1 &other) const{
    return dom == other.dom && lmap == other.lmap;
  }
};

template<typename PWAtomLMapImp, typename LMapImp, typename ASetImp>
struct PWAtomLMapAbs{
  PWAtomLMapAbs(){}
  PWAtomLMapAbs(ASetImp d, LMapImp l){
    pw = PWAtomLMapImp(d, l);
  }
  PWAtomLMapAbs(PWAtomLMapImp &pwatom){
    pw = pwatom;
  }

  ASetImp dom_(){
    return pw.dom_();
  }

  LMapImp lmap_(){
    return pw.lmap_();
  }

  bool empty(){
    return pw.empty();
  }

  ASetImp image(ASetImp &s){
   return pw.image(s);
  }

  ASetImp preImage(ASetImp &s){
    return pw.preImage(s);
  }

  PWAtomLMapAbs minAtomPW(PWAtomLMapAbs &pw2){
    PWAtomLMapImp aux = pw.minAtomPW(pw2.pw);
    return PWAtomLMapAbs(aux);
  }

  bool operator==(const PWAtomLMapAbs &other){
    return pw == other.pw;
  }

  private:
  PWAtomLMapImp pw; 
};

// Piecewise linear maps ------------------------------------------------------------------------

template<template<typename T, class = allocator<T>> class CT1,
         template<typename Value, typename Hash = boost::hash<Value>, 
                  typename Pred = std::equal_to<Value>, 
                  typename Alloc = std::allocator<Value>> class CT2,
         typename PWAtomLMapImp, typename LMapImp, typename SetImp, typename ASetImp, 
         typename NumImp1, typename NumImp2>
struct PWLMapImp1{
  typedef CT1<SetImp> CTSet;
  typedef typename CT1<SetImp>::iterator CTSetIt;
  typedef CT1<LMapImp> CTLMap;
  typedef typename CT1<LMapImp>::iterator CTLMapIt;

  CT1<SetImp> dom; 
  CT1<LMapImp> lmap;  
  int ndim = 0;

  PWLMapImp1(){}
  PWLMapImp1(CTSet d, CTLMap l){
    CTLMapIt itl = l.begin();
    int auxndim = (*(d.begin())).ndim_();
    bool different = false;
  
    if(d.size() == l.size()){
      BOOST_FOREACH(SetImp sd, d){
        if(sd.ndim_() != auxndim || (*itl).ndim_() != auxndim)
          different = true;

        ++itl; 
      }

      if(different){
        //WARNING("Sets and maps should have the same dimension");

        CTSet aux1;
        CTLMap aux2;
        dom = aux1;
        lmap = aux2;
        ndim = 0;
      }

      else{
        dom = d;
        lmap = l;
        ndim = auxndim;
      }
    }

    else{
      //WARNING("Domain size should be equal to map size");

      CTSet aux1;
      CTLMap aux2;
      dom = aux1;
      lmap = aux2;
      ndim = 0;     
    }
  }
  // Id PWLMap, the set stays the same, the map is the id map
  PWLMapImp1(SetImp &s){
    CTSet d;
    CTLMap lm;

    LMapImp aux(s.ndim_());    

    d.insert(d.begin(), s);
    lm.insert(lm.begin(), aux);

    dom = d;
    lmap = lm;
    ndim = 1;
  }

  CTSet dom_(){
    return dom;
  }

  CTLMap lmap_(){
    return lmap;
  }

  int ndim_(){
    return ndim;
  }

  bool empty(){
    return dom.empty() && lmap.empty();
  }

  void addSetLM(SetImp s, LMapImp lm){
    dom.insert(dom.end(), s);
    lmap.insert(lmap.end(), lm);
    PWLMapImp1 auxpw(dom, lmap);

    dom = auxpw.dom;
    lmap = auxpw.lmap;
  }

  void addLMSet(LMapImp lm, SetImp s){
    dom.insert(dom.begin(), s);
    lmap.insert(lmap.begin(), lm);
    PWLMapImp1 auxpw(dom, lmap);
    
    dom = auxpw.dom;
    lmap = auxpw.lmap;
  }

  SetImp image(SetImp &s){
    CTLMapIt itl = lmap.begin(); 

    SetImp res;

    BOOST_FOREACH(SetImp ss, dom){
      SetImp aux1 = ss.cap(s);
      SetImp partialRes;
      
      CT2<ASetImp> aux1as = aux1.asets_();
      BOOST_FOREACH(ASetImp as, aux1as){
        PWAtomLMapImp auxMap(as, *itl);
        ASetImp aux2 = auxMap.image(as);
        partialRes.addAtomSet(aux2);
      } 

      res = res.cup(partialRes);

      ++itl;
    }

    return res;
  }

  SetImp preImage(SetImp &s){
    CTLMapIt itl = lmap.begin();

    SetImp res;

    BOOST_FOREACH(SetImp ss, dom){
       SetImp partialRes;

       CT2<ASetImp> ssas = ss.asets_();
       BOOST_FOREACH(ASetImp as1, ssas){
         PWAtomLMapImp auxMap(as1, *itl);
         
         CT2<ASetImp> sas = s.asets_();
         BOOST_FOREACH(ASetImp as2, sas){
           ASetImp aux2 = auxMap.preImage(as2);
           partialRes.addAtomSet(aux2);
         }
       }

       res = res.cup(partialRes);
 
       ++itl;
    }

    return res;
  } 

  PWLMapImp1 compPW(PWLMapImp1 &pw2){
    CTLMapIt itlm1 = lmap.begin();
    CTLMapIt itlm2 = pw2.lmap.begin();     
 
    CTSet ress;
    CTSetIt itress = ress.begin();
    CTLMap reslm;
    CTLMapIt itreslm = reslm.begin();

    SetImp auxDom;
    SetImp newDom;

    BOOST_FOREACH(SetImp d1, dom){
      itlm2 = pw2.lmap.begin();     

      BOOST_FOREACH(SetImp d2, pw2.dom){
        auxDom = pw2.image(d2);
        auxDom = auxDom.cap(d1);
        auxDom = pw2.preImage(auxDom);
        newDom = auxDom.cap(d2); 

        if(!newDom.empty()){
          LMapImp newLM((*itlm1).compose(*itlm2));

          itress = ress.insert(itress, newDom);
          ++itress;
          itreslm = reslm.insert(itreslm, newLM); 
          ++itreslm; 
        }

        ++itlm2;
      }

      ++itlm1;
    }

    return PWLMapImp1(ress, reslm);
  }
 
  PWLMapImp1 minInvCompact(){
    // Only one component
    if(dom.size() == 1){
      SetImp auxDom = (*(dom.begin()));
      SetImp domInv = image(auxDom);
      LMapImp auxMap = (*(lmap.begin()));
      LMapImp mapInv = auxMap.invLMap();
      CT1<NumImp1> min = auxDom.minElem();
      typename CT1<NumImp1>::iterator itmin = min.begin();

      CT1<NumImp2> resg;
      typename CT1<NumImp2>::iterator itresg = resg.begin();
      CT1<NumImp2> reso;
      typename CT1<NumImp2>::iterator itreso = reso.begin();

      CT1<NumImp2> g = mapInv.gain_();
      CT1<NumImp2> o = mapInv.off_(); 
      typename CT1<NumImp2>::iterator ito = o.begin();
      BOOST_FOREACH(NumImp2 gi, g){
        if(gi == Inf){
          itresg = resg.insert(itresg, 0);
          ++itresg;
          itreso = reso.insert(itreso, *itmin);
          ++itreso;
        }

        else{
          itresg = resg.insert(itresg, gi);
          ++itresg;
          itreso = reso.insert(itreso, *ito);
          ++itreso;
        }

        ++ito;
        ++itmin;
      }

      CTSet auxDomRes;
      auxDomRes.insert(auxDomRes.begin(), domInv);
      LMapImp auxLM(resg, reso);
      CTLMap auxLMRes;
      auxLMRes.insert(auxLMRes.begin(), auxLM);
      return PWLMapImp1(auxDomRes, auxLMRes); 
    }

    else{
      //WARNING("There should be only one component");

      PWLMapImp1 aux;
      return aux;
    }
  }

  SetImp wholeDom(){
    SetImp res; 

    BOOST_FOREACH(SetImp s, dom){
      res = res.cup(s);
    }

    return res;
  }

  PWLMapImp1 combine(PWLMapImp1 &pw2){
    CTSet sres = dom;
    CTSetIt its = sres.end();
    CTLMap lres = lmap;
    CTLMapIt itl = lres.end();

    if(empty())
      return pw2;

    else if(pw2.empty())
      return *this;

    else{
      SetImp aux1 = wholeDom(); 
      CTSet dom2 = pw2.dom_(); 
      CTLMap lm2 = pw2.lmap_();
      CTLMapIt itlm2 = lm2.begin();

      BOOST_FOREACH(SetImp s2, dom2){
        SetImp newDom = s2.diff(aux1);
        
        if(!newDom.empty()){
          its = sres.insert(its, newDom);
          ++its;
          itl = lres.insert(itl, *itlm2);
          ++itl;
        }

        ++itlm2;
      }
    }

    PWLMapImp1 res(sres, lres);
    return res;
  }
 
  PWLMapImp1 atomize(){
    CTSet dres;
    CTSetIt itdres = dres.begin();
    CTLMap lres;
    CTLMapIt itlres = lres.begin(); 

    CTLMapIt itlm = lmap.begin();
    BOOST_FOREACH(SetImp d, dom){
      BOOST_FOREACH(ASetImp as, d.asets_()){
        SetImp aux;
        aux.addAtomSet(as);

        itdres = dres.insert(itdres, aux);
        ++itdres;
        itlres = lres.insert(itlres, *itlm);
        ++itlres;
      }

      ++itlm;
    }   

    PWLMapImp1 res(dres, lres);
    return res;
  }

  bool operator==(const PWLMapImp1 &other) const{
    return dom == other.dom && lmap == other.lmap;
  }
};

template<template<typename T, class = allocator<T>> class CT,
         typename PWLMapImp, typename LMapImp, typename SetImp>
struct PWLMapAbs{
  typedef CT<SetImp> CTSet;
  typedef CT<LMapImp> CTLMap;

  PWLMapAbs(){}
  PWLMapAbs(CTSet d, CTLMap l){
    pw = PWLMapImp(d, l);
  }
  PWLMapAbs(SetImp &s){
    pw = PWLMapImp(s);
  }
  PWLMapAbs(PWLMapImp &pwimp){
    pw = pwimp;
  }

  CTSet dom_(){
    return pw.dom_();
  }

  CTLMap lmap_(){
    return pw.lmap_();
  }

  int ndim_(){
    return pw.ndim_();
  }

  bool empty(){
    return pw.empty();
  }

  void addSetLM(SetImp s, LMapImp lm){
    pw.addSetLM(s, lm);
  }

  void addLMSet(LMapImp lm, SetImp s){
    pw.addLMSet(lm, s);
  }

  SetImp image(SetImp &s){
    return pw.image(s);
  }

  SetImp preImage(SetImp &s){
    return pw.preImage(s);
  }

  PWLMapAbs compPW(PWLMapAbs &pw2){
    PWLMapImp aux = pw.compPW(pw2.pw);
    return PWLMapAbs(aux);
  }

  PWLMapAbs minInvCompact(){
    PWLMapImp aux = pw.minInvCompact();
    return PWLMapAbs(aux);
  }

  SetImp wholeDom(){
    return pw.wholeDom();
  }

  PWLMapAbs combine(PWLMapAbs &pw2){
    PWLMapImp aux = pw.combine(pw2.pw);
    return PWLMapAbs(aux);
  }
 
  PWLMapAbs atomize(){
    PWLMapImp aux = pw.atomize();
    return PWLMapAbs(aux);
  }

  bool operator==(const PWLMapAbs &other) const{
    return pw == other.pw;
  }

  private:
  PWLMapImp pw;
};

typedef int NI1;
typedef float NI2;

template<typename T, class = allocator<T>>
using OrdCT = list<T>;

template<typename Value, typename Hash = boost::hash<Value>, 
         typename Pred = std::equal_to<Value>, 
         typename Alloc = std::allocator<Value>>
using UnordCT = boost::unordered_set<Value>;

typedef IntervalImp1<UnordCT> IntervalImp;
typedef IntervalAbs<UnordCT, IntervalImp, NI1> Interval;

ostream &operator<<(ostream &out, Interval &i);

typedef MultiInterImp1<OrdCT, UnordCT, Interval, NI1> MultiInterImp;
typedef MultiInterAbs<OrdCT, UnordCT, MultiInterImp, Interval, NI1> MultiInterval;

ostream &operator<<(ostream &out, MultiInterval &mi);

typedef AtomSetImp1<OrdCT, UnordCT, MultiInterval, Interval, NI1> AtomSetImp;
typedef AtomSetAbs<OrdCT, UnordCT, AtomSetImp, MultiInterval, Interval, NI1> AtomSet;

ostream &operator<<(ostream &out, AtomSet &as);

typedef SetImp1<OrdCT, UnordCT, AtomSet, NI1> SetImp;
typedef SetAbs<OrdCT, UnordCT, SetImp, AtomSet, NI1> Set;

ostream &operator<<(ostream &out, Set &s);

typedef LMapImp1<OrdCT, NI2> LMapImp;
typedef LMapAbs<OrdCT, LMapImp, NI2> LMap;

ostream &operator<<(ostream &out, LMap &lm);

typedef PWAtomLMapImp1<OrdCT, LMap, AtomSet, MultiInterval, Interval, NI1, NI2> PWAtomLMapImp;
typedef PWAtomLMapAbs<PWAtomLMapImp, LMap, AtomSet> PWAtomLMap;

ostream &operator<<(ostream &out, PWAtomLMap &pwatom);

typedef PWLMapImp1<OrdCT, UnordCT, PWAtomLMap, LMap, Set, AtomSet, NI1, NI2> PWLMapImp;
typedef PWLMapAbs<OrdCT, PWLMapImp, LMap, Set> PWLMap;

ostream &auxSetLMap(ostream &out, Set &s, LMap &lm);

ostream &operator<<(ostream &out, PWLMap &pw);

PWLMap minAtomPW(AtomSet &dom, LMap &lm1, LMap &lm2);
PWLMap minPW(Set &dom, LMap &lm1, LMap &lm2);
PWLMap minMap(PWLMap &pw1, PWLMap &pw2);

PWLMap reduceMapN(PWLMap pw, int dim);

PWLMap mapInf(PWLMap pw);

PWLMap minAdjCompMap(PWLMap pw2, PWLMap pw1);

PWLMap minAdjMap(PWLMap pw2, PWLMap pw1);

/*-----------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
// Graph definition
/*-----------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/

struct SetVertex{
  SetVertex(){
    name = "";
    id = -1;
    Set aux;
    vs = aux;
    index = 0;
  };
  SetVertex(string n, Set v){
    name = n;
    id = -1;
    vs = v;
    index = 0;
  }; 
  SetVertex(string n, int i, Set v, int ind){
    name = n;
    id = i;
    vs = v;
    index = ind;
  }; 

  Set vs_(){
    return vs;
  }

  // For pretty-printing
  string name;

  private:
  // Unique identifier
  int id;
  // Set vertex
  Set vs;
  // For debugging
  int index;
};

struct SetEdge{
  SetEdge(){
    name = "";
    id = -1;
    PWLMap aux;
    es1 = aux;
    es2 = aux;
    index = 0;
  };
  SetEdge(string nm, PWLMap e1, PWLMap e2){
    name = nm;
    id = -1;
    es1 = e1;
    es2 = e2;
    index = 0;
  };
  SetEdge(string nm, int i, PWLMap e1, PWLMap e2, int ind){
    name = nm;
    id = i;
    es1 = e1;
    es2 = e2;
    index = ind;
  };

  PWLMap es1_(){
    return es1;
  }

  PWLMap es2_(){
    return es2;
  }

  string name;

  private:
  int id;
  PWLMap es1;
  PWLMap es2;
  int index;
};

typedef boost::adjacency_list<boost::listS, boost::listS, boost::undirectedS, SetVertex, SetEdge>
 SBGraph;
typedef SBGraph::vertex_descriptor SetVertexDesc;
typedef boost::graph_traits<SBGraph>::vertex_iterator VertexIt;
typedef SBGraph::edge_descriptor SetEdgeDesc;
typedef boost::graph_traits<SBGraph>::edge_iterator EdgeIt;

PWLMap connectedComponents(SBGraph g);

#endif
