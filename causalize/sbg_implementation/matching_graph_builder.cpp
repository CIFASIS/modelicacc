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

#include <ast/queries.h>
#include <causalize/sbg_implementation/matching_graph_builder.h>
#include <util/ast_visitors/matching_exps.h>

using namespace Modelica;
using namespace SBG;

namespace Causalize {
MatchingGraphBuilder::MatchingGraphBuilder(MMO_Class &mmo_class) : _mmo_class(mmo_class) {}

SBG::Set MatchingGraphBuilder::buildSet(VarInfo variable)
{
  SBG::Set set;
  return set;
}

SBG::Set MatchingGraphBuilder::buildSet(Equation eq)
{
  SBG::Set set;
  return set;
}

SetVertexDesc MatchingGraphBuilder::addVertex(std::string vertex_name, SBG::Set set, SBGraph& graph)
{
  SetVertex V(vertex_name, set);
  SetVertexDesc v = boost::add_vertex(graph); 
  graph[v] = V;
  return v;
}

void MatchingGraphBuilder::addEquation(int id, SBG::Set set, SBGraph& graph)
{
  stringstream eq_name;
  eq_name << "eq_" << id;
  _equation_nodes.push_back(addVertex(eq_name.str(), set, graph));  
}

SBGraph MatchingGraphBuilder::makeGraph()
{
  SBGraph graph;
  VarSymbolTable symbols = _mmo_class.syms();

  IdentList variables = _mmo_class.variables();
  // Build unknown nodes.
  foreach_(Name var_name, variables)
  {
    VarInfo variable = symbols[var_name].get();
    if (!isConstant(var_name, symbols) && !isBuiltIn(var_name, symbols) && !isDiscrete(var_name, symbols) && !isParameter(var_name, symbols)) {
      _var_nodes.push_back(addVertex(var_name, buildSet(variable), graph));
    }
  }

  // Build equation nodes.
  EquationList eqs = _mmo_class.equations().equations();
  int id = 1;
  foreach_(Equation eq, eqs)
  {
    SBG::Set range;
    range = buildSet(eq);
    if (is<ForEq>(eq)) {   
      ForEq for_eq = get<ForEq>(eq);
      vector<Equation>  for_eqs = for_eq.elements();
      foreach_(Equation for_el, for_eqs)  
      {
        addEquation(id++, range, graph);
      }
    } else if (is<Equality>(eq)) {
      addEquation(id++, range, graph);
    } else {
      ERROR("Only causalization of for and equality equations");
    }
  }
  return graph;
}

}  // namespace Causalize
