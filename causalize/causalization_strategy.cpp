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
 * causalization_strategy.cpp
 *
 *  Created on: 12/05/2013
 *      Author: fede
 */

#include <causalize/causalization_strategy.h>
#include <causalize/graph/graph_definition.h>
#include <causalize/graph/graph_printer.h>
#include <ast/ast_types.h>
#include <causalize/for_unrolling/process_for_equations.h>
#include <util/debug.h>
#include <causalize/unknowns_collector.h>
#include <causalize/apply_tarjan.h>
#include <boost/lambda/lambda.hpp>
#include <ast/equation.h>
#include <boost/variant/get.hpp>
#include <mmo/mmo_class.h>
#include <util/ast_visitors/contains_expression.h>
#include <util/ast_visitors/partial_eval_expression.h>
#include <util/solve/solve.h>
#include <fstream> 

using namespace Modelica::AST;
namespace Causalize {
CausalizationStrategy::CausalizationStrategy(MMO_Class &mmo_class): _mmo_class(mmo_class) {

  Causalize::process_for_equations(mmo_class);

  const EquationList &equations = mmo_class.equations_ref().equations_ref();

  UnknownsCollector collector(mmo_class);
  ExpList unknowns = collector.collectUnknowns();

  if (equations.size() != unknowns.size()) {
    ERROR ("The model being causalized is not balanced.\n"
           "There are %d equations and %d variables\n",
           equations.size(), unknowns.size());
  }

  int index = 0;

  _all_unknowns = unknowns;

  std::list<Vertex> eqVerts;
  std::list<Vertex> unknownVerts;

  DEBUG('c', "Building causalization graph...\n");
  DEBUG('c', "Equation indexes:\n");

  foreach_(Equation e, equations) {
    VertexProperty vp;
    Equality &eq = get<Equality>(e);
    PartialEvalExpression eval(_mmo_class.syms_ref(),false);
    eq.left_ref()=Apply(eval ,eq.left_ref());
    eq.right_ref()=Apply(eval ,eq.right_ref());
    vp.eqs.push_back(e);
    vp.type = E;
    vp.index = index++;
    vp.visited = false;
    Vertex v = add_vertex(vp, _graph);
    eqVerts.push_back(v);
    if (debugIsEnabled('c'))
      cout << vp.index << ":" << e << endl;
  }

  DEBUG('c', "Unknown indexes:\n");

  index = 0;
  foreach_(Expression e, unknowns) {
    VertexProperty vp;
    vp.unknowns = ExpList(1, e);
    vp.type = U;
    vp.index = index++;
    vp.visited = false;
    Vertex v = add_vertex(vp, _graph);
    unknownVerts.push_back(v);
    if (debugIsEnabled('c'))
      cout << vp.index << ":" << e << endl;
   }

  DEBUG('c', "Graph edges as (equation_index, uknown_index):\n");

  list<Vertex>::iterator acausalEqsIter, unknownsIter;
  foreach_(Vertex eqVertex, eqVerts) {
    foreach_(Vertex unknownVertex , unknownVerts) {
      Modelica::ContainsExpression occurrs(_graph[unknownVertex].unknowns.front());
      Equation e = _graph[eqVertex].eqs.front();
      ERROR_UNLESS(is<Equality>(e), "Causalization of non-equality equation is not supported");
      Equality eq = boost::get<Equality>(e);
      const bool rl = Apply(occurrs,eq.left_ref());
      const bool ll = Apply(occurrs,eq.right_ref()); 
      if(rl || ll) {
        add_edge(eqVertex, unknownVertex, _graph);
        DEBUG('c', "(%d, %d) ", _graph[eqVertex].index, _graph[unknownVertex].index);
      }
    }
  }

  DEBUG('c', "\n");

  _causalEqsEnd.resize(equations.size());
  _causalEqsEndIndex = equations.size() - 1;

  GraphPrinter<VertexProperty,EdgeProperty> gp(_graph);
  gp.printGraph("initial_graph.dot");
}

void CausalizationStrategy::causalize(Modelica::AST::Name name) {
  
  DEBUG('p', "Graph size before simple strategy:%d\n", num_vertices(_graph));

  simpleCausalizationStrategy();

  int graph_size = num_vertices(_graph);

  DEBUG('p', "Graph size after simple strategy:%d\n", graph_size);

  if(graph_size > 0) { // graph still has vertices
    makeCausalMiddle();
    _causalEqsBegining.insert(_causalEqsBegining.end(),_causalEqsMiddle.begin(),_causalEqsMiddle.end());
  }

    for (size_t i = _causalEqsEndIndex + 1; i < _causalEqsEnd.size(); ++i)
  {
    _causalEqsBegining.insert(_causalEqsBegining.end(),_causalEqsEnd[i]);
  }

  _mmo_class.equations_ref().equations_ref() = _causalEqsBegining;

  // Add new functions
  foreach_(ClassType ct, _cl) {
   Class c = get<Class>(ct);
   _mmo_class.types_ref().push_back(c.name());
   MMO_Class *mmo = new MMO_Class(c);
   _mmo_class.tyTable_ref().insert(c.name(), Type::Class(c.name(),mmo));
 }
 char buff[1024];
 std::fstream fs (buff, std::fstream::out);
 fs << "#include <gsl/gsl_multiroots.h>\n";
 fs << "#define pre(X) X\n";
 foreach_(std::string s, c_code)
 fs << s;
 fs.close();
}

void CausalizationStrategy::causalize_simple(Modelica::AST::Name name) {
    
  simpleCausalizationStrategy();

  for (size_t i = _causalEqsEndIndex + 1; i < _causalEqsEnd.size(); ++i)
  {
    _causalEqsBegining.insert(_causalEqsBegining.end(),_causalEqsEnd[i]);
  }

  _mmo_class.equations_ref().equations_ref() = _causalEqsBegining;

  // Add new functions
 foreach_(ClassType ct, _cl) {
   Class c = get<Class>(ct);
   _mmo_class.types_ref().push_back(c.name());
   MMO_Class *mmo = new MMO_Class(c);
   _mmo_class.tyTable_ref().insert(c.name(), Type::Class(c.name(),mmo));
 }
 char buff[1024];
 std::fstream fs (buff, std::fstream::out);
 fs << "#include <gsl/gsl_multiroots.h>\n";
 fs << "#define pre(X) X\n";
 foreach_(std::string s, c_code)
 fs << s;
 fs.close();
}

void CausalizationStrategy::causalize_tarjan(Modelica::AST::Name name) {

  makeCausalMiddle();

  _causalEqsBegining.insert(_causalEqsBegining.end(),_causalEqsMiddle.begin(),_causalEqsMiddle.end());

  _mmo_class.equations_ref().equations_ref() = _causalEqsBegining;

  // Add new functions
 foreach_(ClassType ct, _cl) {
   Class c = get<Class>(ct);
   _mmo_class.types_ref().push_back(c.name());
   MMO_Class *mmo = new MMO_Class(c);
   _mmo_class.tyTable_ref().insert(c.name(), Type::Class(c.name(),mmo));
 }
 char buff[1024];
 std::fstream fs (buff, std::fstream::out);
 fs << "#include <gsl/gsl_multiroots.h>\n";
 fs << "#define pre(X) X\n";
 foreach_(std::string s, c_code)
 fs << s;
 fs.close();

}

void CausalizationStrategy::simpleCausalizationStrategy() {

  std::list<Vertex> eqDegree1Verts;
  std::list<Vertex> unknownDegree1Verts;

  CausalizationGraph::vertex_iterator vi, vi_end;
  for(boost::tie(vi,vi_end) = vertices(_graph); vi != vi_end; ++vi) {
    Vertex v = *vi;
    if (out_degree(v, _graph) == 1 && !_graph[v].visited) {
      Edge e = getUniqueEdge(v);
      Vertex adjacent = target(e, _graph);
      _graph[adjacent].visited = true;
      if (_graph[v].type == E) {
        eqDegree1Verts.push_back(v);
      } else {
        unknownDegree1Verts.push_back(v);
      }
    }
  }

  while(!eqDegree1Verts.empty() || !unknownDegree1Verts.empty()) {
    std::list<Vertex>::iterator eqIter = eqDegree1Verts.begin();
    if (eqIter != eqDegree1Verts.end()) {
      Vertex eq = *eqIter;
      Edge e = getUniqueEdge(eq);
      Vertex unknown = target(e, _graph);
      makeCausalBegining(_graph[eq].eqs.front(), _graph[unknown].unknowns.front());
      remove_edge(e, _graph);
      remove_vertex(eq,_graph);
      collectDegree1Verts(unknown, eqDegree1Verts);
      remove_vertex(unknown,_graph);
      eqDegree1Verts.erase(eqIter);
    }

    std::list<Vertex>::iterator unknownIter = unknownDegree1Verts.begin();
    if(unknownIter != unknownDegree1Verts.end()) {
      Vertex unknown = *unknownIter;
      Edge e = getUniqueEdge(unknown);
      Vertex eq = target(e, _graph);
      makeCausalEnd(_graph[eq].eqs.front(), _graph[unknown].unknowns.front());
      remove_edge(e, _graph);
      remove_vertex(unknown, _graph);
      collectDegree1Verts(eq, unknownDegree1Verts);
      remove_vertex(eq, _graph);
      unknownDegree1Verts.erase(unknownIter);
    }
  }
}

Edge CausalizationStrategy::getUniqueEdge(Vertex v) {
  CausalizationGraph::out_edge_iterator eqOutEdgeIter, eqOutEdgeIterEnd;
  boost::tie(eqOutEdgeIter, eqOutEdgeIterEnd) = out_edges(v, _graph);
  return *eqOutEdgeIter;
}

void CausalizationStrategy::collectDegree1Verts(Vertex v, std::list<Vertex> &degree1Verts) {
  CausalizationGraph::out_edge_iterator outEdgeIter, outEdgeIterEnd, next;
  boost::tie(outEdgeIter, outEdgeIterEnd) = out_edges(v, _graph);
  for(next = outEdgeIter; outEdgeIter != outEdgeIterEnd; outEdgeIter = next) {
    next++;
    Edge adjEdge = *outEdgeIter;
    Vertex adjacent = target(adjEdge, _graph);
    remove_edge(adjEdge, _graph);
    if (out_degree(adjacent, _graph) == 1 && !_graph[adjacent].visited) {
        Edge e = getUniqueEdge(adjacent);
        Vertex adjAdjacent = target(e, _graph);
        _graph[adjAdjacent].visited = true;
        degree1Verts.push_back(adjacent);
    }
  }
}

void CausalizationStrategy::makeCausalBegining(Equation e, Expression unknown) {
  if (debugIsEnabled('c')) {
      cout << "makeCausalBegining" << endl;
      cout << "Causalizing ";
      cout << " " << unknown;
      cout << std::endl;
      cout << "Using ";
      cout << std::endl << e;
      cout << std::endl;
  }
  Equation causalEq = EquationSolver::Solve(e, unknown, _mmo_class.syms_ref(), c_code, _cl);
  _causalEqsBegining.push_back(causalEq);
}

void CausalizationStrategy::makeCausalEnd(Equation e, Expression unknown) {
  if (debugIsEnabled('c')) {
    cout << "makeCausalEnd" << endl;
    cout << "Causalizing";
    cout << " " << unknown;
    cout << std::endl;
    cout << "Using ";
    cout << std::endl << e;
    cout << std::endl;
  }
  Equation causalEq = EquationSolver::Solve(e, unknown, _mmo_class.syms_ref(), c_code, _cl);
  _causalEqsEnd[_causalEqsEndIndex--] = causalEq;
}

/**
 * Applies tarjan algorithm
 */
void CausalizationStrategy::makeCausalMiddle() {

  std::map<int, Causalize::ComponentPtr> components;

  int n_comps = apply_tarjan(_graph, components);

  for (int i = 0; i < n_comps; i++) {

    ComponentPtr component = components[i];

    std::list<Vertex> *uVertices = component->uVertices;
    ExpList unknowns;
    std::list<Vertex>::iterator uIt;
    for (uIt = uVertices->begin(); uIt != uVertices->end(); uIt++) {
      Vertex v = *uIt;
      Expression unknown = _graph[v].unknowns.front();
      unknowns.push_back(unknown);
    }

    std::list<Vertex> *eqVertices = component->eqVertices;
    EquationList eqs;
    std::list<Vertex>::iterator eqIt;
    for (eqIt = eqVertices->begin(); eqIt != eqVertices->end(); eqIt++) {
      Vertex v = *eqIt;
      Equation eq = _graph[v].eqs.front();
      eqs.push_back(eq);
    }

    EquationList causalEqs = EquationSolver::Solve(eqs, unknowns, _mmo_class.syms_ref(), c_code, _cl);
    _causalEqsMiddle.insert(_causalEqsMiddle.end(), causalEqs.begin(), causalEqs.end());
  }
}
}
