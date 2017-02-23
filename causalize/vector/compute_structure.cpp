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

#define HAS_COUNT
#include <causalize/vector/compute_structure.h>
#include <causalize/vector/vector_graph_definition.h>
#include <causalize/graph/graph_printer.h>

#include <boost/tuple/tuple.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/variant/get.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/icl/discrete_interval.hpp>
#include <boost/icl/interval_set.hpp>

#include <mmo/mmo_class.h>
#include <ast/queries.h>

#include <sstream>

using namespace Modelica;
using namespace std;
using namespace boost;
using namespace boost::icl;

namespace Causalize {
Vertex ComputeStructure::GetEquation(Edge e) {
  return ((graph[(source(e, graph))].type == kVertexEquation))
             ? source(e, graph)
             : target(e, graph);
}

Vertex ComputeStructure::GetUnknown(Edge e) {
  return ((graph[(target(e, graph))].type == kVertexUnknown))
             ? target(e, graph)
             : source(e, graph);
}

ComputeStructure::ComputeStructure(VectorCausalizationGraph g, MMO_Class& m)
    : mmo(m) {
  graph = g;
  VectorCausalizationGraph::vertex_iterator vi, vi_end;
  for (boost::tie(vi, vi_end) = vertices(graph); vi != vi_end; vi++) {
    VectorVertex current_element = *vi;
    if (graph[current_element].type == kVertexEquation)
      equationDescriptors.push_back(current_element);
    else
      unknownDescriptors.push_back(current_element);
  }
}

MDI ComputeStructure::GetCandidate (VectorVertex u) {
  // First try to use one of the ranges found on the edges
  foreach_(VectorEdge e, out_edges(u, graph)) {
    Label l = graph[e];
    for (IndexPair p : l.Pairs()) {
      MDI candidate = p.Ran();
      if (TestCandidate(u, candidate)) 
        return candidate;
    }
  }
  // If none is found, take the first and do the intersection with the others
  MDI candidate;
  foreach_(VectorEdge e, out_edges(u, graph)) {
    Label l = graph[e];
    IndexPair p = *l.Pairs().begin();
    candidate = p.Ran();
    break;
  }
  // candidate is the first
  //std::cout << "Taking candidate = " << candidate << " and doing intersection\n";
  foreach_(VectorEdge e, out_edges(u, graph)) {
    Label l = graph[e];
    for (IndexPair p : l.Pairs()) {
      Option<MDI> inter = p.Ran() & candidate;
      if (inter) {
        candidate = inter.get();
        //std::cout << "After intersecting in with = " << p.Ran() << " candidate is " << candidate << "\n";
      }
    }
  }
  return candidate;
}

bool ComputeStructure::TestCandidate(VectorVertex u, MDI candidate) {
  foreach_(VectorEdge e, out_edges(u, graph)) {
    Label l = graph[e];
    for (IndexPair p : l.Pairs()) {
      MDI m = p.Ran();
      if ( (m & candidate) && !m.Contains(candidate)) {
        //std::cout << "Range :" << candidate << " is bad because is not contained in " << m << "\n";
        return false;
      }
    }
  }
  return true;
}

void ComputeStructure::Compute() {
  int step = 0;
  for (VectorVertex u : unknownDescriptors) {
    if (graph[u].unknown.state) {
      if (out_degree(u, graph) == 0) {// State variable not used :s
        WARNING("State variable not used");
        graph[u].count = 0;
      }
      while (graph[u].count > 0) {
        MDI candidate = GetCandidate(u);
        std::cout << "For unknown " << graph[u].unknown() << " in range " << candidate << " of size " << candidate.Size() << "\n";
        foreach_(VectorEdge e, out_edges(u, graph)) {
           Label l = graph[e];
           for (IndexPair p : l.Pairs()) {
              MDI m = p.Ran();
              if (!m.Contains(candidate)) {
                 //std::cout << "Skiping " << m << " since it does not contain " << candidate  << "\n";
                 continue;
              }
              Option<MDI> omdi = m & candidate;
              assert(omdi);
              MDI uk_dom = omdi.get();
              VectorVertex eq = GetEquation(e);
              MDI eq_dom = p.RevertRange(uk_dom);
              std::cout << "\tMust recompute " << graph[eq].index << " in eq range " << eq_dom << "\n";
              //std::cout << "\tP: Usage= " << p.GetUsage() << ". Offset = " << p.GetOffset() << "\n";
              // Agregar al resultado las ecuaciones a calcular y sus rangos. Ojo con el orden //
             }
         }
         /* Update graph */
         graph[u].count -= candidate.Size();
         std::list<VectorEdge> rem_e;
         foreach_(VectorEdge e, out_edges(u, graph)) {
            Label l = graph[e];
            IndexPairSet ips;
            for (IndexPair p : l.Pairs())
               for (IndexPair ip : p.RemoveUnknowns(candidate))
                 ips.insert(ip);
            if (ips.empty()) rem_e.push_back(e);
              graph[e].SetPairs(ips);
          }
          for (VectorEdge e : rem_e)
            remove_edge(e,graph);
          /*stringstream ss;
          ss << "graph_" << step++ << ".dot";
          GraphPrinter<VectorVertexProperty,Label>  gp(graph);
          gp.printGraph(ss.str());*/
          step++;
 
 
 
      }
    }
  }
}
}
