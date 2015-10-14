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
private:
  void makeCausalBegining(Modelica::AST::EquationList eqs, Modelica::AST::ExpList unknowns);
  void makeCausalMiddle();
  void makeCausalEnd(Modelica::AST::EquationList eqs, Modelica::AST::ExpList unknowns);
  CausalizationGraph _graph;
  std::list<Vertex> _eqVertices;
  std::list<Vertex> _unknownVertices;
  std::string c_path;
  Modelica::MMO_Class &_mmo_class;
  Modelica::AST::EquationList _causalEqsBegining;
  Modelica::AST::EquationList _causalEqsMiddle;
  Modelica::AST::EquationList _causalEqsEnd;
  Modelica::AST::ClassList _cl;
  Modelica::AST::ExpList _all_unknowns;
  std::list<std::string> c_code;
  ClassList cl;
};


