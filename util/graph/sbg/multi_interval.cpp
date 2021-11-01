/*****************************************************************************
 
    This file is part of the Set-Based Graph library.

******************************************************************************/

#include <boost/foreach.hpp>

#include <util/graph/sbg/multi_interval.h>

namespace SBG {

// MultiIntervals ---------------------------------------------------------------------------------

#define INTERS_TYPE                  \
   typename MI_TEMP_TYPE::Intervals

MI_TEMPLATE
MI_TEMP_TYPE::MultiInterImp1() : ndim_(0), inters_() {}

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

  BOOST_FOREACH (INTER_IMP i, inters()) 
    res *= i.card();

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

      if (capres.empty()) 
        return MultiInterImp1();

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
  MultiInterImp1 capmis = cap(mi2);

  UNORD_CT<MultiInterImp1> resmis;

  // First multi-interval is empty
  if (inters_ref().empty()) return resmis;

  if (ndim() != mi2.ndim()) return resmis;

  // Empty intersection
  if (capmis.empty()) {
    resmis.insert(*this);
    return resmis;
  }

  // Multi-intervals are the same
  if (inters() == capmis.inters()) return resmis;

  // Differences of each dimension
  IntervalsIt itcap = capmis.inters_ref().begin();
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
  // Traverse dimensions
  BOOST_FOREACH (UNORD_CT<INTER_IMP> nthdiff, diffs) {
    // Intervals in the difference of nth dimension
    BOOST_FOREACH (INTER_IMP ith, nthdiff) {
      if (!ith.empty()) {
        Intervals resi;
        IntervalsIt itresi = resi.begin();

        itcap = capmis.inters_ref().begin();

        // Complete with intersection before nth dimension (so the MIs in the result are disjoint)
        if (count > 0) {
          for (int j = 0; j < count; j++) {
            itresi = resi.insert(itresi, *itcap);
            ++itresi;

            ++itcap;
          }
        }

        // Complete nth dimension with interval in the difference
        itresi = resi.insert(itresi, ith);
        ++itresi;

        // Complete the rest of dimensions with intervals of the first argument
        IntervalsIt auxit1 = it1;
        while (auxit1 != inters_ref().end()) {
          itresi = resi.insert(itresi, *auxit1);
          ++itresi;

          ++auxit1;
        }

        resmis.insert(MultiInterImp1(resi));
      }
    }

    ++count;
    ++it1;
  }

  return resmis;
}

MI_TEMPLATE
ORD_CT<INT_IMP> MI_TEMP_TYPE::minElem()
{
  ORD_CT<INT_IMP> res;
  typename ORD_CT<INT_IMP>::iterator itres = res.begin();

  BOOST_FOREACH (INTER_IMP i, inters()) {
    if (i.empty()) return ORD_CT<INT_IMP>();

    itres = res.insert(itres, i.minElem());
    ++itres;
  }

  return res;
}

MI_TEMPLATE
ORD_CT<INT_IMP> MI_TEMP_TYPE::maxElem()
{
  ORD_CT<INT_IMP> res;
  typename ORD_CT<INT_IMP>::iterator itres = res.begin();

  BOOST_FOREACH (INTER_IMP i, inters()) {
    if (i.empty()) return ORD_CT<INT_IMP>();

    itres = res.insert(itres, i.maxElem());
    ++itres;
  }

  return res;
}

// Two multi-intervals can be normalized only if their intervals are the
// same for each dimension, except for one, in which their respective intervals
// can be normalized
MI_TEMPLATE
MI_TEMP_TYPE MI_TEMP_TYPE::normalize(MI_TEMP_TYPE mi2)
{
  IntervalsIt it1 = inters_ref().begin();
  IntervalsIt it2 = mi2.inters_ref().begin();

  if (ndim() == mi2.ndim()) {
    int diffCount = 0; // Count of different intervals through dimensions
    int dimDiff = 0; // Last dimension with spoted difference
    INTER_IMP i1;
    INTER_IMP i2;
    for (int j = 0; j < ndim(); j++) {
      if (*it1 != *it2) {
        ++diffCount;
        dimDiff = j + 1;
        i1 = *it1;
        i2 = *it2;
      }

      ++it1;
      ++it2;
    }

    if (diffCount == 1) {
      INTER_IMP normalized = i1.normalize(i2);

      if (!normalized.empty())
        return mi2.replace(normalized, dimDiff);
    }
  }

  return MultiInterImp1();
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
MI_TEMP_TYPE MI_TEMP_TYPE::replace(INTER_IMP i, int dim)
{
  Intervals res;
  IntervalsIt itres = res.begin();
  int count = 1;

  BOOST_FOREACH (INTER_IMP ii, inters()) {
    if (dim == count)
      itres = res.insert(itres, i);
    else
      itres = res.insert(itres, ii);

    ++itres;
    ++count;
  }

  return MultiInterImp1(res);
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

std::ostream &operator<<(std::ostream &out, const UNORD_MI &mis)
{
  UNORD_MI auxmis = mis;
  MultiInterval mi1 = *(mis.begin());

  out << "{";
  if (auxmis.size() == 1)
    out << mi1;

  else if (auxmis.size() > 1) {
    auxmis.erase(mi1);

    BOOST_FOREACH (MultiInterval mi, auxmis) 
      out << mi << ", ";
    out << mi1;
  }
  out << "}";

  return out;
}

} // namespace SBG
