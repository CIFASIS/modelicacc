/*
 * causalization_strategy.h
 *
 *  Created on: 12/05/2013
 *      Author: fede
 */

#include <causalize/vector/graph_definition.h>
#include <mmo/mmo_class.h>


class CausalizationStrategy {
public:
  CausalizationStrategy(Modelica::MMO_Class &mmo_class);
  void causalize(Modelica::AST::Name name);
  void causalize_no_opt(Modelica::AST::Name name);
private:
  void simpleCausalizationStrategy();
  Edge getUniqueEdge(Vertex v);
  void makeCausalBegining(Modelica::AST::Equation eq, Modelica::AST::Expression unknown);
  void makeCausalMiddle();
  void makeCausalEnd(Modelica::AST::Equation eq, Modelica::AST::Expression unknown);
  CausalizationGraph _graph;
  Modelica::MMO_Class &_mmo_class;
  Modelica::AST::EquationList _causalEqsBegining;
  Modelica::AST::EquationList _causalEqsMiddle;
  Modelica::AST::EquationList _causalEqsEnd;
  Modelica::AST::ClassList _cl;
  Modelica::AST::ExpList _all_unknowns;
  std::list<std::string> c_code;
};


