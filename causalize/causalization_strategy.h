/*
 * causalization_strategy.h
 *
 *  Created on: 12/05/2013
 *      Author: fede moya
 */

#include <causalize/graph/graph_definition.h>
#include <mmo/mmo_class.h>


namespace Causalize {
class CausalizationStrategy {
public:
  CausalizationStrategy(Modelica::MMO_Class &mmo_class);
  void causalize(Modelica::AST::Name name);
private:
  void makeCausalBegining(Modelica::AST::EquationList eqs, Modelica::AST::ExpList unknowns);
  void makeCausalMiddle();
  void makeCausalEnd(Modelica::AST::EquationList eqs, Modelica::AST::ExpList unknowns);
  Causalize::CausalizationGraph _graph;
  std::list<Causalize::EquationVertex> _eqVertices;
  std::list<Causalize::UnknownVertex> _unknownVertices;
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
}


