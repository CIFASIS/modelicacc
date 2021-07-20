/*****************************************************************************
 
    This file is part of the Set-Based Graph library.

******************************************************************************/

#include <boost/foreach.hpp>

#include <util/graph/sbg/defs.h>
#include <util/graph/sbg/multi_interval.h>

namespace SBG {

// MultiIntervals ---------------------------------------------------------------------------------

#define INTERS_TYPE                  \
   typename MI_TEMP_TYPE::Intervals

MI_TEMPLATE
MI_TEMP_TYPE::MultiInterImp1() : ndim_(0)
{
  Intervals emptyInters;
  inters_ = emptyInters;
};

MI_TEMPLATE
MI_TEMP_TYPE::MultiInterImp1(INTERS_TYPE inters)
{
  bool areEmptyInters = false;

  BOOST_FOREACH (INTER_IMP i, inters)
   if (i.empty()) areEmptyInters = true;

  if (areEmptyInters) {
    Intervals emptyInters;
    inters_ = emptyInters;
    ndim_ = 0;
  }

  else {
    inters_ = inters;
    ndim_ = inters.size();
  }
}

member_imp_temp(MI_TEMPLATE, MI_TEMP_TYPE, INTERS_TYPE, inters);
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
bool MI_TEMP_TYPE::isIn(ORD_CT<INT_IMP> elem)
{
  IntervalsIt it2 = inters_ref().begin();

  if ((int)elem.size() != ndim()) return false;

  BOOST_FOREACH (INT_IMP n, elem) {
    if (!(*it2).isIn(n)) return false;

    ++it2;
  }

  return true;
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
ORD_CT<INT_IMP> MI_TEMP_TYPE::minElem()
{
    ORD_CT<INT_IMP> res;
    typename ORD_CT<INT_IMP>::iterator itres = res.begin();

    BOOST_FOREACH (INTER_IMP i, inters()) {
      if (i.empty()) {
        ORD_CT<INT_IMP> aux;
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

template struct MultiInterImp1<OrdCT, UnordCT, Interval, INT>;

MI_TEMPLATE
std::ostream &operator<<(std::ostream &out, const MI_TEMP_TYPE &mi)
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

template std::ostream &operator<<(std::ostream &out, const MultiInterval &mi);

size_t hash_value(const MultiInterval &mi) { 
  MultiInterval aux = mi;
  return aux.hash(); 
}

} // namespace SBG
