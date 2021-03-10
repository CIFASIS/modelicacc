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
#include <util/ast_visitors/eval_expression.h>
#include <util/ast_visitors/matching_exps.h>
#include <util/ast_visitors/pwl_map_values.h>

using namespace Modelica;
using namespace SBG;

namespace Causalize {
MatchingGraphBuilder::MatchingGraphBuilder(MMO_Class &mmo_class) : _mmo_class(mmo_class), _U(), _F() {}

Real MatchingGraphBuilder::getValue(Expression exp)
{
  VarSymbolTable symbols = _mmo_class.syms();
  EvalExpression eval_exp(symbols);
  return Apply(eval_exp, exp);
}

SBG::Set MatchingGraphBuilder::buildSet(MultiInterval intervals)
{
  SBG::Set set;
  AtomSet node_set(intervals);
  set.addAtomSet(node_set);
  return set;
}

SBG::Set MatchingGraphBuilder::buildSet(VarInfo variable)
{
  Option<ExpList> dims = variable.indices();
  MultiInterval variable_intervals;
  if (dims) {
    foreach_(Expression d, dims.get()) {
      Real res = getValue(d);
      Interval interval(1, 1, res);
      variable_intervals.addInter(interval);
    }
  } else {
    Interval interval(1, 1, 1);
    variable_intervals.addInter(interval);
  }
  return buildSet(variable_intervals);
}

SBG::Set MatchingGraphBuilder::buildSet(Equation eq)
{
  MultiInterval equation_intervals;
  if (is<ForEq>(eq)) {
    ForEq for_eq = get<ForEq>(eq);
    IndexList indexes = for_eq.range().indexes();
    foreach_(Index idx, indexes) {
      OptExp exp = idx.exp();
      if (!exp || !is<Range>(exp.get())) {
        ERROR("Only Range expressions supported");
      }
      Range range = get<Range>(exp.get());
      Real lower = getValue(range.start());
      Real step = 1;
      Real upper = getValue(range.end());
      if (range.step()) {
        step = getValue(range.step().get());
      }
      Interval interval(lower, step, upper);
      equation_intervals.addInter(interval);
    }
  } else {
    Interval interval(1, 1, 1);
    equation_intervals.addInter(interval);
  }
  return buildSet(equation_intervals);
}

/**
 * @brief Add the offset to a given equation domain.
 * 
 * @param dom 
 * @param offset 
 * @return SBG::Set The new set with all the offsets applied.
 */
SBG::Set MatchingGraphBuilder::generateMapDom(SBG::Set dom, int offset)
{
  MultiInterval edge_set_intervals;
  SBG::contAS atom_sets = dom.asets_(); 
  foreach_(AtomSet atom_set, atom_sets) {
    MultiInterval dom_intervals = atom_set.aset_();
    foreach_(Interval inter, dom_intervals.inters_()) {
      edge_set_intervals.addInter(Interval(inter.lo_()+offset, inter.step_(), inter.hi_()+offset));
    }
  }
  return buildSet(edge_set_intervals);
}

SetVertexDesc MatchingGraphBuilder::addVertex(std::string vertex_name, SBG::Set set, SBGraph& graph)
{
  SetVertex V(vertex_name, set);
  SetVertexDesc v = boost::add_vertex(graph); 
  graph[v] = V;
  return v;
}

void MatchingGraphBuilder::addEquation(Equation eq, int id, SBG::Set set, SBGraph& graph)
{
  if (!is<Equality>(eq)) {
    ERROR("Only Equality equations supported.");
  }
  string eq_name = "eq_" + to_string(id);
  _F.push_back(make_pair(addVertex(eq_name, set, graph),get<Equality>(eq)));  
}

PWLMap MatchingGraphBuilder::buildPWLMap(OrdCT<NI2> constants, OrdCT<NI2> slopes, SBG::Set dom)
{
  LMap lmap(slopes, constants);

  OrdCT<Set> doms;
  doms.insert(doms.end(), dom);
  
  OrdCT<LMap> maps;
  maps.insert(maps.end(), lmap); 
  PWLMap map(doms, maps);
  return map;
}

MatchingGraphBuilder::MatchingMaps MatchingGraphBuilder::generatePWLMaps(Expression exp, SBG::Set dom, int offset)
{
  assert(is<Reference>(exp));
  VarSymbolTable symbols = _mmo_class.syms();
  OrdCT<NI2> constant_pwl_map_u;
  OrdCT<NI2>::iterator constant_pwl_map_u_it = constant_pwl_map_u.begin();
  OrdCT<NI2> slope_pwl_map_u;
  OrdCT<NI2>::iterator slope_pwl_map_u_it = slope_pwl_map_u.begin();
  OrdCT<NI2> constant_pwl_map_f;
  OrdCT<NI2>::iterator constant_pwl_map_f_it = constant_pwl_map_f.begin();
  OrdCT<NI2> slope_pwl_map_f;
  OrdCT<NI2>::iterator slope_pwl_map_f_it = slope_pwl_map_f.begin();
  
  Reference occur = get<Reference>(exp);   
  ExpList indexes = get<1>(occur.ref().front());
  SBG::Set map_dom = generateMapDom(dom, offset);
  foreach_(Expression idx, indexes)
  {
    PWLMapValues pwl_map_values(symbols);
    Apply(pwl_map_values, idx);
    constant_pwl_map_u_it = constant_pwl_map_u.insert(constant_pwl_map_u_it, pwl_map_values.constant() - pwl_map_values.slope() * offset);
    slope_pwl_map_u_it = slope_pwl_map_u.insert(slope_pwl_map_u_it, pwl_map_values.slope());
    constant_pwl_map_f_it = constant_pwl_map_f.insert(constant_pwl_map_f_it, -offset);
    slope_pwl_map_f_it = slope_pwl_map_f.insert(slope_pwl_map_f_it, 1); 
  }
  if (indexes.empty()) { // Scalar variable.
    constant_pwl_map_u_it = constant_pwl_map_u.insert(constant_pwl_map_u_it, - offset);
    slope_pwl_map_u_it = slope_pwl_map_u.insert(slope_pwl_map_u_it, 1);
    constant_pwl_map_f_it = constant_pwl_map_f.insert(constant_pwl_map_f_it, - offset);
    slope_pwl_map_f_it = slope_pwl_map_f.insert(slope_pwl_map_f_it, 1); 
  }
  return make_pair(buildPWLMap(constant_pwl_map_f, slope_pwl_map_f, map_dom), buildPWLMap(constant_pwl_map_u, slope_pwl_map_u, map_dom));
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
      _U.push_back(addVertex(var_name, buildSet(variable), graph));
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
        addEquation(for_el, id++, range, graph);
      }
    } else if (is<Equality>(eq)) {
      addEquation(eq, id++, range, graph);
    } else {
      ERROR("Only causalization of for and equality equations");
    }
  }

  // Build edges.
  int set_edge_offset = 0;
  unsigned int edge_id = 1;
  foreach_(EquationDesc eq_desc, _F)
  {
    SetVertexDesc eq_vertex_desc = eq_desc.first; 
    SetVertex eq_vertex = graph[eq_vertex_desc];
    Expression left = eq_desc.second.left();
    Expression right = eq_desc.second.right();
    SBG::Set dom = eq_vertex.vs_(); 
    foreach_(SetVertexDesc unknown_vertex_desc, _U)
    {
      SetVertex unknown_vertex = graph[unknown_vertex_desc];
      MatchingExps matching_exps(unknown_vertex.name);
      Apply(matching_exps, left);
      Apply(matching_exps, right);
      list<Expression> matched_exps = matching_exps.matchedExps();
      cout << "Matched exps for: " << unknown_vertex.name << " in " << eq_desc.second << endl;
      cout << "Equation dom: " << dom << endl;
      foreach_(Expression exp, matched_exps)
      {
        cout << "Expression: " << exp << endl;
        MatchingMaps maps = generatePWLMaps(exp, dom, set_edge_offset); 
        set_edge_offset += dom.size();
        string edge_name = "E_" + to_string(edge_id++);
        cout << "MapF: " << maps.first << endl;
        cout << "MapU: " << maps.second << endl;
        SetEdge edge(edge_name, maps.first, maps.second);
        SetEdgeDesc e;
        bool b;
        boost::tie(e, b) = boost::add_edge(eq_vertex_desc, unknown_vertex_desc, graph);
        graph[e] = edge;
      } 
    }
  }
  return graph;
}

}  // namespace Causalize
