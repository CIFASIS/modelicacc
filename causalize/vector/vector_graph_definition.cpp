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
    std::string ipsSt = "{" + boost::algorithm::join(ipsStList, ",") + "}";
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
    return (this->intervals)<(other.intervals);
  }

  std::list<Interval> MDI::Partition(Interval iA, Interval iB) {
    std::list<Interval> ret;
    int a = iA.lower();
    int b = iA.upper();
    int c = iB.lower();
    int d = iB.upper();
    if ((a<c) && (d<b)) {
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
    for(MDI xs: mdiList) {
      IntervalList ys=IntervalList(xs.intervals.begin(), xs.intervals.end());
      ys.push_front(i);
      mdiListRet.push_back(ys);
    }
    return mdiListRet;
  }

  std::list<MDI> MDI::PutLists(MDI mdi, std::list<MDI> mdiList) {
    std::list<MDI> mdiListRet;
    for(Interval i: mdi.intervals) {
      std::list<MDI> zss = PutHead(i, mdiList);
      for(MDI zs: zss) {
        mdiListRet.push_back(zs);
      }
    }
    return mdiListRet;
  }

  std::ostream& operator<<(std::ostream &os, const MDI mdi) {
    std::list<std::string> xsStList;
    for(Interval x: mdi.intervals) {
      std::stringstream ss;
      if ( x.lower()== x.upper()) 
        ss << "[" << x.lower() << "]";
      else 
        ss << "["<< x.lower() << ":" << x.upper() << "]";
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
      for(Interval i: xs) {
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
    for(MDI m: mdiList) {
      ERROR_UNLESS(m.Dimension()==mdi.Dimension(), "Dimension error #1");
      if (m.Dimension()!=mdi.Dimension()) {
        std::cout << "Dimension error #2\n";
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
    //If usage.size > this->dimension, add empty intervals, without affecting the subsequent removal
    newIntervals.resize(usage.size());
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
      ERROR("Dimension error #3\n");
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
    //TODO: Apply offset
    if (this->Dimension()!=other.Dimension()) {
      std::cout << *this << " " << other << "\n";
      ERROR("Dimension error #4\n");
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
      ERROR("Dimension error #5\n");
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

  bool MDI::Contains(const MDI &other) const {
    if (this->Dimension()!=other.Dimension())
      return false;
    else {
      for (int i=0; i<(int)this->intervals.size(); i++){
        if (!boost::icl::contains(this->intervals[i],other.intervals[i]))
          return false;
      }
      //If each interval of "this" contains its corresponding interval of "other" return true
      return true;
    }
    return false;
  }
  /*****************************************************************************
   ****************************************************************************/



  /*****************************************************************************
   ****                            INDEX PAIR                               ****
   *****************************************************************************/
  std::list<IndexPair> IndexPair::operator-(const IndexPair& other) const {
    ERROR_UNLESS((this->Dom().Dimension()==other.Dom().Dimension()) &&
        (this->Ran().Dimension()==other.Ran().Dimension()), "Dimension error #6\n");
    std::list<MDI> remainsDom = this->Dom()-other.Dom();
    std::list<MDI> remainsRan = this->Ran()-other.Ran();
    std::list<MDI>::iterator domIter = remainsDom.begin();
    std::list<MDI>::iterator ranIter = remainsRan.begin();
    std::list<IndexPair> ret;
    while (domIter!=remainsDom.end()) {
      ret.push_back(IndexPair(*domIter,*ranIter,this->offset, this->usage)); 
      domIter++;
      ranIter++;
    }
    return ret;
  }

  IndexPairSet IndexPair::RemoveUnknowns(MDI mdi) {
    if (Ran().Dimension()==0) {
      ERROR_UNLESS(mdi.Dimension()==0, "Removing ranges of different dimension");
      return {};
    }
    if (Option<MDI> intersection = mdi & this->Ran()) {
      MDI domToRemove = intersection.get().ApplyUsage(this->usage);
      MDI ranToRemove = intersection.get();
      std::list<MDI> remainsDom = {this->Dom()};
      if (this->Dom().Dimension()!=0) 
        remainsDom = this->Dom().Remove(domToRemove, -offset);

      std::list<MDI> remainsRan = this->Ran()-ranToRemove;
      std::list<MDI>::iterator domIter = remainsDom.begin();
      std::list<MDI>::iterator ranIter = remainsRan.begin();
      IndexPairSet ret;
      while (domIter!=remainsDom.end()) {
        ret.insert(IndexPair(*domIter,*ranIter,this->offset, this->usage));
        domIter++;
        ranIter++;
      }
      return ret;
    }
    else return IndexPairSet({*this});
  }

  IndexPairSet IndexPair::RemoveEquations(MDI mdi) {
    //If there is intersection, there is something to remove
    if (Option<MDI> domToRemove = mdi & this->Dom()) {
      //If the dimension of domToRemove is 0, is not a for-eq, we should remove it all
      if (domToRemove.get().Dimension() == 0) {
       ERROR_UNLESS(this->Ran().Size() == 1, "Dimension error #7");
       return IndexPairSet();
      } else {
        MDI ranToRemove = domToRemove.get().ApplyUsage(this->usage);
        ////std::cout << "From " << this->Ran() << " remove " << ranToRemove << "\n";
        std::list<MDI> remainsRan = {this->Ran()};
        if (this->Ran().Dimension()!=0) 
          remainsRan = this->Ran().Remove(ranToRemove, offset);
        std::list<MDI> remainsDom = this->Dom()-domToRemove.get();
        std::list<MDI>::iterator domIter = remainsDom.begin();
        std::list<MDI>::iterator ranIter = remainsRan.begin();
        ERROR_UNLESS(ranIter != remainsRan.end(), "remainsRan is empty");
        IndexPairSet ret;
        while (domIter!=remainsDom.end()) {
          IndexPair ip(*domIter,*ranIter,this->offset, this->usage);
          ret.insert(ip);
          domIter++;
          if (this->Ran().Dimension()!=0) 
            ranIter++;
        }
        return ret;
      }
    }
    else return IndexPairSet({*this});
  }

  bool IndexPair::operator<(const IndexPair& other) const {
    return this->Dom() < other.Dom() || this->Ran() < other.Ran() || this->OS() < other.OS();
  }

  std::ostream& operator<<(std::ostream &os, const IndexPair &ip) {
    os << "(" << ip.Dom() << ", " << ip.Ran() << ")";
    /*if (ip.OS().Size()) {
      os << "Offset = {";
      for (int i: ip.OS()) 
        os << i << " ";
      os << "} Usage = {";
      for (int i: ip.GetUsage()) 
        os << i << " ";
      os << "}";
    }*/
    return os;
  }

  bool IndexPair::Contains(const IndexPair& other) const {
    if (this->offset!=other.offset || this->usage!=other.usage)
      return false;
    if (this->dom.Size()<other.dom.Size() || this->ran.Size()<other.ran.Size())
      return false;
    if (this->dom.Contains(other.dom) & this->ran.Contains(other.ran))
      return true;
    else
      return false;
  }

  Option<IndexPair> IndexPair::operator&(const IndexPair& other) const {
    //TODO:
    return Option<IndexPair>();
  }
  /*****************************************************************************
   ****************************************************************************/



  /*****************************************************************************
   ****                              LABEL                                  ****
   *****************************************************************************/
  Label::Label(IndexPairSet ips): ips(ips) {
//   this->RemoveDuplicates();
  }


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

  void Label::RemoveUnknowns(MDI const mdi) {
    IndexPairSet newIps;
    for(IndexPair ip: this->ips) {
      IndexPairSet afterRemove = ip.RemoveUnknowns(mdi);
      newIps.insert(afterRemove.begin(), afterRemove.end());
    }
//    std::cout << "\nLabel::RemoveUnknowns result:\n" << newIps << "\n";
    this->ips=newIps;
  }

  void Label::RemoveEquations(MDI const mdi) {
    IndexPairSet newIps;
    for(IndexPair ipOld: this->ips) {
      for(IndexPair ipNew: ipOld.RemoveEquations(mdi)) {
        newIps.insert(ipNew);
      }
    }
//    std::cout << "\nLabel::RemoveEquations result:\n" << newIps << "\n";
    this->ips=newIps;
  }


  std::ostream& operator<<(std::ostream &os, const Label &label) {
    os << label.ips;
    return os;
  }

  void Label::RemoveDuplicates() {
    bool removeSomething = true;
    while (removeSomething) {
      IndexPairSet retIPS = ips;
      for (IndexPairSet::iterator checkingIP=ips.begin(); checkingIP!=ips.end(); checkingIP++) {
        //Ignore pairs 1-1
        if (checkingIP->Dom().Size()==1 && checkingIP->Ran().Size()==1)
          continue;
        for (IndexPairSet::iterator ip=ips.begin(); ip!=ips.end(); ip++) {
          //Ignore the same pair
          if (checkingIP == ip)
            continue;
          if (Option<IndexPair> hasToRemove = *checkingIP & *ip) {
            retIPS.erase(*ip);
            IndexPair ipToRemove = hasToRemove.get();
            std::list<IndexPair> remainingIPS = (*ip)-ipToRemove;
            retIPS.insert(remainingIPS.begin(), remainingIPS.end());
            ips = retIPS;
          }
        }
      }
      removeSomething = false;
    }
  }
  /*****************************************************************************
   ****************************************************************************/


  std::ostream& operator<<(std::ostream &os, const std::list<MDI> &mdiList) {
    std::list<std::string> mdiStList;
    for(MDI mdi: mdiList) {
      std::stringstream ss;
      ss << mdi;
      mdiStList.push_back(ss.str());
    }
    os << "{" << boost::algorithm::join(mdiStList, ",") << "}";
    return os;
  }

  std::ostream& operator<<(std::ostream &os, const std::list<IndexPair> &ipList) {
    std::list<std::string> ipsStList;
    foreach_(IndexPair ip, ipList){
      std::ostringstream ipSt;
      ipSt << ip;
      ipsStList.push_back(ipSt.str());
    }
    std::string ipsSt = "{" + boost::algorithm::join(ipsStList, ",") + "}";
    os << ipsSt;
    return os;
  }


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
