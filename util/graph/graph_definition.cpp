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
#include <util/graph/graph_definition.h>
#include <util/table.h>

namespace SBG {
size_t hash_value(SetVertex v) { return v.hash(); }

/*-----------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
// Data implementation
/*-----------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/

// Intervals --------------------------------------------------------------------------------------

INTER_TEMPLATE
INTER_TEMP_TYPE::IntervalImp1(){};

INTER_TEMPLATE
INTER_TEMP_TYPE::IntervalImp1(bool isEmpty) : lo_(-1), step_(-1), hi_(-1), empty_(isEmpty) {};

INTER_TEMPLATE
INTER_TEMP_TYPE::IntervalImp1(int vlo, int vstep, int vhi)
{
  if (vlo >= 0 && vstep > 0 && vhi >= 0) {
    empty_ = false;
    lo_ = vlo;
    step_ = vstep;

    if (vlo <= vhi && vhi < Inf) {
      int rem = std::fmod(vhi - vlo, vstep);
      hi_ = vhi - rem;
    }

    else if (vlo <= vhi && vhi == Inf) {
      hi_ = Inf;
    }

    else {
      // WARNING("Wrong values for subscript (check low <= hi)");
      empty_ = true;
    }
  }

  else if (vlo >= 0 && vstep == 0 && vhi == vlo) {
    empty_ = false;
    lo_ = vlo;
    hi_ = vhi;
    step_ = 1;
  }

  else {
    // WARNING("Subscripts should be positive");
    lo_ = -1;
    step_ = -1;
    hi_ = -1;
    empty_ = true;
  }
};

member_imp_temp(INTER_TEMPLATE, INTER_TEMP_TYPE, int, lo);
member_imp_temp(INTER_TEMPLATE, INTER_TEMP_TYPE, int, step);
member_imp_temp(INTER_TEMPLATE, INTER_TEMP_TYPE, int, hi);
member_imp_temp(INTER_TEMPLATE, INTER_TEMP_TYPE, bool, empty);

INTER_TEMPLATE
int INTER_TEMP_TYPE::gcd(int a, int b)
{
  int c;

  do {
    c = a % b;
    if (c > 0) {
      a = b;
      b = c;
    }
  } while (c != 0);

  return b;
}

INTER_TEMPLATE
int INTER_TEMP_TYPE::lcm(int a, int b)
{
  if (a < 0 || b < 0) return -1;

  return (a * b) / gcd(a, b);
}

INTER_TEMPLATE
bool INTER_TEMP_TYPE::isIn(int x)
{
  if (x < lo() || x > hi() || empty()) return false;

  float aux = fmod(x - lo(), step());
  if (aux == 0) return true;

  return false;
}

INTER_TEMPLATE
int INTER_TEMP_TYPE::card() 
{
  int res = 0;

  if (step() != 0)
    res = (hi() - lo()) / step() + 1;

  return res;
}

INTER_TEMPLATE
INTER_TEMP_TYPE INTER_TEMP_TYPE::offset(int off)
{
  int newLo = lo() + off;
  int newHi = hi() + off;

  if (!empty())
    return IntervalImp1(newLo, step(), newHi);

  else
    return IntervalImp1(true);
}

INTER_TEMPLATE
INTER_TEMP_TYPE INTER_TEMP_TYPE::cap(INTER_TEMP_TYPE i2)
{
  int maxLo = max(lo(), i2.lo()), newLo = -1;
  int newStep = lcm(step(), i2.step());
  int newEnd = min(hi(), i2.hi());

  if (!empty() && !i2.empty())
    for (int i = 0; i < newStep; i++) {
      int res1 = maxLo + i;

      if (isIn(res1) && i2.isIn(res1)) {
        newLo = res1;
        break;
      }
    }

  else
    return IntervalImp1(true);

  if (newLo < 0) return IntervalImp1(true);

  return IntervalImp1(newLo, newStep, newEnd);
}

INTER_TEMPLATE
UNORD_CT<INTER_TEMP_TYPE> INTER_TEMP_TYPE::diff(INTER_TEMP_TYPE i2)
{
  UNORD_CT<IntervalImp1> res;
  IntervalImp1 capres = cap(i2);

  if (capres.empty()) {
    res.insert(*this);
    return res;
  }

  if (capres == *this) return res;

  // "Before" intersection
  if (lo() < capres.lo()) {
    IntervalImp1 aux = IntervalImp1(lo(), 1, capres.lo() - 1);
    IntervalImp1 left = cap(aux);
    res.insert(left);
  }

  // "During" intersection
  if (capres.step() <= (capres.hi() - capres.lo())) {
    int nInters = capres.step() / step();
    for (int i = 1; i < nInters; i++) {
      IntervalImp1 aux = IntervalImp1(capres.lo() + i * step(), capres.step(), capres.hi());
      res.insert(aux);
    }
  }

  // "After" intersection
  if (hi() > capres.hi()) {
    IntervalImp1 aux = IntervalImp1(capres.hi() + 1, 1, hi());
    IntervalImp1 right = cap(aux);
    res.insert(right);
  }

  return res;
}

INTER_TEMPLATE
int INTER_TEMP_TYPE::minElem() { return lo(); }

INTER_TEMPLATE
int INTER_TEMP_TYPE::maxElem() { return hi(); }

INTER_TEMPLATE
bool INTER_TEMP_TYPE::operator==(const INTER_TEMP_TYPE &other) const 
{
  return (lo() == other.lo()) && (step() == other.step()) && (hi() == other.hi()) && (empty() == other.empty());
}

INTER_TEMPLATE
bool INTER_TEMP_TYPE::operator!=(const INTER_TEMP_TYPE &other) const
{
  return !(*this == other);
}

INTER_TEMPLATE
ostream &operator<<(ostream &out, const INTER_TEMP_TYPE &i)
{
  out << "[" << i.lo() << ":" << i.step() << ":" << i.hi() << "]";
  return out;
}

template struct IntervalImp1<UnordCT>;

size_t hash_value(const Interval &inter) 
{ 
  size_t seed = 0;
  boost::hash_combine(seed, inter.lo());
  return seed; 
}

// >>>>> To add new implementation, add new methods:
// X IntervalImp2::func1() { ... }
// X IntervalImp2::func2() { ... }
// ...
// ...
//
// template struct IntervalImp2<UnordCT>; --- If it is a template class
//
// Then modify hash_value implementation for Interval 

// Multi intervals --------------------------------------------------------------------------------

#define INTER_TYPE                  \
   typename MI_TEMP_TYPE::Intervals

MI_TEMPLATE
MI_TEMP_TYPE::MultiInterImp1() : ndim_(0)
{
  Intervals emptyRes;
  inters_ = emptyRes;
};

MI_TEMPLATE
MI_TEMP_TYPE::MultiInterImp1(INTER_TYPE is)
{
  IntervalsIt it = is.begin();
  bool areEmptys = false;

  while (it != is.end()) {
   if ((*it).empty()) areEmptys = true;

    ++it;
  }

  if (areEmptys) {
    Intervals aux;
    inters_ = aux;
    ndim_ = 0;
  }

  else {
    inters_ = is;
    ndim_ = is.size();
  }
}

member_imp_temp(MI_TEMPLATE, MI_TEMP_TYPE, INTER_TYPE, inters);
member_imp_temp(MI_TEMPLATE, MI_TEMP_TYPE, int, ndim);

MI_TEMPLATE
void MI_TEMP_TYPE::addInter(INTER_IMP i)
{
  if (!i.empty()) {
    inters_.insert(inters_.end(), i);
    ++ndim_;
  }
}

MI_TEMPLATE
bool MI_TEMP_TYPE::empty()
{
  if (inters().empty()) return true;

  return false;
}

MI_TEMPLATE
bool MI_TEMP_TYPE::isIn(ORD_CT<NUMERIC_IMP> elem)
{
  IntervalsIt it2 = inters_ref().begin();

  if ((int)elem.size() != ndim()) return false;

  BOOST_FOREACH (NUMERIC_IMP n, elem) {
    if (!(*it2).isIn(n)) return false;

    ++it2;
  }

  return true;
}

MI_TEMPLATE
MI_TEMP_TYPE MI_TEMP_TYPE::cap(MI_TEMP_TYPE mi2)
{
  Intervals res;
  IntervalsIt itres = res.begin();

  IntervalsIt it2 = mi2.inters_ref().begin();
  if (ndim() == mi2.ndim()) {
    BOOST_FOREACH (INTER_IMP i1, inters()) {
      INTER_IMP capres = i1.cap(*it2);

      if (capres.empty()) {
        Intervals aux;
        return MultiInterImp1(aux);
      }

      itres = res.insert(itres, capres);
      ++itres;

      ++it2;
    }
  }

  return MultiInterImp1(res);
}

MI_TEMPLATE
UNORD_CT<MI_TEMP_TYPE> MI_TEMP_TYPE::diff(MI_TEMP_TYPE mi2)
{
  MultiInterImp1 capres = cap(mi2);

  UNORD_CT<MultiInterImp1> resmi;

  if (inters_ref().empty()) return resmi;

  if (ndim() != mi2.ndim()) {
    return resmi;
  }

  if (capres.empty()) {
    resmi.insert(*this);
    return resmi;
  }

  if (inters() == capres.inters()) return resmi;

  IntervalsIt itcap = capres.inters_ref().begin();
  ORD_CT<UNORD_CT<INTER_IMP>> diffs;
  typename ORD_CT<UNORD_CT<INTER_IMP>>::iterator itdiffs = diffs.begin();

  BOOST_FOREACH (INTER_IMP i, inters()) {
    itdiffs = diffs.insert(itdiffs, i.diff(*itcap));

    ++itcap;
    ++itdiffs;
  }

  IntervalsIt it1 = inters_ref().begin();
  ++it1;
  itdiffs = diffs.begin();

  int count = 0;
  BOOST_FOREACH (UNORD_CT<INTER_IMP> vdiff, diffs) {
    BOOST_FOREACH (INTER_IMP i, vdiff) {
      if (!i.empty()) {
        Intervals resi;
        IntervalsIt itresi = resi.begin();

        itcap = capres.inters_ref().begin();

        if (count > 0) {
          for (int j = 0; j < count; j++) {
            itresi = resi.insert(itresi, *itcap);
            ++itresi;

            ++itcap;
          }
        }

        itresi = resi.insert(itresi, i);
        ++itresi;

        IntervalsIt auxit1 = it1;
        while (auxit1 != inters_ref().end()) {
          itresi = resi.insert(itresi, *auxit1);
          ++itresi;

          ++auxit1;
        }

        resmi.insert(MultiInterImp1(resi));
      }
    }

    ++count;
    ++it1;
  }

  return resmi;
}

MI_TEMPLATE
MI_TEMP_TYPE MI_TEMP_TYPE::crossProd(MI_TEMP_TYPE mi2)
{
  Intervals res;
  IntervalsIt itres = res.begin();

  BOOST_FOREACH (INTER_IMP i, inters()) {
    itres = res.insert(itres, i);
    ++itres;
  }

  BOOST_FOREACH (INTER_IMP i, mi2.inters()) {
    itres = res.insert(itres, i);
    ++itres;
  }

  return MultiInterImp1(res);
}

MI_TEMPLATE
ORD_CT<NUMERIC_IMP> MI_TEMP_TYPE::minElem()
{
    ORD_CT<NUMERIC_IMP> res;
    typename ORD_CT<NUMERIC_IMP>::iterator itres = res.begin();

    BOOST_FOREACH (INTER_IMP i, inters()) {
      if (i.empty()) {
        ORD_CT<NUMERIC_IMP> aux;
        return aux;
      }

      itres = res.insert(itres, i.minElem());
      ++itres;
    }

    return res;
}

MI_TEMPLATE
MI_TEMP_TYPE MI_TEMP_TYPE::replace(INTER_IMP i, int dim)
{
  Intervals auxRes;
  IntervalsIt itAux = auxRes.begin();
  int count = 1;

  BOOST_FOREACH (INTER_IMP ii, inters()) {
    if (dim == count)
      itAux = auxRes.insert(itAux, i);
     else
       itAux = auxRes.insert(itAux, ii);

    ++itAux;

    ++count;
  }

  return MultiInterImp1(auxRes);
}

MI_TEMPLATE
int MI_TEMP_TYPE::card()
{
  int res = 1;

  BOOST_FOREACH (INTER_IMP i, inters()) {
     res *= i.card();
 }

  if (inters().empty()) res = 0;

  return res;
}

MI_TEMPLATE
bool MI_TEMP_TYPE::operator==(const MI_TEMP_TYPE &other) const { return inters() == other.inters(); } 

MI_TEMPLATE
bool MI_TEMP_TYPE::operator!=(const MI_TEMP_TYPE &other) const { return inters() != other.inters(); } 

MI_TEMPLATE
bool MI_TEMP_TYPE::operator<(const MI_TEMP_TYPE &other) const 
{
  typename Intervals::const_iterator iti2 = other.inters().begin();

  BOOST_FOREACH (INTER_IMP i1, inters()) {
    INTER_IMP aux = *iti2;
    if (i1.minElem() < aux.minElem()) return true;

    ++iti2;
  }

  return false;
}

MI_TEMPLATE
size_t MI_TEMP_TYPE::hash()
{
  size_t seed = 0;
  boost::hash_combine(seed, card());
  return seed; 
}

MI_TEMPLATE
ostream &operator<<(ostream &out, const MI_TEMP_TYPE &mi)
{
  ORD_CT<INTER_IMP> is = mi.inters();
  typename ORD_CT<INTER_IMP>::iterator it = is.begin();

  if (is.size() == 0) return out;

  if (is.size() == 1) {
    out << *it;
    return out;
  }

  out << *it;
  ++it;
  while (next(it, 1) != is.end()) {
    out << "x" << *it;
    ++it;
  }
  out << "x" << *it;

  return out;
}

template struct MultiInterImp1<OrdCT, UnordCT, Interval, NI1>;

size_t hash_value(const MultiInterval &mi) { 
  MultiInterval aux = mi;
  return aux.hash(); 
}

// Atomic sets ------------------------------------------------------------------------------------

AS_TEMPLATE
AS_TEMP_TYPE::AtomSetImp1() 
{
  MI_IMP emptyRes;
  aset_ = emptyRes;
  ndim_ = 0;
}

AS_TEMPLATE
AS_TEMP_TYPE::AtomSetImp1(MI_IMP as)
{
  aset_ = as;
  ndim_ = as.ndim();
}

member_imp_temp(AS_TEMPLATE, AS_TEMP_TYPE, MI_IMP, aset);
member_imp_temp(AS_TEMPLATE, AS_TEMP_TYPE, int, ndim);

AS_TEMPLATE
bool AS_TEMP_TYPE::empty() { return aset_ref().empty(); }

AS_TEMPLATE
bool AS_TEMP_TYPE::isIn(ORD_CT<NUMERIC_IMP> elem) { return aset_ref().isIn(elem); }

AS_TEMPLATE
int AS_TEMP_TYPE::card() { return aset_ref().card(); }

AS_TEMPLATE
AS_TEMP_TYPE AS_TEMP_TYPE::cap(AS_TEMP_TYPE aset2) 
{
  AS_TEMP_TYPE res(aset_ref().cap(aset2.aset()));
  return res;
}

AS_TEMPLATE
UNORD_CT<AS_TEMP_TYPE> AS_TEMP_TYPE::diff(AS_TEMP_TYPE aset2)
{
  UNORD_CT<AtomSetImp1> res;

  UNORD_CT<MI_IMP> atomicDiff = aset_ref().diff(aset2.aset());

  if (atomicDiff.empty()) {
    UNORD_CT<AtomSetImp1> emptyRes;
    return emptyRes;
  }

  else {
    BOOST_FOREACH (MI_IMP mi, atomicDiff) 
      res.insert(AtomSetImp1(mi));
  }

  return res;
}

AS_TEMPLATE
ORD_CT<NUMERIC_IMP> AS_TEMP_TYPE::minElem() { return aset_ref().minElem(); }

AS_TEMPLATE
AS_TEMP_TYPE AS_TEMP_TYPE::crossProd(AS_TEMP_TYPE aset2) 
{ 
  return AtomSetImp1(aset_ref().crossProd(aset2.aset()));
}

AS_TEMPLATE
AS_TEMP_TYPE AS_TEMP_TYPE::replace(INTER_IMP i, int dim) 
{
  return AtomSetImp1(aset_ref().replace(i, dim));
}

AS_TEMPLATE
bool AS_TEMP_TYPE::operator==(const AS_TEMP_TYPE &other) const { return aset() == other.aset(); } 

AS_TEMPLATE
bool AS_TEMP_TYPE::operator!=(const AS_TEMP_TYPE &other) const { return aset() != other.aset(); } 

AS_TEMPLATE
size_t AS_TEMP_TYPE::hash()
{
  size_t seed = 0;
  boost::hash_combine(seed, aset_ref().hash());
  return seed; 
}

template struct AtomSetImp1<OrdCT, UnordCT, MultiInterval, Interval, NI1>;

size_t hash_value(const AtomSet &as) { 
  AtomSet aux = as.aset();
  return aux.hash(); 
}

ostream &operator<<(ostream &out, const AtomSet &as)
{
  MultiInterval mi = as.aset();

  out << "{" << mi << "}";

  return out;
}

// Sets -------------------------------------------------------------------------------------------

#define AS_TYPE                  \
   typename SET_TEMP_TYPE::AtomSets

SET_TEMPLATE
SET_TEMP_TYPE::SetImp1()
{
  AtomSets aux;
  asets_ = aux;
  ndim_ = 0;
}

SET_TEMPLATE
SET_TEMP_TYPE::SetImp1(AS_IMP as)
{
  asets_ref().insert(as);
  ndim_ = as.ndim();
}

SET_TEMPLATE
SET_TEMP_TYPE::SetImp1(AtomSets ss)
{
  AS_IMP aux2;

  if (!ss.empty()) {
    aux2 = *(ss.begin());
    int aux1 = aux2.ndim();
    bool equalDims = true;
    // Check if all atomic sets have the same dimension
    BOOST_FOREACH (AS_IMP as, ss) {
      if (aux1 != as.ndim()) equalDims = false;
    }

    if (equalDims && aux1 != 0) {
      asets_ = ss;
      ndim_ = aux1;
    }

    else {
      AtomSets aux3;
      asets_ = aux3;
      ndim_ = 0;
    }
  }

  else {
    asets_ = ss;
    ndim_ = 0;
  }
}

member_imp_temp(SET_TEMPLATE, SET_TEMP_TYPE, AS_TYPE, asets);
member_imp_temp(SET_TEMPLATE, SET_TEMP_TYPE, int, ndim);

SET_TEMPLATE
void SET_TEMP_TYPE::addAtomSet(AS_IMP aset2)
{
  if (!aset2.empty() && aset2.ndim() == ndim() && !empty())
    asets_.insert(aset2);

  else if (!aset2.empty() && empty()) {
    asets_.insert(aset2);
    ndim_ = aset2.ndim();
  }
}

SET_TEMPLATE
void SET_TEMP_TYPE::addAtomSets(AS_TYPE sets2)
{
  BOOST_FOREACH (AS_IMP as, sets2)
    addAtomSet(as);
}

SET_TEMPLATE
bool SET_TEMP_TYPE::empty()
{
  if (asets_ref().empty()) return true;

  bool res = true;
  BOOST_FOREACH (AS_IMP as, asets()) {
    if (!as.empty()) res = false;
  }

  return res;
}

SET_TEMPLATE
bool SET_TEMP_TYPE::isIn(ORD_CT<NUMERIC_IMP> elem)
{
  BOOST_FOREACH (AS_IMP as, asets()) 
    if (as.isIn(elem)) return true;

  return false;
}

SET_TEMPLATE
int SET_TEMP_TYPE::card()
{
  int res = 0;

  BOOST_FOREACH (AS_IMP as, asets())
    res += as.card();

  return res;
}

SET_TEMPLATE 
bool SET_TEMP_TYPE::subseteq(SET_TEMP_TYPE set2)
{
  SetImp1 sdiff = (*this).diff(set2);

  if (sdiff.empty()) return true;

  return false;
}

SET_TEMPLATE
bool SET_TEMP_TYPE::subset(SET_TEMP_TYPE set2)
{
  SetImp1 sdiff1 = (*this).diff(set2);
  SetImp1 sdiff2 = set2.diff(*this);

  if (sdiff1.empty() && !sdiff2.empty()) return true;

  return false;
}

SET_TEMPLATE
SET_TEMP_TYPE SET_TEMP_TYPE::cap(SET_TEMP_TYPE set2)
{
  AS_IMP aux1, aux2;

  if (empty() || set2.empty()) {
    SetImp1 emptyRes;
    return emptyRes;
  }

  AtomSets res;

  BOOST_FOREACH (AS_IMP as1, asets()) {
    BOOST_FOREACH (AS_IMP as2, set2.asets()) {
      AS_IMP capres = as1.cap(as2);

      if (!capres.empty()) res.insert(capres);
    }
  }

  return SetImp1(res);
}

SET_TEMPLATE
SET_TEMP_TYPE SET_TEMP_TYPE::diff(SET_TEMP_TYPE set2)
{
  SetImp1 res;
  AtomSets capres = cap(set2).asets();

  if (!capres.empty()) {
    BOOST_FOREACH (AS_IMP as1, asets()) {
      AtomSets aux;
      aux.insert(as1);

      BOOST_FOREACH (AS_IMP as2, capres) {
        SetImp1 newSets;

        BOOST_FOREACH (AS_IMP as3, aux) {
          AtomSets diffres = as3.diff(as2);
          newSets.addAtomSets(diffres);
        }

        aux = newSets.asets();
      }

      res.addAtomSets(aux);
    }
  }

  else
    res.addAtomSets(asets());

  return res;
}

SET_TEMPLATE
SET_TEMP_TYPE SET_TEMP_TYPE::cup(SET_TEMP_TYPE set2)
{
  SetImp1 res = *this;
  SetImp1 aux = set2.diff(*this);

  if (!aux.empty()) res.addAtomSets(aux.asets());

  return res;
}

SET_TEMPLATE
ORD_CT<NUMERIC_IMP> SET_TEMP_TYPE::minElem()
{
  ORD_CT<NUMERIC_IMP> res;

  if (empty()) return res;

  AS_IMP min = *(asets_ref().begin());

  BOOST_FOREACH (AS_IMP as1, asets()) 
    if (as1.aset_ref().minElem() < min.minElem()) min = as1;

  return min.minElem();
} 

SET_TEMPLATE
SET_TEMP_TYPE SET_TEMP_TYPE::crossProd(SET_TEMP_TYPE set2)
{
  AtomSets res;

  BOOST_FOREACH (AS_IMP as1, asets()) {
    BOOST_FOREACH (AS_IMP as2, set2.asets()) {
      AS_IMP auxres = as1.crossProd(as2);
      res.insert(auxres);
    }
  }

  return SetImp1(res);
}

SET_TEMPLATE
bool SET_TEMP_TYPE::operator==(const SET_TEMP_TYPE &other) const
{
  SetImp1 aux1 = *this;
  SetImp1 aux2 = other;
  SetImp1 diff1 = aux1.diff(aux2);
  SetImp1 diff2 = aux2.diff(aux1);

  if (diff1.empty() && diff2.empty())
    return true;

  return false; 
}

SET_TEMPLATE
bool SET_TEMP_TYPE::operator!=(const SET_TEMP_TYPE &other) const
{
  return !(*this == other); 
}

SET_TEMPLATE
size_t SET_TEMP_TYPE::hash()
{
  size_t seed = 0;
  boost::hash_combine(seed, asets_ref().size());
  return seed; 
}

template struct SetImp1<OrdCT, UnordCT, AtomSet, NI1>;

size_t hash_value(const Set &set) { 
  Set aux = set;
  return aux.hash(); 
}

ostream &operator<<(ostream &out, const Set &set)
{
  UnordCT<AtomSet> asets = set.asets();
  UnordCT<AtomSet>::iterator it = asets.begin();
  AtomSet aux;

  if (asets.size() == 0) {
    out << "{}";
    return out;
  }

  if (asets.size() == 1) {
    aux = *it;
    out << "{" << aux << "}";
    return out;
  }

  aux = *it;
  out << "{" << aux;
  ++it;
  while (next(it, 1) != asets.end()) {
    aux = *it;
    out << ", " << aux;
    ++it;
  }
  aux = *it;
  out << ", " << aux << "}";

  return out;
}

// Linear maps ------------------------------------------------------------------------------------

#define NUM_TYPE                  \
   typename LM_TEMP_TYPE::OrdNumeric

LM_TEMPLATE
LM_TEMP_TYPE::LMapImp1()
{
  OrdNumeric empty;
  gain_ = empty;
  offset_ = empty;
  ndim_ = 0;
}

LM_TEMPLATE
LM_TEMP_TYPE::LMapImp1(NUM_TYPE g, NUM_TYPE o)
{
  OrdNumeric empty;

  if (g.size() == o.size()) {
    gain_ = g;
    offset_ = o;
    ndim_ = g.size();
  }

  else {
    gain_ = empty;
    offset_ = empty;
    ndim_ = 0;
  }
}

LM_TEMPLATE
LM_TEMP_TYPE::LMapImp1(int dim)
{
  OrdNumeric g;
  OrdNumericIt itg = g.begin();
  OrdNumeric o;
  OrdNumericIt ito = o.begin();

  for (int i = 0; i < dim; i++) {
    itg = g.insert(itg, 1.0);
    ++itg;
    ito = o.insert(ito, 0);
    ++ito;
  }

  gain_ = g;
  offset_ = o;
  ndim_ = dim;
}

member_imp_temp(LM_TEMPLATE, LM_TEMP_TYPE, NUM_TYPE, gain);
member_imp_temp(LM_TEMPLATE, LM_TEMP_TYPE, NUM_TYPE, offset);
member_imp_temp(LM_TEMPLATE, LM_TEMP_TYPE, int, ndim);

LM_TEMPLATE
void LM_TEMP_TYPE::addGO(NUMERIC_IMP g, NUMERIC_IMP o)
{
  gain_.insert(gain_.end(), g);
  offset_.insert(offset_.end(), o);
  ++ndim_;
}

LM_TEMPLATE
bool LM_TEMP_TYPE::empty()
{
  if (gain_ref().empty() && offset_ref().empty()) return true;

  return false;
}

LM_TEMPLATE
LM_TEMP_TYPE LM_TEMP_TYPE::compose(LM_TEMP_TYPE lm2)
{
  OrdNumeric resg;
  OrdNumericIt itresg = resg.begin();
  OrdNumeric reso;
  OrdNumericIt itreso = reso.begin();

  OrdNumericIt ito1 = offset_ref().begin();
  OrdNumericIt itg2 = lm2.gain_ref().begin();
  OrdNumericIt ito2 = lm2.offset_ref().begin();

  if (ndim() == lm2.ndim()) {
    BOOST_FOREACH (NUMERIC_IMP g1i, gain()) {
      itresg = resg.insert(itresg, g1i * (*itg2));
      ++itresg;
      itreso = reso.insert(itreso, (*ito2) * g1i + (*ito1));
      ++itreso;

      ++ito1;
      ++itg2;
      ++ito2;
    }
  }

  else {
    LMapImp1 aux;
    return aux;
  }

  return LMapImp1(resg, reso);
}

LM_TEMPLATE
LM_TEMP_TYPE LM_TEMP_TYPE::invLMap()
{
  OrdNumeric resg;
  OrdNumericIt itresg = resg.begin();
  OrdNumeric reso;
  OrdNumericIt itreso = reso.begin();

  OrdNumericIt ito1 = offset_ref().begin();

  BOOST_FOREACH (NUMERIC_IMP g1i, gain()) {
    if (g1i != 0) {
      itresg = resg.insert(itresg, 1 / g1i);
      ++itresg;

      itreso = reso.insert(itreso, -(*ito1) / g1i);
      ++itreso;
    }

    else {
      itresg = resg.insert(itresg, Inf);
      ++itresg;

      itreso = reso.insert(itreso, -Inf);
      ++itreso;
    }

    ++ito1;
  }

  return LMapImp1(resg, reso);
}

LM_TEMPLATE
bool LM_TEMP_TYPE::operator==(const LM_TEMP_TYPE &other) const
{
  return gain() == other.gain() && offset() == other.offset();
}

template<typename NUMERIC_IMP>
std::string mapOper(NUMERIC_IMP &cte) { return (cte >= 0) ? "+ " : ""; }

LM_TEMPLATE
ostream &operator<<(ostream &out, const LM_TEMP_TYPE &lm)
{
  NUM_TYPE g = lm.gain();
  NUM_TYPE::iterator itg = g.begin();
  NUM_TYPE o = lm.offset();
  NUM_TYPE::iterator ito = o.begin();

  if (g.size() == 0) return out;

  if (g.size() == 1) {
    out << "[" << *itg << " * x " << mapOper<NUMERIC_IMP>(*ito) << *ito << "]";
    return out;
  }

  out << "[" << *itg << " * x " << mapOper<NUMERIC_IMP>(*ito) << *ito;
  ++itg;
  ++ito;
  while (next(itg, 1) != g.end()) {
    out << ", " << *itg << " * x " << mapOper<NUMERIC_IMP>(*ito) << *ito;

    ++itg;
    ++ito;
  }
  out << ", " << *itg << " * x " << mapOper<NUMERIC_IMP>(*ito) << *ito << "]";

  return out;
}

template struct LMapImp1<OrdCT, NI2>;

/*-----------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
// Printing instances
/*-----------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/

ostream &operator<<(ostream &out, PWAtomLMap &pwatom)
{
  AtomSet d = pwatom.dom_();
  LMap lm = pwatom.lmap_();

  out << "(" << d << ", " << lm << ")";
  return out;
}

ostream &operator<<(ostream &out, PWLMap &pw)
{
  OrdCT<Set> d = pw.dom_();
  OrdCT<Set>::iterator itd = d.begin();
  OrdCT<LMap> l = pw.lmap_();
  OrdCT<LMap>::iterator itl = l.begin();

  if (d.size() == 0) {
    out << "[]";
    return out;
  }

  if (d.size() == 1) {
    out << "[(" << *itd << ", " << *itl << ")]";
    return out;
  }

  out << "[(" << *itd << ", " << *itl << ") ";
  ++itd;
  ++itl;
  while (next(itd, 1) != d.end()) {
    out << ", (" << *itd << ", " << *itl << ")";

    ++itd;
    ++itl;
  }
  out << ", (" << *itd << ", " << *itl << ")]";

  return out;
}

ostream &operator<<(ostream &out, SetVertex &V)
{
  string Vnm = V.name_();
  Set Vvs = V.vs_();

  out << Vnm << ": " << Vvs << "\n";

  return out;
}

ostream &operator<<(ostream &out, SetEdge &E)
{
  string Enm = E.name_();
  OrdCT<Set> dom = E.es1_().dom_();
  OrdCT<Set>::iterator itdom = dom.begin();

  out << Enm << " dom: ";
  out << "[";
  for (; next(itdom, 1) != dom.end(); ++itdom)
    out << *itdom << ", ";
  out << *itdom << "]\n";

  OrdCT<LMap> lmleft = E.es1_().lmap_();
  OrdCT<LMap>::iterator itleft = lmleft.begin();
  OrdCT<LMap> lmright = E.es2_().lmap_();
  OrdCT<LMap>::iterator itright = lmright.begin();

  out << Enm << " left | right: ";
  out << "[";
  for (; next(itleft, 1) != lmleft.end(); ++itleft)
    out << *itleft << ", ";
  out << *itleft << "] | ";

  out << "[";
  for (; next(itright, 1) != lmright.end(); ++itright)
    out << *itright << ", ";
  out << *itright << "]\n";

  return out;
}

/*-----------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
// Map operations
/*-----------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/

// This operations are implemented here to simplify the implementation.

PWLMap offsetMap(OrdCT<NI1> &offElem, PWLMap &pw) 
{
  OrdCT<LMap> reslm;
  OrdCT<LMap>::iterator itreslm = reslm.begin();

  OrdCT<NI1>::iterator itelem;

  if ((int) offElem.size() == pw.ndim_()) {
    itelem = offElem.begin();

    BOOST_FOREACH (LMap lm, pw.lmap_()) {
      OrdCT<NI2> resg;
      OrdCT<NI2>::iterator itresg = resg.begin();
      OrdCT<NI2> reso;
      OrdCT<NI2>::iterator itreso = reso.begin();

      BOOST_FOREACH (NI2 gi, lm.gain()) {
        itresg = resg.insert(itresg, gi);

        ++itresg;
      }

      BOOST_FOREACH (NI2 oi, lm.offset()) {
        itreso = reso.insert(itreso, oi + (NI2) *itelem);

        ++itreso; 
        ++itelem;
      }

      itreslm = reslm.insert(itreslm, LMap(resg, reso));
    }
  }

  return PWLMap(pw.dom_(), reslm);
}

// This function calculates if pw1 and pw2 are equivalent
// (have the same whole dom, and the image of each dom is
// the same)
// e.g. [({1:1:10}, [1 * x + 0])] and 
//      [({1:1:3}, [1 * x + 0]), ({4:1:10}, [1 * x + 0])]
//      are equivalent
// but [({1:1:10}, [1 * x + 0])] and
//     [({1:1:10}, [0 * x + 1])]
//     aren't equivalent
bool equivalentPW(PWLMap pw1, PWLMap pw2)
{
  Set dom1 = pw1.wholeDom();
  Set dom2 = pw2.wholeDom();

  if (dom1 == dom2) {
    OrdCT<LMap> lm1 = pw1.lmap_();
    OrdCT<LMap>::iterator itlm1 = lm1.begin();

    BOOST_FOREACH (Set d1, pw1.dom_()) {
      OrdCT<LMap> lm2 = pw2.lmap_();
      OrdCT<LMap>::iterator itlm2 = lm2.begin();

      BOOST_FOREACH (Set d2, pw2.dom_()) { 
        Set domcap = d1.cap(d2);

        PWLMap auxpw1;
        auxpw1.addSetLM(domcap, *itlm1);
        PWLMap auxpw2;
        auxpw2.addSetLM(domcap, *itlm2);

        Set im1 = auxpw1.image(domcap);
        Set im2 = auxpw2.image(domcap);

        if (im1 != im2)
          return false;

        ++itlm2;
      }

      ++itlm1;
    }

    return true;
  }

  return false;
}

PWLMap minAtomPW(AtomSet &dom, LMap &lm1, LMap &lm2)
{
  OrdCT<NI2> g1 = lm1.gain();
  OrdCT<NI2> o1 = lm1.offset();
  OrdCT<NI2>::iterator ito1 = o1.begin();
  OrdCT<NI2> g2 = lm2.gain();
  OrdCT<NI2>::iterator itg2 = g2.begin();
  OrdCT<NI2> o2 = lm2.offset();
  OrdCT<NI2>::iterator ito2 = o2.begin();
  OrdCT<Interval> ints = dom.aset_ref().inters();
  OrdCT<Interval>::iterator itints = ints.begin();

  AtomSet asAux = dom;
  LMap lmAux = lm1;
  OrdCT<NI2> resg = g1;
  OrdCT<NI2> reso = o1;
  int count = 1;

  OrdCT<Set> domRes;
  OrdCT<LMap> lmRes;

  if (lm1.ndim() == lm2.ndim()) {
    BOOST_FOREACH (NI2 g1i, g1) {
      if (g1i != *itg2) {
        NI2 xinter = (*ito2 - *ito1) / (g1i - *itg2);

        // Intersection before domain
        if (xinter <= (*itints).lo()) {
          if (*itg2 < g1i) lmAux = lm2;

          Set sAux;
          sAux.addAtomSet(asAux);

          domRes.insert(domRes.begin(), sAux);
          lmRes.insert(lmRes.begin(), lmAux);
        }

        // Intersection after domain
        else if (xinter >= (*itints).hi()) {
          if (*itg2 > g1i) lmAux = lm2;

          Set sAux;
          sAux.addAtomSet(asAux);

          domRes.insert(domRes.begin(), sAux);
          lmRes.insert(lmRes.begin(), lmAux);
        }

        // Intersection in domain
        else {
          Interval i1((*itints).lo(), (*itints).step(), floor(xinter));
          Interval i2(i1.hi() + i1.step(), (*itints).step(), (*itints).hi());

          AtomSet as1 = asAux.replace(i1, count);
          AtomSet as2 = asAux.replace(i2, count);

          Set d1;
          d1.addAtomSet(as1);

          Set d2;
          d2.addAtomSet(as2);

          domRes.insert(domRes.end(), d1);
          domRes.insert(domRes.end(), d2);

          if (g1i > *itg2) {
            lmRes.insert(lmRes.end(), lm1);
            lmRes.insert(lmRes.end(), lm2);
          }

          else {
            lmRes.insert(lmRes.end(), lm2);
            lmRes.insert(lmRes.end(), lm1);
          }
        }

        PWLMap auxRes(domRes, lmRes);
        return auxRes;
      }

      else if (*ito1 != *ito2) {
        if (*ito2 < *ito1) lmAux = lm2;

        Set sAux;
        sAux.addAtomSet(asAux);
        domRes.insert(domRes.begin(), sAux);
        lmRes.insert(lmRes.begin(), lmAux);

        PWLMap auxRes(domRes, lmRes);
        return auxRes;
      }

      ++ito1;
      ++itg2;
      ++ito2;
      ++itints;
      ++count;
    }
  }

  Set sAux;
  sAux.addAtomSet(dom);
  domRes.insert(domRes.begin(), sAux);
  lmRes.insert(lmRes.begin(), lm1);
  PWLMap auxRes(domRes, lmRes);
  return auxRes;
}

PWLMap minPW(Set &dom, LMap &lm1, LMap &lm2)
{
  OrdCT<Set> sres;
  OrdCT<LMap> lres;

  Set sres1;
  Set sres2;
  LMap lres1;
  LMap lres2;

  UnordCT<AtomSet> asets = dom.asets();
  UnordCT<AtomSet>::iterator itas = asets.begin();

  if (!dom.empty()) {
    PWLMap aux;
    AtomSet asAux = *itas;
    aux = minAtomPW(asAux, lm1, lm2);
    if (!aux.empty()) {
      sres1 = *(aux.dom_().begin());
      lres1 = *(aux.lmap_().begin());
      ++itas;

      OrdCT<Set> d;
      OrdCT<Set>::iterator itd;
      OrdCT<LMap> l;
      OrdCT<LMap>::iterator itl;
      while (itas != asets.end()) {
        asAux = *itas;
        aux = minAtomPW(asAux, lm1, lm2);
        d = aux.dom_();
        itd = d.begin();
        l = aux.lmap_();
        itl = l.begin();

        while (itd != d.end()) {
          if (*itl == lres1)
            sres1 = sres1.cup(*itd);

          else {
            if (sres2.empty()) {
              sres2 = *itd;
              lres2 = *itl;
            }

            else
              sres2 = sres2.cup(*itd);
          }

          ++itd;
          ++itl;
        }

        ++itas;
      }
    }
  }

  if (!sres1.empty() && !lres1.empty()) {
    sres.insert(sres.end(), sres1);
    lres.insert(lres.end(), lres1);
  }

  if (!sres2.empty() && !lres2.empty()) {
    sres.insert(sres.end(), sres2);
    lres.insert(lres.end(), lres2);
  }

  PWLMap res(sres, lres);
  return res;
}

PWLMap minMap(PWLMap &pw1, PWLMap &pw2)
{
  PWLMap res;

  OrdCT<LMap> lm1 = pw1.lmap_();
  OrdCT<LMap>::iterator itl1 = lm1.begin();
  OrdCT<LMap> lm2 = pw2.lmap_();

  if (!pw1.empty() && !pw2.empty()) {
    BOOST_FOREACH (Set s1i, pw1.dom_()) {
      OrdCT<LMap>::iterator itl2 = lm2.begin();

      BOOST_FOREACH (Set s2j, pw2.dom_()) {
        Set dom = s1i.cap(s2j);

        if (!dom.empty()) {
          PWLMap aux = minPW(dom, *itl1, *itl2);

          if (res.empty())
            res = aux;

          else
            res = aux.combine(res);
        }

        ++itl2;
      }

      ++itl1;
    }
  }

  return res;
}

PWLMap minInv(PWLMap &pw, Set &s)
{
  if (!pw.empty()) {
    // Initialization
    OrdCT<Set> auxdom = pw.dom_();
    OrdCT<Set>::iterator itdom = auxdom.begin();
    OrdCT<LMap> auxlm = pw.lmap_();
    OrdCT<LMap>::iterator itlm = auxlm.begin();

    Set dom1 = *itdom;
    ++itdom;
    LMap lm1 = *itlm;
    ++itlm;

    OrdCT<Set> pw2dom;
    pw2dom.insert(pw2dom.begin(), dom1);
    OrdCT<LMap> pw2lm;
    pw2lm.insert(pw2lm.begin(), lm1);
    PWLMap pw2(pw2dom, pw2lm);

    PWLMap invpw2 = pw2.minInvCompact(s);
    pw2 = invpw2;

    for (; itdom != auxdom.end(); ++itdom) {
      OrdCT<Set> pwidom;
      pwidom.insert(pwidom.begin(), *itdom);
      OrdCT<LMap> pwilm;
      pwilm.insert(pwilm.begin(), *itlm);
      PWLMap pwi(pwidom, pwilm);

      PWLMap invpwi = pwi.minInvCompact(s);

      PWLMap minmap = minMap(pw2, invpwi);
      PWLMap combmap = invpwi.combine(pw2);
      pw2 = combmap;

      if (!minmap.empty()) {
        PWLMap combmap2 = minmap.combine(pw2);
        pw2 = combmap2;
      }

      ++itlm;
    }

    return pw2;
  }

  PWLMap res;
  return res;
}

PWLMap reduceMapN(PWLMap pw, int dim)
{
  OrdCT<Set> sres = pw.dom_();
  OrdCT<Set>::iterator itsres = sres.end();
  OrdCT<LMap> lres = pw.lmap_();
  OrdCT<LMap>::iterator itlres = lres.end();

  OrdCT<LMap> lm = pw.lmap_();
  OrdCT<LMap>::iterator itlm = lm.begin();

  unsigned int i = 1;
  BOOST_FOREACH (Set di, pw.dom_()) {
    int count1 = 1;

    OrdCT<NI2> g = (*itlm).gain();
    OrdCT<NI2>::iterator itg = g.begin();
    OrdCT<NI2> o = (*itlm).offset();
    OrdCT<NI2>::iterator ito = o.begin();
    // Get the dim-th gain and offset
    while (count1 < dim) {
      ++itg;
      ++ito;
      ++count1;
    }

    if (*itg == 1 && *ito < 0) {
      NI2 off = -(*ito);

      BOOST_FOREACH (AtomSet adom, di.asets()) {
        MultiInterval mi = adom.aset();
        OrdCT<Interval> inters = mi.inters();
        OrdCT<Interval>::iterator itints = inters.begin();

        int count2 = 1;
        while (count2 < dim) {
          ++itints;
          ++count2;
        }

        NI1 loint = (*itints).lo();
        NI1 hiint = (*itints).hi();

        if ((hiint - loint) > (off * off)) {
          OrdCT<Set> news;
          OrdCT<Set>::iterator itnews = news.begin();
          OrdCT<LMap> newl;
          OrdCT<LMap>::iterator itnewl = newl.begin();

          for (int k = 1; k <= off; k++) {
            OrdCT<NI2> newo = (*itlm).offset();
            OrdCT<NI2>::iterator itnewo = newo.begin();

            OrdCT<NI2> resg;
            OrdCT<NI2>::iterator itresg = resg.begin();
            OrdCT<NI2> reso;
            OrdCT<NI2>::iterator itreso = reso.begin();

            int count3 = 1;
            BOOST_FOREACH (NI2 gi, (*itlm).gain()) {
              if (count3 == dim) {
                itresg = resg.insert(itresg, 0);
                itreso = reso.insert(itreso, loint + k - off - 1);
              }

              else {
                itresg = resg.insert(itresg, gi);
                itreso = reso.insert(itreso, *itnewo);
              }

              ++itresg;
              ++itreso;
              ++itnewo;
              ++count3;
            }

            LMap newlmap(resg, reso);
            Interval newinter(loint + k - 1, off, hiint);
            AtomSet auxas = adom.replace(newinter, dim);
            Set newset;
            newset.addAtomSet(auxas);

            itnews = news.insert(itnews, newset);
            ++itnews;
            itnewl = newl.insert(itnewl, newlmap);
            ++itnewl;
          }

          PWLMap newmap(news, newl);

          UnordCT<AtomSet> auxnewd;
          BOOST_FOREACH (AtomSet auxasi, di.asets()) {
            if (auxasi != adom) auxnewd.insert(auxasi);
          }

          Set newdomi(auxnewd);

          if (newdomi.empty()) {
            itlres = lres.begin();

            if (i < sres.size()) {
              OrdCT<Set> auxs;
              OrdCT<Set>::iterator itauxs = auxs.begin();
              OrdCT<LMap> auxl;
              OrdCT<LMap>::iterator itauxl = auxl.begin();

              unsigned int count4 = 1;
              BOOST_FOREACH (Set si, sres) {
                if (count4 != i) {
                  itauxs = auxs.insert(itauxs, si);
                  ++itauxs;
                  itauxl = auxl.insert(itauxl, *itlres);
                  ++itauxl;
                }

                ++count4;
                ++itlres;
              }

              sres = auxs;
              lres = auxl;
            }

            else {
              OrdCT<Set> auxs;
              OrdCT<Set>::iterator itauxs = auxs.begin();
              OrdCT<LMap> auxl;
              OrdCT<LMap>::iterator itauxl = auxl.begin();

              unsigned int count4 = 1;
              BOOST_FOREACH (Set si, sres) {
                if (count4 < i) {
                  itauxs = auxs.insert(itauxs, si);
                  ++itauxs;
                  itauxl = auxl.insert(itauxl, *itlres);
                  ++itauxl;
                }

                ++count4;
                ++itlres;
              }

              sres = auxs;
              lres = auxl;
            }
          }

          else {
            OrdCT<Set> auxs;
            OrdCT<Set>::iterator itauxs = auxs.begin();
            OrdCT<Set>::iterator itauxsres = sres.begin();
            unsigned int count5 = 1;
            while (itauxsres != sres.end()) {
              if (count5 == i)
                itauxs = auxs.insert(itauxs, newdomi);

              else
                itauxs = auxs.insert(itauxs, *itauxsres);

              ++itauxs;
              ++itauxsres;
              ++count5;
            }

            sres = auxs;
          }

          BOOST_FOREACH (Set newi, newmap.dom_()) {
            itsres = sres.insert(itsres, newi);
            ++itsres;
          }

          BOOST_FOREACH (LMap newi, newmap.lmap_()) {
            itlres = lres.insert(itlres, newi);
            ++itlres;
          }
        }
      }
    }

    ++itlm;
    ++i;
  }

  PWLMap res(sres, lres);
  return res;
}

PWLMap mapInf(PWLMap pw)
{
  PWLMap res;
  if (!pw.empty()) {
    res = reduceMapN(pw, 1);

    for (int i = 2; i <= res.ndim_(); ++i) res = reduceMapN(res, i);

    int maxit = 0;

    OrdCT<Set> doms = res.dom_();
    OrdCT<Set>::iterator itdoms = doms.begin();
    BOOST_FOREACH (LMap lm, res.lmap_()) {
      OrdCT<NI2> o = lm.offset();
      OrdCT<NI2>::iterator ito = o.begin();

      NI2 a = 0;
      NI2 b = *(lm.gain().begin());

      BOOST_FOREACH (NI2 gi, lm.gain()) {
        a = max(a, gi * abs(*ito));
        b = min(b, gi);

        ++ito;
      }

      ito = o.begin();
      if (a > 0) {
        NI2 its = 0;

        OrdCT<NI2> g = lm.gain();
        OrdCT<NI2>::iterator itg = g.begin();
        // For intervals in which size <= off ^ 2 (check reduceMapN, this intervals are not "reduced")
        for (int dim = 0; dim < res.ndim_(); ++dim) {
          if (*itg == 1 && *ito != 0) {
            BOOST_FOREACH (AtomSet asi, (*itdoms).asets()) {
              MultiInterval mii = asi.aset();
              OrdCT<Interval> ii = mii.inters();
              OrdCT<Interval>::iterator itii = ii.begin();

              for (int count = 0; count < dim; ++count) ++itii;

              its = max(its, ceil(((*itii).hi() - (*itii).lo()) / abs(*ito)));
            }
          }

          // else
          //  ++maxit;

          ++itg;
          ++ito;
        }

        maxit += its;
      }

      else if (b == 0)
        ++maxit;

      ++itdoms;
    }

    if (maxit == 0)
      return res;

    else
      maxit = floor(log2(maxit)) + 1;

    for (int j = 0; j < maxit; ++j) res = res.compPW(res);
  }

  return res;
}

// pw3, pw2, pw1 are such that:
//   pw3 : A -> B, and is a compact map
//   pw2 : A -> C
//   pw1 : C -> D
PWLMap minAdjCompMap(PWLMap pw3, PWLMap pw2, PWLMap pw1)
{
  PWLMap res;

  OrdCT<Set> auxd = pw3.dom_();
  int auxsize = auxd.size();
  if (auxsize == 1) {
    Set dom = *(pw3.dom_().begin());
    LMap lm = *(pw3.lmap_().begin());

    Set dominv = pw3.image(dom);
    LMap lminv = lm.invLMap();
    OrdCT<NI2> lminvo = lminv.offset();

    PWLMap invpw;
    invpw.addSetLM(dominv, lminv);

    NI2 maxg = *(lminv.gain().begin());
    NI2 ming = maxg;
    BOOST_FOREACH (NI2 gi, lminv.gain()) {
      maxg = max(maxg, gi);
      ming = min(ming, gi);
    }

    // Bijective map, therefore, it's invertible
    if (maxg < Inf) {
      res = pw2.compPW(invpw);
    }

    // Constant map
    else if (ming == Inf) {
      if (!pw3.empty()) {
        Set im2 = pw2.image(dom);
        Set compim12 = pw1.image(im2);

        // Get vertices in image of pw2 with minimum image in pw1
        OrdCT<NI1> mincomp = compim12.minElem();
        MultiInterval micomp;
        BOOST_FOREACH (NI1 mincompi, mincomp) {
          Interval i(mincompi, 1, mincompi);
          micomp.addInter(i);
        }
        AtomSet ascomp(micomp);
        Set scomp;
        scomp.addAtomSet(ascomp);
        Set mins2 = pw1.preImage(scomp);

        // Choose minimum in mins2, and assign dom(pw1) this element as image
        OrdCT<NI1> min2 = mins2.minElem();
        OrdCT<NI1>::iterator itmin2 = min2.begin();

        OrdCT<NI2> reso;
        OrdCT<NI2>::iterator itreso = reso.begin();
        OrdCT<NI2> resg;
        OrdCT<NI2>::iterator itresg = resg.begin();
        for (int i = 0; i < dominv.ndim(); ++i) {
          itresg = resg.insert(itresg, 0);
          ++itresg;
          itreso = reso.insert(itreso, (NI2)(*itmin2));
          ++itreso;

          ++itmin2;
        }

        res.addSetLM(dominv, LMap(resg, reso));
      }
    }

    // Bijective in some dimensions, and constant in others
    else {
      OrdCT<NI2>::iterator itinvo = lminvo.begin();
      OrdCT<NI2> newinvg;
      OrdCT<NI2>::iterator itnewinvg = newinvg.begin();
      OrdCT<NI2> newinvo;
      OrdCT<NI2>::iterator itnewinvo = newinvo.begin();
      OrdCT<NI1> mindom = dom.minElem();
      OrdCT<NI1>::iterator itmindom = mindom.begin();

      // Replace inverse of constant maps for composition (with a dummy value)
      BOOST_FOREACH (NI2 invgi, lminv.gain()) {
        if (invgi >= Inf) {
          itnewinvg = newinvg.insert(itnewinvg, 0);
          itnewinvo = newinvo.insert(itnewinvo, (NI2) (*itmindom));
        }

        else {
          itnewinvg = newinvg.insert(itnewinvg, invgi);
          itnewinvo = newinvo.insert(itnewinvo, *itinvo);
        }

        ++itinvo;
        ++itnewinvg;
        ++itnewinvo;
        ++itmindom;
      }

      LMap newinvlm(newinvg, newinvo);
      PWLMap newinvpw;
      newinvpw.addSetLM(dominv, newinvlm);

      // Compose
      PWLMap auxres = pw2.compPW(newinvpw);

      OrdCT<Set> domaux = auxres.dom_();
      OrdCT<LMap> lmaux = auxres.lmap_();
      OrdCT<LMap>::iterator itlmaux = lmaux.begin();

      OrdCT<LMap> lmres;
      OrdCT<LMap>::iterator itlmres = lmres.begin();

      // Replace values of constant maps with the desired value
      Set im2 = pw2.image(dom);
      Set compim12 = pw1.image(im2);

      // Get vertices in image of pw2 with minimum image in pw1
      OrdCT<NI1> mincomp = compim12.minElem();
      MultiInterval micomp;
      BOOST_FOREACH (NI1 mincompi, mincomp) {
        Interval i(mincompi, 1, mincompi);
        micomp.addInter(i);
      }
      AtomSet ascomp(micomp);
      Set scomp;
      scomp.addAtomSet(ascomp);
      Set mins2 = pw1.preImage(scomp);

      // Choose minimum in min2
      // Assign dom(pw1) this element as image
      OrdCT<NI1> min2 = mins2.minElem();
      OrdCT<NI1>::iterator itmin2 = min2.begin();

      BOOST_FOREACH (Set domauxi, domaux) {
        OrdCT<NI2> replaceg;
        OrdCT<NI2>::iterator itrepg = replaceg.begin();
        OrdCT<NI2> replaceo;
        OrdCT<NI2>::iterator itrepo = replaceo.begin();

        OrdCT<NI2> auxg = (*itlmaux).gain();
        OrdCT<NI2>::iterator itauxg = auxg.begin();
        OrdCT<NI2> auxo = (*itlmaux).offset();
        OrdCT<NI2>::iterator itauxo = auxo.begin();

        BOOST_FOREACH (NI2 invgi, lminv.gain()) {
          // i-th dimension constant
          if (invgi >= Inf) {
            itrepg = replaceg.insert(itrepg, 0);
            itrepo = replaceo.insert(itrepo, (NI2)(*itmin2));
          }

          // i-th dimension bijective
          else {
            itrepg = replaceg.insert(itrepg, *itauxg);
            itrepo = replaceo.insert(itrepo, *itauxo);
          }

          ++itauxg;
          ++itauxo;
          ++itrepg;
          ++itrepo;
          ++itmin2;
        }

        itlmres = lmres.insert(itlmres, LMap(replaceg, replaceo));
        LMap den(replaceg, replaceo);

        ++itlmaux;
        ++itlmres;
      }

      PWLMap replacedpw(auxres.dom_(), lmres);
      res = replacedpw;
    }
  }

  return res;
}

// pw2, pw1 are such that:
//   pw2 : A -> B, and is a compact map
//   pw1 : A -> C
PWLMap minAdjCompMap(PWLMap pw2, PWLMap pw1)
{
  Set dom1 = pw1.wholeDom();
  Set im1 = pw1.image(dom1);
  PWLMap idmap(im1);

  return minAdjCompMap(pw2, pw1, idmap);
}

PWLMap minAdjMap(PWLMap pw3, PWLMap pw2, PWLMap pw1)
{
  PWLMap res;

  if (!pw3.empty()) {
    OrdCT<Set> dom = pw3.dom_();
    OrdCT<Set>::iterator itdom = dom.begin();
    OrdCT<LMap> lm = pw3.lmap_();
    OrdCT<LMap>::iterator itlm = lm.begin();

    Set auxdom = *itdom;
    LMap auxlm = *itlm;

    PWLMap map31;
    map31.addSetLM(auxdom, auxlm);

    res = minAdjCompMap(map31, pw2, pw1);
    ++itdom;
    ++itlm;

    PWLMap minAdj;
    PWLMap minM;
    while (itdom != dom.end()) {
      PWLMap map3i;
      map3i.addSetLM(*itdom, *itlm);
      minAdj = minAdjCompMap(map3i, pw2, pw1);
      minM = minMap(res, minAdj);

      res = minAdj.combine(res);

      if (!minM.empty()) res = minM.combine(res);

      ++itdom;
      ++itlm;
    }
  }

  return res;
}

PWLMap minAdjMap(PWLMap pw2, PWLMap pw1)
{
  /*
  BOOST_FOREACH(Set domi, pw1.dom_()) {
    PWLMap pwi(domi);
    idmap = idmap.concat(pwi);
  }
  */

  Set dom1 = pw1.wholeDom();
  Set im1 = pw1.image(dom1);
  PWLMap idmap(im1);

  return minAdjMap(pw2, pw1, idmap);
}

SetEdge restrictEdge(SetEdge e, Set dom)
{
  PWLMap es1 = e.es1_();
  PWLMap es2 = e.es2_();

  PWLMap res1 = es1.restrictMap(dom);
  PWLMap res2 = es2.restrictMap(dom);

  SetEdge res(e.name, e.id_(), res1, res2, 0);
  return res;
}

}  // namespace SBG
