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
    PartialEvalExpression eval(_mmo_class.syms_ref(),false);
    eq.left_ref()=boost::apply_visitor(eval ,eq.left_ref());
    eq.right_ref()=boost::apply_visitor(eval ,eq.right_ref());
    vp.eqs = EquationList(1,e);
    vp.type = E;
    vp.index = index++;
    EquationVertex v = add_vertex(vp, _graph);
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
    UnknownVertex v = add_vertex(vp, _graph);
    _unknownVertices.push_back(v);
    if (debugIsEnabled('c'))
      cout << vp.index << ":" << e << endl;
   }

  DEBUG('c', "Graph edges as (equation_index, uknown_index):\n");

  list<EquationVertex>::iterator acausalEqsIter;
  list<UnknownVertex>::iterator unknownsIter;
  foreach_(EquationVertex eqVertex, _eqVertices) {
    foreach_(UnknownVertex unknownVertex , _unknownVertices) {
      Modelica::ContainsExpression occurrs(_graph[unknownVertex].unknowns.front()); 
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

  char *path = get_current_dir_name();
  std::stringstream filename;
  filename << path << "/" << _mmo_class.name().c_str() << ".c";
  free(path);
  c_path = filename.str();

  GraphPrinter<VertexProperties,EdgeProperties> gp(_graph);
  gp.printGraph("initial_graph.dot");
}

void CausalizationStrategy::causalize(Name name) {

  list<EquationVertex>::size_type acausalEqsSize;

  do {

    acausalEqsSize = _eqVertices.size();

    list<Vertex>::iterator iter, auxiliaryIter;

    auxiliaryIter = _eqVertices.begin();
    for(iter = auxiliaryIter; iter != _eqVertices.end(); iter = auxiliaryIter) {
      ++auxiliaryIter;
      EquationVertex eq = *iter;
      if (out_degree(eq, _graph) == 1) {
        Edge e = *out_edges(eq, _graph).first;
        UnknownVertex unknown = target(e, _graph);
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
      } else if (out_degree(eq, _graph) == 0) {
        cout << "Causalizing " << _graph[eq].eqs.front() << std::endl;
        ERROR("Problem is singular. Not supported yet.\n");
      }
    }

    auxiliaryIter = _unknownVertices.begin();
    for(iter = auxiliaryIter; iter != _unknownVertices.end(); iter = auxiliaryIter) {
      ++auxiliaryIter;
      UnknownVertex unknown = *(iter);
      if(out_degree(unknown, _graph) == 1) {
        Edge e = *out_edges(unknown, _graph).first;
        EquationVertex eq = target(e, _graph);
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

  
  // Add new functions
  foreach_(ClassType ct, cl) {
    Class c = get<Class>(ct);
    _mmo_class.types_ref().push_back(c.name());
    MMO_Class *mmo = new MMO_Class(c);
    _mmo_class.tyTable_ref().insert(c.name(), Type::Class(c.name(),mmo));
  }
  if (c_code.size()) { 
    std::fstream fs (c_path.c_str() , std::fstream::out);  
     fs << "#include <gsl/gsl_multiroots.h>\n";
    fs << "#define pre(X) X\n";
    foreach_(std::string s, c_code) 
        fs << s;
    fs.close();
  }
}

void CausalizationStrategy::makeCausalBegining(EquationList eqs, ExpList unknowns) {
  EquationList causalEqs = EquationSolver::solve(eqs, unknowns, _mmo_class.syms_ref(),c_code,cl, c_path);
  foreach_(Equation e, causalEqs) {
    _causalEqsBegining.push_back(e);
  }
}

void CausalizationStrategy::makeCausalEnd(EquationList eqs, ExpList unknowns) {
  EquationList causalEqs = EquationSolver::solve(eqs, unknowns, _mmo_class.syms_ref(),c_code,cl, c_path );
  foreach_(Equation e, causalEqs) {
    _causalEqsEnd.insert(_causalEqsEnd.begin(),e);
  }
}

/**
 * Applies tarjan algorithm
 */
void CausalizationStrategy::makeCausalMiddle() {

  std::map<int, Component> components;

  int n_comps = apply_tarjan(_graph, components);

  for (int i = 0; i < n_comps; i++) {

    Component component = components[i];

    std::list<UnknownVertex> *uVertices = component->uVertices;
    ExpList unknowns;
    std::list<UnknownVertex>::iterator uIt;
    for (uIt = uVertices->begin(); uIt != uVertices->end(); uIt++) {
      UnknownVertex v = *uIt;
      ExpList _unknowns = _graph[v].unknowns;
      Expression unknown = _unknowns.front();
      unknowns.push_back(unknown);
    }

    std::list<EquationVertex> *eqVertices = component->eqVertices;
    EquationList eqs;
    std::list<EquationVertex>::iterator eqIt;
    for (eqIt = eqVertices->begin(); eqIt != eqVertices->end(); eqIt++) {
      EquationVertex v = *eqIt;
      EquationList _eqs = _graph[v].eqs;
      Equation eq = _eqs.front();
      eqs.push_back(eq);
    }

    EquationList causalEqs = EquationSolver::solve(eqs, unknowns, _mmo_class.syms_ref(),c_code,cl, c_path );

    _causalEqsMiddle.insert(_causalEqsMiddle.end(), causalEqs.begin(), causalEqs.end());
  }
}
}
