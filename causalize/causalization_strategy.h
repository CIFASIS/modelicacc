/*
 * causalization_strategy.h
 *
 *  Created on: 12/05/2013
 *      Author: fede
 */

#include <causalize/graph/graph_definition.h>
#include <mmo/mmo_class.h>


namespace Causalize {
class CausalizationStrategy {
public:
  CausalizationStrategy(Modelica::MMO_Class &mmo_class);
  void causalize(Modelica::AST::Name name);
  void causalize_simple(Modelica::AST::Name name);
  void causalize_tarjan(Modelica::AST::Name name);
private:
  void simpleCausalizationStrategy();
  Edge getUniqueEdge(Vertex v);
  void collectDegree1Verts(Vertex v, std::list<Vertex> &degree1Verts);
  void makeCausalBegining(Modelica::AST::Equation eq, Modelica::AST::Expression unknown);
  void makeCausalMiddle();
  void makeCausalEnd(Modelica::AST::Equation eq, Modelica::AST::Expression unknown);
  CausalizationGraph _graph;
  Modelica::MMO_Class &_mmo_class;
  Modelica::AST::EquationList _causalEqsBegining;
  Modelica::AST::EquationList _causalEqsMiddle;
  std::vector<Modelica::AST::Equation> _causalEqsEnd;
  int _causalEqsEndIndex;
  Modelica::AST::ClassList _cl;
  Modelica::AST::ExpList _all_unknowns;
  std::list<std::string> c_code;
};
}
