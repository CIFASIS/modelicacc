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

/*
 *  Created on: 2 Aug 2016
 *      Author: Diego Hollmann
 */

#include <causalize/vector/vector_graph_definition.h>
#include <ast/expression.h>
#include <boost/variant/get.hpp>
#include <util/debug.h>
#include <stdarg.h>


namespace Causalize {

  VectorUnknown::VectorUnknown(VarInfo varInfo, Modelica::AST::Reference var): Unknown(varInfo, var) {
    if (!varInfo.indices()) {
      dimension = 0;
    } else {
      dimension = varInfo.indices().get().size();
    }
  }

  void VectorUnknown::SetIndex(Modelica::AST::ExpList index) {
    ERROR_UNLESS((int)index.size()==dimension, "Indexes size different than unknown dimension");
    if (dimension!=0) {
      if (Modelica::AST::is<Modelica::AST::Call>(expression)) {
        get<Modelica::AST::Reference>(get<Modelica::AST::Call>(expression).args_.front()).ref_.front().get<1>() = index;
      } else if (Modelica::AST::is<Modelica::AST::Reference>(expression)) {
        get<Modelica::AST::Reference>(expression).ref_.front().get<1>() = index;
      } else {
        ERROR("Wrong unknown expression type");
      }
    }
  }

  std::ostream& operator<<(std::ostream &os, const IndexPairSet &ips) {
    std::list<std::string> ipsStList;
    foreach_(IndexPair ip, ips){
      std::ostringstream ipSt;
      ipSt << ip;
      ipsStList.push_back(ipSt.str());
    }
    std::string ipsSt = "<" + boost::algorithm::join(ipsStList, ",") + ">";
    os << ipsSt;
    return os;
  }


  /*****************************************************************************
   ****                              Offset                                  ****
   *****************************************************************************/
  Offset Offset::operator-() const {
    std::vector<int> ret(offset.size());
    for(int i = 0; i<(int)offset.size(); i++) {
      ret[i] = -offset[i];
    }
    return ret;
  };
  /*****************************************************************************
   ****************************************************************************/



  /*****************************************************************************
   ****                               MDI                                   ****
   *****************************************************************************/
  MDI::MDI(int d, ...) {
    intervals.resize(d);
    va_list vl;
    va_start(vl,d);
    for (int i=0;i<d;i++) {
      int lower=va_arg(vl,int);
      int upper=va_arg(vl,int);
      intervals[i]=CreateInterval(lower, upper);
    }
    va_end(vl);
  }

  MDI::MDI(IntervalList intervalList): intervals(IntervalVector(intervalList.begin(), intervalList.end())) { }

  int MDI::Size() const{
    int size = 1;
    foreach_(Interval i, intervals) {
      size*=boost::icl::size(i);
    }
    return size;
  }

  bool MDI::operator<(const MDI& other) const {
    return this->intervals<other.intervals;
  }

  std::list<Interval> MDI::Partition(Interval iA, Interval iB) {
    std::list<Interval> ret;
    int a = iA.lower();
    int b = iA.upper();
    int c = iB.lower();
    int d = iB.upper();
    if ((a<c) && (c<d) && (d<b)) {
      ret.push_back(CreateInterval(a,c-1));
      ret.push_back(CreateInterval(c,d));
      ret.push_back(CreateInterval(d+1,b));
    } else if ((c<=a) && (a<=d) && (d<b)) {
  //      return {CreateInterval(a,d), CreateInterval(d+1,b)}; /* c++11 */
      ret.push_back(CreateInterval(a,d));
      ret.push_back(CreateInterval(d+1,b));
    } else if ((a<c) && (c<=b) && (b<=d)) {
      ret.push_back(CreateInterval(a,c-1));
      ret.push_back(CreateInterval(c,b));
    } else {
      ret.push_back(CreateInterval(a,b));
    }
    return ret;
  }

  std::list<MDI> MDI::PutHead(Interval i, std::list<MDI> mdiList) {
    std::list<MDI> mdiListRet;
    BOOST_FOREACH(MDI xs, mdiList) {
      IntervalList ys=IntervalList(xs.intervals.begin(), xs.intervals.end());
      ys.push_front(i);
      mdiListRet.push_back(ys);
    }
    return mdiListRet;
  }

  std::list<MDI> MDI::PutLists(MDI mdi, std::list<MDI> mdiList) {
    std::list<MDI> mdiListRet;
    BOOST_FOREACH(Interval i, mdi.intervals) {
      std::list<MDI> zss = PutHead(i, mdiList);
      BOOST_FOREACH(MDI zs, zss) {
        mdiListRet.push_back(zs);
      }
    }
    return mdiListRet;
  }

  std::ostream& operator<<(std::ostream &os, const MDI mdi) {
    std::list<std::string> xsStList;
    BOOST_FOREACH(Interval x, mdi.intervals) {
      std::stringstream ss;
      ss << "["<< x.lower() << "," << x.upper() << "]";
      xsStList.push_back(ss.str());
    }
    os << "<" << boost::algorithm::join(xsStList, ",") << ">";
    return os;
  }

  std::list<MDI> MDI::CartProd(std::list<MDI> xss) {
    std::list<MDI> yss;
    if (xss.size()==0) return yss;
    else if (xss.size()==1) {
      IntervalVector xs = xss.front().intervals;
      BOOST_FOREACH(Interval i, xs) {
        IntervalList ys;
        ys.push_back(i);
        yss.push_back(ys);
      }
      return yss;
    } else {
      std::list<MDI> zss = xss;
      zss.pop_front();
      return PutLists(xss.front(), CartProd(zss));
    }
  }

  std::list<MDI> MDI::Filter(std::list<MDI> mdiList, MDI mdi) {
    std::list<MDI> mdiListRet;
    BOOST_FOREACH(MDI m, mdiList) {
      ERROR_UNLESS(m.Dimension()==mdi.Dimension(), "Dimension error");
      if (m.Dimension()!=mdi.Dimension()) {
        std::cout << "Dimension error\n";
        abort();
      }
      MDI::iterator iterXS = m.begin();
      MDI::iterator iterYS = mdi.begin();
      bool hasInter = true;
      for(int i=0; i<(int)m.Dimension(); i++) {
        hasInter&= intersects(*iterXS,*iterYS);
        iterXS++;
        iterYS++;
      }
      if (!hasInter) {
        mdiListRet.push_back(m);
      }
    }
    return mdiListRet;
  }

//  MDI MDI::ApplyOffset(Offset offset) {
//    IntervalVector copyIntervals = intervals;
//    for(int i=0; i<(int)copyIntervals.size(); i++) {
//      copyIntervals[i] = CreateInterval(copyIntervals[i].lower()+offset[i],copyIntervals[i].upper()+offset[i]);
//    }
//    return MDI(copyIntervals);
//  }

  MDI MDI::ApplyUsage(std::vector<int> usage) {
    IntervalVector newIntervals;
    newIntervals.resize(intervals.size());
    int usages = 0;
    for(int i: usage) {
      if (i>=0) {
        newIntervals[i] = CreateInterval(intervals[i].lower(), intervals[i].upper());
        usages++;
      }
    }
    newIntervals.resize(usages);
    return MDI(newIntervals);
  }

  std::list<MDI> MDI::operator-(const MDI &other) {
    if (this->Dimension()!=other.Dimension()) {
      std::cout << "Dimension error\n";
      abort();
    }
    std::list<MDI> ret;
    MDI::iterator iterA = this->begin();
    MDI::const_iterator iterB = other.begin();
    std::list<MDI> prod;
    for(int i=0; i<this->Dimension(); i++) {
      prod.push_back(Partition(*iterA,*iterB));
      iterA++;
      iterB++;
    }
    ret = CartProd(prod);
    return Filter(ret, other);
  }

  std::list<MDI> MDI::Remove(const MDI &other, Offset offset) {
    if (this->Dimension()!=other.Dimension()) {
      std::cout << "Dimension error\n";
      abort();
    }
    std::list<MDI> ret;
    MDI::iterator iterA = this->begin();
    MDI::const_iterator iterB = other.begin();
    std::list<MDI> prod;
    for(int i=0; i<this->Dimension(); i++) {
      prod.push_back(Partition(*iterA,*iterB));
      iterA++;
      iterB++;
    }
    ret = CartProd(prod);
    return Filter(ret, other);
  }

  Option<MDI> MDI::operator&(const MDI &other) const {
    if (this->Dimension() != other.Dimension()) { //TODO: Is this condition OK?
      ERROR("Dimension error\n");
    }
    IntervalList intersection;
    for(int i=0; i<this->Dimension(); i++) {
      //If i-th interval does not intersect with its corresponding interval in the other MDI: return an empty MDI
      if (!intersects(this->intervals[i],other.intervals[i])) return Option<MDI>();
      else intersection.push_back((this->intervals[i])&(other.intervals[i]));
    }
    //All intervals intersect with its corresponding interval in the other MDI: return the resulting intersection MDI
    return MDI(intersection);
  }
  /*****************************************************************************
   ****************************************************************************/



  /*****************************************************************************
   ****                            INDEX PAIR                               ****
   *****************************************************************************/
  std::list<IndexPair> IndexPair::operator-(const IndexPair& other) {
    ERROR_UNLESS((this->Dom().Dimension()==other.Dom().Dimension()) &&
        (this->Ran().Dimension()==other.Ran().Dimension()), "Dimension error\n");
    std::list<MDI> remainsDom = this->Dom()-other.Dom();
    std::list<MDI> remainsRan = this->Ran()-other.Ran();
    std::list<MDI>::iterator domIter = remainsDom.begin();
    std::list<MDI>::iterator ranIter = remainsRan.begin();
    std::list<IndexPair> ret;
    while (domIter!=remainsDom.end()) {
      ret.push_back(IndexPair(*domIter,*ranIter,this->offset));
      domIter++;
      ranIter++;
    }
    return ret;
  }

  std::list<IndexPair> IndexPair::RemoveUnknowns(MDI mdi) {
    if (Option<MDI> ranToRemove = mdi&this->Ran()) {
      ranToRemove = ranToRemove.get().ApplyUsage(this->usage);
      std::list<MDI> remainsRan = this->Dom().Remove(ranToRemove.get(), offset);
      std::list<MDI> remainsDom = this->Ran()-ranToRemove.get();
      std::list<MDI>::iterator domIter = remainsDom.begin();
      std::list<MDI>::iterator ranIter = remainsRan.begin();
      std::list<IndexPair> ret;
      while (domIter!=remainsDom.end()) {
        ret.push_back(IndexPair(*domIter,*ranIter,this->offset));
        domIter++;
        ranIter++;
      }
      return ret;
    }
    else return std::list<IndexPair>({*this});
  }

  std::list<IndexPair> IndexPair::RemoveEquations(MDI mdi) {
    if (Option<MDI> domToRemove = mdi&this->Ran()) {
      domToRemove = domToRemove.get().ApplyUsage(this->usage);
      std::list<MDI> remainsRan = this->Ran().Remove(domToRemove.get(), -offset);
      std::list<MDI> remainsDom = this->Dom()-domToRemove.get();
      std::list<MDI>::iterator domIter = remainsDom.begin();
      std::list<MDI>::iterator ranIter = remainsRan.begin();
      std::list<IndexPair> ret;
      while (domIter!=remainsDom.end()) {
        ret.push_back(IndexPair(*domIter,*ranIter,this->offset));
        domIter++;
        ranIter++;
      }
      return ret;
    }
    else return std::list<IndexPair>({*this});
  }

  bool IndexPair::operator<(const IndexPair& other) const {
    return this->Dom() < other.Dom() && this->Ran() < other.Ran() && this->OS() < other.OS();
  }

  std::ostream& operator<<(std::ostream &os, const IndexPair &ip) {
    os << "(Eq=(" << ip.Dom() << "), Unk=(" << ip.Ran() << "))";
    return os;
  }
  /*****************************************************************************
   ****************************************************************************/



  /*****************************************************************************
   ****                              LABEL                                  ****
   *****************************************************************************/
  void Label::RemovePairs(IndexPairSet ips) {
    foreach_(IndexPair ipRemove, ips) {
      IndexPairSet newIps = this->ips;
      foreach_(IndexPair ip, this->ips) {
        if ((ip.Dom()&ipRemove.Dom()) && (ip.Ran()&ipRemove.Ran()) && (ip.OS())==(ipRemove.OS())) {
          newIps.erase(ip);
          foreach_(IndexPair ipRemaining, (ip-ipRemove)) {
            this->ips.insert(ipRemaining);
          }
        }
      }
      this->ips = newIps;
    }
  }

  void Label::RemoveUnknowns(MDI const mdi) const {
    IndexPairSet newIps = this->ips;
    for(IndexPair ipOld: ips) {
      for(IndexPair ipNew: ipOld.RemoveUnknowns(mdi)) {
        newIps.insert(ipNew);
      }
    }
  }

  void Label::RemoveEquations(MDI const mdi) const {
    IndexPairSet newIps = this->ips;
    for(IndexPair ipOld: ips) {
      for(IndexPair ipNew: ipOld.RemoveEquations(mdi)) {
        newIps.insert(ipNew);
      }
    }
  }


  std::ostream& operator<<(std::ostream &os, const Label &label) {
    os << label.ips;
    return os;
  }
  /*****************************************************************************
   ****************************************************************************/


  unsigned long int EdgeCount(IndexPairSet labels) {
    unsigned long int count = 0;
    /*foreach_(IndexPair ip, labels) {
      unsigned long int eq_count = IntervalCount(get<0>(ip).first);
      unsigned long int unk_count = IntervalCount(get<1>(ip).first);
      count += (eq_count > unk_count ? eq_count : unk_count);
    //unsigned long int eq_ount = 
      
    }*/
    return count;
  }


}
