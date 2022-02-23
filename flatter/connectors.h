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

#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <boost/type_traits/remove_cv.hpp>
#include <boost/variant/get.hpp>
#include <boost/variant/static_visitor.hpp>
#include <set>

#include <ast/ast_types.h>
#include <ast/element.h>
#include <ast/equation.h>
#include <ast/expression.h>
#include <ast/statement.h>
#include <flatter/class_finder.h>
#include <mmo/mmo_class.h>

#include <util/ast_visitors/replace_expression.h>
#include <util/ast_visitors/constant_expression.h>
#include <sbg/sbg.hpp>
#include <sbg/sbg_algorithms.hpp>
#include <sbg/sbg_printer.hpp>
#include <util/table.h>
#include <util/type.h>

using namespace Modelica;
using namespace Modelica::AST;
using namespace SBG;

typedef Option<ExpList> ExpOptList;

struct VarsDimsTable : public SymbolTable<Name, int> {
  VarsDimsTable() {}
};


class Connectors {
  public:
  Connectors(MMO_Class &c);

  void debug(std::string filename);

  void solve();

  protected:
  // Initialization
  bool isFlowVar(Name v);
  set<Name> getByPrefix(Name n);
  void addVar(Name n, VarInfo vi);
  bool init();

  // Set-vertex creation
  Option<VarInfo> addConnectorVars(Name n);
  Real getValue(Expression exp);
  Set buildSet(MultiInterval mi);
  Set buildSet(VarInfo v);
  Option<SetVertexDesc> buildVertex(Name n);

  // Set-edge creation
  MultiInterval buildEdgeMultiInterval(VarInfo v, int offset);
  MultiInterval fillDims(MultiInterval mi, int olddim, int dim);
  int locateCounterDimension(ExpOptList r, Name nm);
  Set buildEdgeDom(ExpOptList r);
  LMap buildLM(MultiInterval mi1, MultiInterval mi2);
  MultiInterval subscriptMI(MultiInterval mi, ExpOptList r);
  PWLMap buildEdgeMap(Set dom, Set im, ExpOptList r);
  bool existsEdge(Name nm);
  void buildEdge(ExpOptList r1, ExpOptList r2, SetVertexDesc V1, SetVertexDesc V2);

  // Check subscripts restrictions
  bool checkLinearBase(Expression e);
  bool checkLinear(Expression e);
  bool checkLinearList(ExpList expl);
  bool checkCounters(ExpList l1, ExpList l2);
  bool checkSubscripts(ExpOptList range1, ExpOptList range2);

  // Deal with connectos
  Pair<Name, ExpOptList> separate(Expression e);
  bool connect(Connect co);

  // Graph creation
  bool checkIndependentCounters(ForEq feq);
  void buildDisconnected();
  Pair<bool, EquationList> buildConnects(EquationList &eqs);
  Pair<bool, EquationList> buildGraph(EquationList &eqs);

  // Code generation helpers
  Name getName(MultiInterval as);
  MultiInterval getAtomSet(MultiInterval as);
  bool isIdMap(LMap lm);
  Indexes buildIndex(Set connected);
  vector<Name> getEffVars(Set connector);
  vector<Name> getFlowVars(Set connector);
  Set getRepd(MultiInterval atomRept);
  ExpList buildSubscripts(Indexes indexes, MultiInterval original, MultiInterval as, int dims);
  ExpList buildRanges(MultiInterval original, MultiInterval as);
  ExpList buildLoopExpr(Indexes indexes, MultiInterval as, vector<Name> vars);
  ExpList buildAddExpr(MultiInterval atomRept, MultiInterval as);
  EquationList buildLoop(Indexes indexes, EquationList eqs);

  // Effort and flow equations
  EquationList buildEffEquations(Indexes indexes, MultiInterval atomRept, Set repd);
  EquationList buildFlowEquations(Indexes indexes, MultiInterval atomRept, Set repd);

  // Code generation
  EquationList generateCode();
  EquationList simplifyCode(EquationList eql);

  private:
  member_(MMO_Class, mmoclass); // Micro Modelica model

  member_(SBGraph, G); // Connect Graph. Variables and edges are labeled with variables names
  member_(PWLMap, ccG); // Connected components of SBG G
  member_(int, maxdim); // Number of dimensions of the variable with maximum dimension in the model
  member_(vector<INT>, vCount); // Vertices count in each dimension
  member_(vector<INT>, eCount); // Edges count in each dimension
  member_(int, ECount); // Set-edges count. Used to name set edges 

  member_(EquationList, notConnectEqs); // List "non-connect" equations

  member_(set<Name>, varsNms); // All var names in the mmoclass
  member_(set<Name>, negVars); // Connectors in a connect whose sign is negative
  member_(set<Name>, effVars); // Names of effort variables
  member_(set<Name>, flowVars); // Names of flow variables

  member_(vector<Name>, counters); // Helper that saves counters
  member_(ExpList, countersCG); // Helper that saves counters names for code generation

  member_(VarsDimsTable, varsDims); // Save number of dimensions of each variable
};

#endif
