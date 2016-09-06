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

  std::ostream& operator<<(std::ostream &os, const IndexPair &ip) {
    std::pair<std::list<std::string>, std::list<std::string> > pairSt;
    foreach_(Interval i, Ran(ip).getIntervals()) {
      std::stringstream ss;
      ss << i;
      pairSt.first.push_back(ss.str());
    }
    foreach_(Interval i, Dom(ip).getIntervals()) {
      std::stringstream ss;
      ss << i;
      pairSt.second.push_back(ss.str());
    }
    std::string joinedString = "(Eq=(" + boost::algorithm::join(pairSt.first, ",") + "),Unk=(" + boost::algorithm::join(pairSt.second, ",") + "))";
    os << joinedString;
//    os << "Usage of first pair = {";
//    foreach_(int i, get<0>(ip).second)
//      os << i << ", ";
//    os << "}. Usage of second pair {";
//    foreach_(int i, get<1>(ip).second)
//      os << i << ", ";
//    os << "}";
//    os << "Offset list = {";
//    foreach_(int i, get<2>(ip)) {
//      os << " " << i;
//    }
//    os << "}\n";
    return os;
  }

  std::ostream& operator<<(std::ostream &os, const IndexPairOld &ip) {  //TODO: BORRAR!
    return os;
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

  std::ostream& operator<<(std::ostream &os, const IndexPairSetOld &ips) {  //TODO: BORRAR!
    std::list<std::string> ipsStList;
    foreach_(IndexPairOld ip, ips){
      std::ostringstream ipSt;
      ipSt << ip;
      ipsStList.push_back(ipSt.str());
    }
    std::string ipsSt = "<" + boost::algorithm::join(ipsStList, ",") + ">";
    os << ipsSt;
    return os;
  }

  std::ostream& operator<<(std::ostream &os, const VectorEdgeProperty &ep) {  //TODO: BORRAR!
    os << ep.labels;
    return os;
  }


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

  int MDI::Size() const{
    int size = 1;
    foreach_(Interval i, intervals) {
      size*=(i.upper()+1-i.lower());
    }
    return size;
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
      IntervalList ys=xs.intervalList;
      ys.push_front(i);
      mdiListRet.push_back(ys);
    }
    return mdiListRet;
  }

  std::list<MDI> MDI::PutLists(MDI mdi, std::list<MDI> mdiList) {
    std::list<MDI> mdiListRet;
    BOOST_FOREACH(Interval i, mdi.intervalList) {
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
      IntervalList xs = xss.front().intervalList;
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


  std::list<MDI> MDI::operator-(MDI &other) {
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

  void Label::RemovePairs(IndexPairSet ips) {

  }

  std::ostream& operator<<(std::ostream &os, const Label &label) {
    os << label.ips;
    return os;
  }


  void VectorEdgeProperty::RemovePairs(IndexPairSetOld ips) {  //TODO: BORRAR!

  }

  void VectorEdgeProperty::RemoveUnknowns(IndexPairSetOld ips_remove) { //TODO: BORRAR!
    /*
    IndexPairSet new_labels;
    foreach_(IndexPair ip, labels)  {
      bool found=false;
      foreach_(IndexPair ip_remove, ips_remove)  {
        if (ip_remove.second == ip.second) {
          found=true;
          break;
        }
      }
      if (!found)
        new_labels.insert(ip);
    }
    labels = new_labels;
    */
  } ;
  void VectorEdgeProperty::RemoveEquations(IndexPairSetOld ips_remove) { //TODO: BORRAR!
    /*
    IndexPairSet new_labels;
    foreach_(IndexPair ip, labels)  {
      bool found=false;
      foreach_(IndexPair ip_remove, ips_remove)  {
        if (ip_remove.first == ip.first) {
          found=true;
          break;
        }
      }
      if (!found)
        new_labels.insert(ip);
    }
    labels = new_labels;
    */
  }
  unsigned long int IntervalCount (IntervalList ilu) {
    unsigned long int count = 1;
    foreach_(Interval i, ilu) {
      count *= Size(i);
    }
    return count;
  }
  unsigned long int VectorEdgeProperty::EdgeCount() { //TODO: BORRAR!
    return Causalize::EdgeCount(labels);
  }
  unsigned long int EdgeCount(IndexPairSetOld labels) {
    unsigned long int count = 0;
    foreach_(IndexPairOld ip, labels) {
      unsigned long int eq_count = IntervalCount(get<0>(ip).first);
      unsigned long int unk_count = IntervalCount(get<1>(ip).first);
      count += (eq_count > unk_count ? eq_count : unk_count);
    //unsigned long int eq_ount = 
      
    }
    return count;
  }


}
