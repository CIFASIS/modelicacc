/*
 * causalization_strategy.cpp
 *
 *  Created on: 12/05/2013
 *      Author: fede
 */

#include <causalize/causalization_strategy.h>
#include <causalize/vector/graph_definition.h>
#include <ast/ast_types.h>
#include <causalize/for_unrolling/process_for_equations.h>
#include <util/debug.h>
#include <causalize/unknowns_collector.h>
#include <causalize/apply_tarjan.h>
#include <boost/lambda/lambda.hpp>
#include <ast/equation.h>
#include <boost/variant/get.hpp>
#include <mmo/mmo_class.h>
#include <util/ast_visitors/contains.h>
#include <util/ast_visitors/part_evalexp.h>
#include <util/solve/solve.h>
#include <fstream> 

using namespace Modelica::AST;
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
    VertexProperties vp;
    Equality &eq = get<Equality>(e);
    PartEvalExp eval(_mmo_class.syms_ref(),false);
    eq.left_ref()=boost::apply_visitor(eval ,eq.left_ref());
    eq.right_ref()=boost::apply_visitor(eval ,eq.right_ref());
    vp.eq = e;
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
    VertexProperties vp;
    vp.unknown = e;
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
      Modelica::contains occurrs(_graph[unknownVertex].unknown);
      Equation e = _graph[eqVertex].eq;
      ERROR_UNLESS(is<Equality>(e), "Causalization of non-equality equation is not supported");
      Equality eq = boost::get<Equality>(e);
      const bool rl = boost::apply_visitor(occurrs,eq.left_ref());
      const bool ll = boost::apply_visitor(occurrs,eq.right_ref()); 
      if(rl || ll) {
        add_edge(eqVertex, unknownVertex, _graph);
        DEBUG('c', "(%d, %d) ", _graph[eqVertex].index, _graph[unknownVertex].index);
      }
    }
  }

  DEBUG('c', "\n");

  _causalEqsEnd.resize(equations.size());

}

void CausalizationStrategy::causalize(Modelica::AST::Name name) {
  
  simpleCausalizationStrategy();

  makeCausalMiddle();
  
  _causalEqsBegining.insert(_causalEqsBegining.end(),_causalEqsMiddle.begin(),_causalEqsMiddle.end());
  _causalEqsBegining.insert(_causalEqsBegining.end(),_causalEqsEnd.begin(),_causalEqsEnd.end());

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

  _causalEqsBegining.insert(_causalEqsBegining.end(),_causalEqsEnd.begin(),_causalEqsEnd.end());

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
    if (out_degree(v, _graph) == 1) {
      _graph[v].visited = true;
      if (_graph[v].type == E) {
        eqDegree1Verts.push_back(v);
      } else {
        unknownDegree1Verts.push_back(v);
      }
    }
  }

  int causalEqsEndIndex = _causalEqsEnd.size() - 1;

  while(!eqDegree1Verts.empty() || !unknownDegree1Verts.empty()) {
    std::list<Vertex>::iterator eqIter = eqDegree1Verts.begin();
    if (eqIter != eqDegree1Verts.end()) {
      Vertex eq = *eqIter;
      Edge e = getUniqueEdge(eq);
      Vertex unknown = target(e, _graph);
      makeCausalBegining(_graph[eq].eq, _graph[unknown].unknown);
      remove_edge(e, _graph);
      remove_vertex(eq,_graph);
      // If the unknown is in the degree1 set also remove it. This is the case where both eq and unknown have degree=1
      unknownDegree1Verts.remove(unknown);
      CausalizationGraph::out_edge_iterator outEdgeIter, outEdgeIterEnd, next;
      boost::tie(outEdgeIter, outEdgeIterEnd) = out_edges(unknown, _graph);
      for(next = outEdgeIter; outEdgeIter != outEdgeIterEnd; outEdgeIter = next) {
        next++;
        Edge unkownAdjEdge = *outEdgeIter;
        Vertex unknownAdjEq = target(unkownAdjEdge, _graph);
        remove_edge(unkownAdjEdge, _graph);
        if (out_degree(unknownAdjEq, _graph) == 1) {
            Edge e = getUniqueEdge(unknownAdjEq);
            Vertex eqAdjUnknown = target(e, _graph);
            if(!_graph[eqAdjUnknown].visited) {
                _graph[unknownAdjEq].visited = true;
                eqDegree1Verts.push_back(unknownAdjEq);
            }
        }
      }
      remove_vertex(unknown,_graph);
      eqDegree1Verts.erase(eqIter);
    }

    std::list<Vertex>::iterator unknownIter = unknownDegree1Verts.begin();
    if(unknownIter != unknownDegree1Verts.end()) {
      Vertex unknown = *unknownIter;
      Edge e = getUniqueEdge(unknown);
      Vertex eq = target(e, _graph);
      makeCausalEnd(_graph[eq].eq, _graph[unknown].unknown, causalEqsEndIndex--);
      remove_edge(e, _graph);
      // If the eq is in the degree1 set also remove it. This is the case where both eq and unknown have degree=1
      eqDegree1Verts.remove(eq);
      remove_vertex(unknown, _graph);
      CausalizationGraph::out_edge_iterator outEdgeIter, outEdgeIterEnd, next;
      boost::tie(outEdgeIter, outEdgeIterEnd) = out_edges(eq, _graph);
      for(next = outEdgeIter; outEdgeIter != outEdgeIterEnd; outEdgeIter = next) {
        next++;
        Edge eqAdjEdge = *outEdgeIter;
        Vertex eqAdjUnknown = target(eqAdjEdge, _graph);
        remove_edge(eqAdjEdge, _graph);
        if (out_degree(eqAdjUnknown, _graph) == 1) {
            Edge e = getUniqueEdge(eqAdjUnknown);
            Vertex unknownAdjEq = target(e, _graph);
            if(!_graph[unknownAdjEq].visited) {
                _graph[eqAdjUnknown].visited = true;
                unknownDegree1Verts.push_back(eqAdjUnknown);
            }
        }
      }
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
  EquationList causalEqs = EquationSolver::solve(EquationList(1, e), ExpList(1, unknown), _mmo_class.syms_ref(), c_code, _cl);
  foreach_(Equation e, causalEqs) {
    _causalEqsBegining.push_back(e);
  }
}

void CausalizationStrategy::makeCausalEnd(Equation e, Expression unknown, int causalEqsEndIndex) {
  if (debugIsEnabled('c')) {
    cout << "makeCausalEnd" << endl;
    cout << "Causalizing";
    cout << " " << unknown;
    cout << std::endl;
    cout << "Using ";
    cout << std::endl << e;
    cout << std::endl;
  }
  EquationList causalEqs = EquationSolver::solve(EquationList(1, e), ExpList(1, unknown), _mmo_class.syms_ref(), c_code, _cl);
  foreach_(Equation e, causalEqs) {
    _causalEqsEnd[causalEqsEndIndex] = e;
  }
}

/**
 * Applies tarjan algorithm
 */
void CausalizationStrategy::makeCausalMiddle() {

  std::map<int, causalize::Component> components;

  int n_comps = apply_tarjan(_graph, components);

  for (int i = 0; i < n_comps; i++) {

    causalize::Component component = components[i];

    std::list<Vertex> *uVertices = component->uVertices;
    ExpList unknowns;
    std::list<Vertex>::iterator uIt;
    for (uIt = uVertices->begin(); uIt != uVertices->end(); uIt++) {
      Vertex v = *uIt;
      Expression unknown = _graph[v].unknown;
      unknowns.push_back(unknown);
    }

    std::list<Vertex> *eqVertices = component->eqVertices;
    EquationList eqs;
    std::list<Vertex>::iterator eqIt;
    for (eqIt = eqVertices->begin(); eqIt != eqVertices->end(); eqIt++) {
      Vertex v = *eqIt;
      Equation eq = _graph[v].eq;
      eqs.push_back(eq);
    }

    EquationList causalEqs = EquationSolver::solve(eqs, unknowns, _mmo_class.syms_ref(), c_code, _cl);

    _causalEqsMiddle.insert(_causalEqsMiddle.end(), causalEqs.begin(), causalEqs.end());
  }
}
