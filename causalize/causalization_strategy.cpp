/*
 * causalization_strategy.cpp
 *
 *  Created on: 12/05/2013
 *      Author: fede
 */

#include <causalize/causalization_strategy.h>
#include <causalize/vector/graph_definition.h>
#include <causalize/vector/graph_printer.h>
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

  EquationList &equations = mmo_class.equations_ref().equations_ref();

  UnknownsCollector collector(mmo_class);
  ExpList unknowns = collector.collectUnknowns();

  if (equations.size() != unknowns.size()) {
    ERROR ("The model being causalized is not balanced.\n"
           "There are %d equations and %d variables\n",
           equations.size(), unknowns.size());
  }

  int index = 0;

  _all_unknowns = unknowns;

  DEBUG('c', "Building causalization graph...\n");

  DEBUG('c', "Equation indexes:\n");

  foreach_(Equation &e, equations) {
    VertexProperties vp;
    Equality &eq = get<Equality>(e);
    PartEvalExp eval(_mmo_class.syms_ref(),false);
    eq.left_ref()=boost::apply_visitor(eval ,eq.left_ref());
    eq.right_ref()=boost::apply_visitor(eval ,eq.right_ref());
    vp.eqs = EquationList(1,e);
    vp.type = E;
    vp.index = index++;
    Vertex v = add_vertex(vp, _graph);
    _eqVertices.push_back(v);
    if (debugIsEnabled('c'))
      cout << vp.index << ":" << e << endl;
  }

  DEBUG('c', "Unknown indexes:\n");

  index = 0;
  foreach_(Expression e, unknowns) {
    VertexProperties vp;
    vp.unknowns = ExpList(1, e);
    vp.type = U;
    vp.index = index++;
    Vertex v = add_vertex(vp, _graph);
    _unknownVertices.push_back(v);
    if (debugIsEnabled('c'))
      cout << vp.index << ":" << e << endl;
   }

  DEBUG('c', "Graph edges as (equation_index, uknown_index):\n");

  list<Vertex>::iterator acausalEqsIter, unknownsIter;
  foreach_(Vertex eqVertex, _eqVertices) {
    foreach_(Vertex unknownVertex , _unknownVertices) {
      Modelica::contains occurrs(_graph[unknownVertex].unknowns.front()); 
      Equation e = _graph[eqVertex].eqs.front();
      ERROR_UNLESS(is<Equality>(e), "Causalization of non-equality equation is not supported");
      Equality eq = boost::get<Equality>(e);
      const bool rl = boost::apply_visitor(occurrs,eq.right_ref());
      const bool ll = boost::apply_visitor(occurrs,eq.left_ref());
      if(rl || ll) {
        add_edge(eqVertex, unknownVertex, _graph);
        DEBUG('c', "(%d, %d) ", _graph[eqVertex].index, _graph[unknownVertex].index);
      }
    }
  }
  DEBUG('c', "\n");

}

void CausalizationStrategy::causalize(Name name) {

  static int step=0;
  GraphPrinter gp(_graph);
  char buff[1024];
  sprintf(buff,"graph_%d.dot",step++);
  gp.printGraph(buff);

  list<Vertex>::size_type acausalEqsSize;

  do {

    acausalEqsSize = _eqVertices.size();

    list<Vertex>::iterator iter, auxiliaryIter;

    auxiliaryIter = _eqVertices.begin();
    for(iter = auxiliaryIter; iter != _eqVertices.end(); iter = auxiliaryIter) {
      ++auxiliaryIter;
      Vertex eq = *iter;
      if (out_degree(eq, _graph) == 1) {
        Edge e = *out_edges(eq, _graph).first;
        Vertex unknown = target(e, _graph);
        remove_out_edge_if(unknown, boost::lambda::_1 != e, _graph);
        if (debugIsEnabled('c')) {
          cout << "Causalizing ";
          foreach_(Expression e, _graph[unknown].unknowns)
            cout << " " << e;
          cout << std::endl;
          cout << "Using ";
          foreach_(Equation e, _graph[eq].eqs)
            cout << std::endl << e;
          cout << std::endl;
        }
        makeCausalBegining(_graph[eq].eqs, _graph[unknown].unknowns);
        clear_vertex(unknown,_graph);
        remove_vertex(unknown,_graph);
        clear_vertex(eq,_graph);
        remove_vertex(eq,_graph);
        _eqVertices.erase(iter);
        _unknownVertices.remove(unknown);
        //GraphPrinter gpp(_graph);
        //sprintf(buff,"graph_%d.dot",step++);
        //gpp.printGraph(buff);
      } else if (out_degree(eq, _graph) == 0) {
        cout << "Causalizing " << _graph[eq].eqs.front() << std::endl;
        ERROR("Problem is singular. Not supported yet.\n");
      }
    }

    auxiliaryIter = _unknownVertices.begin();
    for(iter = auxiliaryIter; iter != _unknownVertices.end(); iter = auxiliaryIter) {
      ++auxiliaryIter;
      Vertex unknown = *(iter);
      if(out_degree(unknown, _graph) == 1) {
        Edge e = *out_edges(unknown, _graph).first;
        Vertex eq = target(e, _graph);
        remove_out_edge_if(eq, boost::lambda::_1 != e, _graph);
        if (debugIsEnabled('c')) {
          cout << "Causalizing " << _graph[unknown].unknowns.size() << " variables " << _graph[unknown].unknowns.front() << std::endl;

        }
        makeCausalEnd(_graph[eq].eqs, _graph[unknown].unknowns);
        clear_vertex(unknown,_graph);
        remove_vertex(unknown,_graph);
        clear_vertex(eq,_graph);
        remove_vertex(eq,_graph);
        _eqVertices.remove(eq);
        _unknownVertices.erase(iter);
        //sprintf(buff,"graph_%d.dot",step++);
        //GraphPrinter gpp(_graph);
        //gpp.printGraph(buff);

      } else if (out_degree(unknown, _graph) == 0) {
        cout << "Causalizing " << _graph[unknown].unknowns.size() << " variables " << _graph[unknown].unknowns.front() << std::endl;
        ERROR("Problem is singular. Not supported yet.\n");
      }
    }

  } while (!_eqVertices.empty()
      && (acausalEqsSize != _eqVertices.size()));

  if (acausalEqsSize == _eqVertices.size()) {
    DEBUG('c', "Algebraic loop(s) detected.\n");
    makeCausalMiddle();
   _causalEqsBegining.insert(_causalEqsBegining.end(),_causalEqsMiddle.begin(),_causalEqsMiddle.end());
  }

  _causalEqsBegining.insert(_causalEqsBegining.end(),_causalEqsEnd.begin(),_causalEqsEnd.end());
  _mmo_class.equations_ref().equations_ref() = _causalEqsBegining;

  sprintf(buff,"%s.c",_mmo_class.name().c_str());
  
  // Add new functions
  foreach_(ClassType ct, cl) {
    Class c = get<Class>(ct);
    _mmo_class.types_ref().push_back(c.name());
    MMO_Class *mmo = new MMO_Class(c);
    _mmo_class.tyTable_ref().insert(c.name(), Type::Class(c.name(),mmo));
  }
  std::fstream fs (buff, std::fstream::out);  
  fs << "#include <gsl/gsl_multiroots.h>\n";
  fs << "#define pre(X) X\n";
  foreach_(std::string s, c_code) 
    fs << s;
  fs.close();

}

void CausalizationStrategy::makeCausalBegining(EquationList eqs, ExpList unknowns) {
  EquationList causalEqs = EquationSolver::solve(eqs, unknowns, _mmo_class.syms_ref(),c_code,cl);
  foreach_(Equation e, causalEqs) {
    _causalEqsBegining.push_back(e);
  }
}

void CausalizationStrategy::makeCausalEnd(EquationList eqs, ExpList unknowns) {
  EquationList causalEqs = EquationSolver::solve(eqs, unknowns, _mmo_class.syms_ref(),c_code,cl);
  foreach_(Equation e, causalEqs) {
    _causalEqsEnd.insert(_causalEqsEnd.begin(),e);
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
      ExpList _unknowns = _graph[v].unknowns;
      Expression unknown = _unknowns.front();
      unknowns.push_back(unknown);
    }

    std::list<Vertex> *eqVertices = component->eqVertices;
    EquationList eqs;
    std::list<Vertex>::iterator eqIt;
    for (eqIt = eqVertices->begin(); eqIt != eqVertices->end(); eqIt++) {
      Vertex v = *eqIt;
      EquationList _eqs = _graph[v].eqs;
      Equation eq = _eqs.front();
      eqs.push_back(eq);
    }

    EquationList causalEqs = EquationSolver::solve(eqs, unknowns, _mmo_class.syms_ref(),c_code,cl);

    _causalEqsMiddle.insert(_causalEqsMiddle.end(), causalEqs.begin(), causalEqs.end());
  }
}
