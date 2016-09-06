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

#ifndef VECTOR_GRAPH_DEFINITION_
#define VECTOR_GRAPH_DEFINITION_

#include <utility>
#include <set>
#include <boost/config.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/icl/discrete_interval.hpp>

#include <ast/equation.h>
#include <causalize/graph/graph_definition.h>

#include <ast/ast_types.h>
#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <sstream>
#include <string>

namespace ICL = boost::icl;
namespace Causalize {
  /// @brief This is the property for a vertex in the incidence graph. Nodes can be of two types: Equation or Unknow.

  struct VectorUnknown: Unknown {
    int dimension;
    std::vector<int> dimensionList;
    VectorUnknown(){};
    VectorUnknown(VarInfo varInfo, Modelica::AST::Reference var);
    void SetIndex(Modelica::AST::ExpList index);
  };

  struct VectorVertexProperty: VertexProperty {
  /// @brief The number of equations or unknowns left to causalize in this node
    int count;
    VectorUnknown unknown;
  };

  /// @brief A pair representing a usage of a variable in an equation
  typedef ICL::discrete_interval<int> Interval;
  inline Interval CreateInterval(int a, int b) {
    return ICL::discrete_interval<int>(a,b, ICL::interval_bounds::closed());
  }
  typedef std::list<Interval> IntervalList;


  /*****************************************************************************
   ****                               MDI                                   ****
   *****************************************************************************/
  class MDI { //Multi-Dimensional Interval

  public:
    MDI(int d, ... );
    inline MDI(IntervalList intervalList): intervalList(intervalList) {};
    inline int Dimension() const {return intervals.size(); }
    int Size () const;
    inline IntervalList getIntervals() {return intervalList; }
    std::list<MDI> operator-(MDI &other);
    friend std::ostream& operator<<(std::ostream &os, const MDI mdi);

  private:
      std::vector<Interval> intervals;
      std::list<int> usage;

      IntervalList intervalList;   //TODO: BORRAR!
      typedef IntervalList::iterator iterator;
      typedef IntervalList::const_iterator const_iterator;
      inline iterator begin() { return intervalList.begin(); }
      inline const_iterator begin() const { return intervalList.begin(); }
      inline iterator end() { return intervalList.end(); }

      IntervalList Partition(Interval iA, Interval iB);
      std::list<MDI> PutHead(Interval i, std::list<MDI> mdiList);
      std::list<MDI> Filter(std::list<MDI> mdiList, MDI mdi);
      std::list<MDI> CartProd(std::list<MDI> mdiList);
      std::list<MDI> PutLists(MDI mdi, std::list<MDI> mdiList);
  };


  typedef std::pair<IntervalList, std::vector<int> > IntervalListUsage;   //TODO: BORRAR!

  typedef std::list<int> Offset;

  typedef boost::tuple<IntervalListUsage, IntervalListUsage, Offset> IndexPairOld;  //TODO: BORRAR!

  typedef boost::tuple<MDI, MDI, Offset> IndexPair;
  inline IndexPairOld CreateIndexPair(Interval a, Interval b, std::vector<int> ua, std::vector<int> ub, std::list<int> offset) {
    return make_tuple(make_pair(std::list<Interval>(1,a),ua) , make_pair(std::list<Interval>(1,b),ub), offset);
  }

  inline MDI Ran(IndexPair ip) { return get<0>(ip); }

  inline MDI Dom(IndexPair ip) { return get<1>(ip); }

  inline unsigned long int Size(Interval i) { return i.upper() - i.lower(); }  //TODO: BORRAR!

  std::ostream& operator<<(std::ostream &os, const IndexPairOld &ip);  //TODO: BORRAR!

  typedef std::set<IndexPairOld> IndexPairSetOld; //TODO: BORRAR!
  typedef std::set<IndexPair> IndexPairSet;



  /*****************************************************************************
   ****                              LABEL                                  ****
   *****************************************************************************/
  class Label {
  public:
    inline Label(IndexPairSet ips): ips(ips) {};
    Label nrres(MDI const mdi) const;
    Label ndres(MDI const mdi) const;
    void RemovePairs(IndexPairSet ips);
    unsigned long int EdgeCount();
    inline bool IsEmpty() { return ips.size()==0; }
    friend std::ostream& operator<<(std::ostream &os, const Label &label);
  private:
    IndexPairSet ips;
  };

  std::ostream& operator<<(std::ostream &os, const IndexPairSet &ips);

  std::ostream& operator<<(std::ostream &os, const IndexPairSetOld &ips);   //TODO:: BORRAR!


  unsigned long int EdgeCount(IndexPairSetOld);


  struct VectorEdgeProperty {  //TODO: BORRAR!
    friend std::ostream& operator<<(std::ostream &os, const VectorEdgeProperty &ep);
    IndexPairSetOld labels;
/// @brief This function removes a set of pairs from this Edge
/// @param ips set of pairs to remove
    void RemovePairs(IndexPairSetOld ips);
    void RemoveUnknowns(IndexPairSetOld ips_remove);
    void RemoveEquations(IndexPairSetOld ips_remove);
    unsigned long int EdgeCount();
    inline bool IsEmpty() { return labels.size()==0; }

  };

  /// @brief This is the definition of the Incidence graph for the vector case.
  typedef boost::adjacency_list<boost::listS, boost::listS, boost::undirectedS, VectorVertexProperty, VectorEdgeProperty> VectorCausalizationGraph;
  /// @brief This a node from the vectorized incidence graph
  typedef Causalize::VectorCausalizationGraph::vertex_descriptor VectorVertex;
  /// @brief An equation vertex is the same as a regular vertex
  typedef VectorVertex VectorEquationVertex;
  /// @brief An unknown vertex is the same as a regular vertex
  typedef VectorVertex VectorUnknownVertex;
  /// @brief This is an edge of the vectorized causalization graph
  typedef VectorCausalizationGraph::edge_descriptor VectorEdge;
  /// @brief This struct represents a set of causalized vars for the vector algorithm

  struct CausalizedVar{
    VectorUnknown unknown;
    Equation equation;
    IndexPairSetOld pairs;
  };
}
#endif
