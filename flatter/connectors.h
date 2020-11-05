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
#include <util/graph/graph_printer.h>
#include <util/table.h>
#include <util/type.h>

using namespace Modelica;
using namespace Modelica::AST;
using namespace SBG;

typedef Option<ExpList> ExpOptList; 

struct VertexNameTable : public SymbolTable<MultiInterval, Name>{
  VertexNameTable(){}
};

struct NameVertexTable : public SymbolTable<Name, MultiInterval>{
  NameVertexTable(){}
};

class Connectors{
  public:
  Connectors(MMO_Class &c);

  void debug(std::string filename);

  void solve();
  bool createGraph(EquationList &eqs);
  bool connect(Connect co);
  Pair<Name, ExpOptList> separate(Expression e);
  MultiInterval createVertex(Name n);
  bool checkRanges(ExpOptList range1, ExpOptList range2);
  Option<SetEdgeDesc> existsEdge(SetVertexDesc d1, SetVertexDesc d2);
  void updateGraph(SetVertexDesc d1, SetVertexDesc d2, MultiInterval mi1, MultiInterval mi2);
  void generateCode(PWLMap pw);
  OrdCT<NI1> getOff(MultiInterval mi);
  Pair<vector<Name>, vector<Name>> separateVars();
  bool isFlowVar(Name n);
  vector<Pair<Name, Name>> getVars(vector<Name> vs, Set sauxi);
  Pair<ExpList, bool> transMulti(MultiInterval mi1, MultiInterval mi2, ExpList nms, bool forFlow);
  MultiInterval applyOff(MultiInterval mi, OrdCT<NI1> off);
  EquationList simplifyCode(EquationList &eql);
  //ExpList lmToExpList(LMap lm, ExpList vs);

  private:
  SBGraph G;
  member_(vector<NI1>, vCount);
  member_(vector<NI1>, eCount1);
  member_(int, eCount2);
  member_(MMO_Class, mmoclass);
  member_(VertexNameTable, vnmtable);
  member_(NameVertexTable, nmvtable);
  //member_(EquationList, oldeqs);
  static EquationList oldeqs;
  static EquationList::iterator itold;
};

#endif
