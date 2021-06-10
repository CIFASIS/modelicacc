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
#include <util/graph/graph_definition.h>
#include <util/graph/graph_algorithms.h>
#include <util/graph/graph_printer.h>
#include <util/table.h>
#include <util/type.h>

using namespace Modelica;
using namespace Modelica::AST;
using namespace SBG;

typedef Option<ExpList> ExpOptList;

struct VertexNameTable : public SymbolTable<MultiInterval, Name> {
  VertexNameTable() {}
};

struct NameVertexTable : public SymbolTable<Name, MultiInterval> {
  NameVertexTable() {}
};

class Connectors {
  public:
  Connectors(MMO_Class &c);

  void debug(std::string filename);

  void solve();

  protected:
  void init();

  Real getValue(Expression exp);
  Set buildSet(MultiInterval mi);
  Set buildSet(VarInfo v);
  Option<SetVertexDesc> createVertex(Name n);

  MultiInterval buildEdgeMultiInterval(VarInfo v, int offset);
  MultiInterval fillDims(MultiInterval mi, int olddim, int dim);
  int locateCounterDimension(ExpOptList r, Name nm);
  Set createEdgeDom(ExpOptList r);
  LMap createLM(MultiInterval mi1, MultiInterval mi2);
  MultiInterval subscriptMI(MultiInterval mi, ExpOptList r);
  PWLMap createEdgeMap(Set dom, Set im, ExpOptList r);
  bool existsEdge(Name nm);
  void createEdge(ExpOptList r1, ExpOptList r2, SetVertexDesc V1, SetVertexDesc V2);

  bool checkLinearBase(Expression e);
  bool checkLinear(Expression e);
  bool checkLinearList(ExpList expl);
  bool checkCounters(ExpList l1, ExpList l2);
  bool checkSubscripts(ExpOptList range1, ExpOptList range2);

  Pair<Name, ExpOptList> separate(Expression e);
  bool connect(Connect co);

  bool checkIndependentCounters(Name n, Expression e);
  Pair<bool, EquationList> createGraph(EquationList &eqs);

  member_(PWLMap, ccG); // Connected components of SBG G

  private:
  member_(MMO_Class, mmoclass); // Micro Modelica model

  member_(SBGraph, G); // Connect Graph. Variables and edges are labeled with variables names
  member_(int, maxdim);
  member_(vector<NI1>, vCount); // Vertices count in each dimension
  member_(vector<NI1>, eCount); // Edges count in each dimension
  member_(int, ECount); // Set-edges count. Used to name set edges 

  member_(EquationList, notConnectEqs); // List "non-connect" equations
  member_(EquationList::iterator, itNotConn);

  member_(UnordCT<Name>, varsNms); // All var names in the mmoclass
  member_(OrdCT<Name>, counters); // Helper that saves counters
};

#endif
