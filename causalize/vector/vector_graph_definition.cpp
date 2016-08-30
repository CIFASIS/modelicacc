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
    foreach_(boost::icl::interval_set<int> i, ip.first) {
      std::stringstream ss;
      ss << i;
      pairSt.first.push_back(ss.str());
    }
    foreach_(boost::icl::interval_set<int> i, ip.second) {
      std::stringstream ss;
      ss << i;
      pairSt.second.push_back(ss.str());
    }
    std::string joinedString = "((" + boost::algorithm::join(pairSt.first, ",") + "),(" + boost::algorithm::join(pairSt.second, ",") + "))";
    os << joinedString;
    return os;
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

  std::ostream& operator<<(std::ostream &os, const VectorEdgeProperty &ep) {
    os << ep.labels;
    return os;
  }

  /*
  std::set<std::list<int> > VectorEdgeProperty::getDom() const {
    std::set<std::list<int> > dom;
    foreach_(IndexPair ip, labels)
      dom.insert(ip.first);
    return dom;
  }
  std::set<std::list<int> > VectorEdgeProperty::getRan() const {
    std::set<std::list<int> > ran;
    foreach_(IndexPair ip, labels)
      ran.insert(ip.second);
    return ran;
  }
  */

  void VectorEdgeProperty::RemovePairs(IndexPairSet ips) {
    foreach_(IndexPair ip, ips)
      labels.erase(ip);
  }

  void VectorEdgeProperty::RemoveUnknowns(IndexPairSet ips_remove) {
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
  } ;
  void VectorEdgeProperty::RemoveEquations(IndexPairSet ips_remove) {
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
  }

}
