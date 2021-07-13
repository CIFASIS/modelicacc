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
MatchingGraphBuilder::MatchingGraphBuilder(MMO_Class &mmo_class) : _mmo_class(mmo_class), _U(), _F(), _eq_usage() {}

Real MatchingGraphBuilder::getValue(Expression exp)
{
  VarSymbolTable symbols = _mmo_class.syms();
  EvalExpression eval_exp(symbols);
  return Apply(eval_exp, exp);
}

void MatchingGraphBuilder::addDims(size_t max_dim, size_t exp_dim, SBG::MultiInterval& intervals, int offset)
{
  if (max_dim > exp_dim) {
    for (size_t i = exp_dim; i < max_dim; i++) {
      Interval interval(offset, 1, offset);
      intervals.addInter(interval);
    }
  }      
}

void MatchingGraphBuilder::addDims(size_t max_dim, size_t exp_dim, contNI2& constant, contNI2& slope)
{
  contNI2::iterator constant_it = constant.end();
  contNI2::iterator slope_it = slope.end();
  if (max_dim > exp_dim) {
    for (size_t i = exp_dim; i < max_dim; i++) {
      constant_it = constant.insert(constant_it, 0);
      slope_it = slope.insert(slope_it, 0);
    }
  }
}

SBG::Set MatchingGraphBuilder::buildSet(MultiInterval intervals)
{
  SBG::Set set;
  AtomSet node_set(intervals);
  set.addAtomSet(node_set);
  return set;
}

SBG::Set MatchingGraphBuilder::buildSet(VarInfo variable, int offset, size_t max_dim)
{
  Option<ExpList> dims = variable.indices();
  MultiInterval variable_intervals;
  if (dims) {
    foreach_(Expression d, dims.get()) {
      Real res = getValue(d) - 1;
      Interval interval(offset, 1, offset + res);
      variable_intervals.addInter(interval);
    }
    addDims(max_dim, dims->size(), variable_intervals, offset);
  } else {
    addDims(max_dim, 0, variable_intervals, offset);
  }
  return buildSet(variable_intervals);
}

SBG::Set MatchingGraphBuilder::buildSet(Equation eq, string eq_id, int offset, size_t max_dim)
{
  Usage usage;
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
      Real end = offset + upper - lower;
      Interval interval(offset, step, end);
      equation_intervals.addInter(interval);
      usage[idx.name()] = lower;
    }
    addDims(max_dim, indexes.size(), equation_intervals, offset);
    _eq_usage[eq_id] = usage;
  } else {
    addDims(max_dim, 0, equation_intervals, offset);
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
SBG::Set MatchingGraphBuilder::generateMapDom(SBG::Set dom, SBG::Set unk_dom, int offset, size_t max_dim)
{
  MultiInterval edge_set_intervals;
  SBG::contAS atom_sets = dom.asets_(); 
  foreach_(AtomSet atom_set, atom_sets) {
    MultiInterval dom_intervals = atom_set.aset_();
    foreach_(Interval inter, dom_intervals.inters_()) {
      Real end = inter.card() + offset - 1;
      edge_set_intervals.addInter(Interval(offset, inter.step(), end));
    }
    addDims(max_dim, dom_intervals.inters_().size(), edge_set_intervals, offset);
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

void MatchingGraphBuilder::addEquation(Equation eq, string id, SBG::Set set, SBGraph& graph)
{
  if (!is<Equality>(eq)) {
    ERROR("Only Equality equations supported.");
  }
  _F.push_back(make_pair(addVertex(id, set, graph),get<Equality>(eq)));  
}

PWLMap MatchingGraphBuilder::buildPWLMap(contNI2 constants, contNI2 slopes, SBG::Set dom)
{
  LMap lmap(slopes, constants);

  OrdCT<Set> doms;
  doms.insert(doms.end(), dom);
  
  OrdCT<LMap> maps;
  maps.insert(maps.end(), lmap); 
  PWLMap map(doms, maps);
  return map;
}

MatchingGraphBuilder::MatchingMaps MatchingGraphBuilder::generatePWLMaps(Expression exp, SBG::Set dom, SBG::Set unk_dom, int offset, string eq_id, size_t max_dim)
{
  assert(is<Reference>(exp));
  VarSymbolTable symbols = _mmo_class.syms();
  contNI2 constant_pwl_map_u;
  contNI2::iterator constant_pwl_map_u_it = constant_pwl_map_u.begin();
  contNI2 slope_pwl_map_u;
  contNI2::iterator slope_pwl_map_u_it = slope_pwl_map_u.begin();
  contNI2 constant_pwl_map_f;
  contNI2::iterator constant_pwl_map_f_it = constant_pwl_map_f.begin();
  contNI2 slope_pwl_map_f;
  contNI2::iterator slope_pwl_map_f_it = slope_pwl_map_f.begin();
  
  Reference occur = get<Reference>(exp);   
  ExpList indexes = get<1>(occur.ref().front());
  SBG::contNI1 init_elems = unk_dom.minElem();
  assert(init_elems.size() == indexes.size() || indexes.size() == 0);
  SBG::contNI1::iterator min_elem = init_elems.begin();
  SBG::Set map_dom = generateMapDom(dom, unk_dom, offset, max_dim);
  int map_offset = offset - 1;

  foreach_(Expression idx, indexes)
  {
    NI1 set_vertex_init = *min_elem -1;
    PWLMapValues pwl_map_values(symbols);
    Apply(pwl_map_values, idx);
    Usage usage = _eq_usage[eq_id];
    int range_init_value = usage[pwl_map_values.variable()];
    int set_vertex_offset = pwl_map_values.slope() * offset;
    int map_first_value = pwl_map_values.constant() + pwl_map_values.slope() * range_init_value + set_vertex_init;
    constant_pwl_map_u_it = constant_pwl_map_u.insert(constant_pwl_map_u_it, map_first_value - set_vertex_offset);
    slope_pwl_map_u_it = slope_pwl_map_u.insert(slope_pwl_map_u_it, pwl_map_values.slope());
    min_elem++; 
  }
  if (indexes.empty()) { // Scalar variable.
    NI1 set_vertex_init = *min_elem -1;
    constant_pwl_map_u_it = constant_pwl_map_u.insert(constant_pwl_map_u_it, - map_offset + set_vertex_init);
    slope_pwl_map_u_it = slope_pwl_map_u.insert(slope_pwl_map_u_it, 1);
    addDims(max_dim, 1, constant_pwl_map_u, slope_pwl_map_u);
  } else { 
    addDims(max_dim, indexes.size(), constant_pwl_map_u, slope_pwl_map_u);
  }
  for (NI1 init : dom.minElem()) {
    NI1 set_vertex_init = init -1;
    constant_pwl_map_f_it = constant_pwl_map_f.insert(constant_pwl_map_f_it, -map_offset + set_vertex_init);
    slope_pwl_map_f_it = slope_pwl_map_f.insert(slope_pwl_map_f_it, 1);
  }
  addDims(max_dim, dom.minElem().size(), constant_pwl_map_f, slope_pwl_map_f);
  return make_pair(buildPWLMap(constant_pwl_map_f, slope_pwl_map_f, map_dom), buildPWLMap(constant_pwl_map_u, slope_pwl_map_u, map_dom));
}

SBGraph MatchingGraphBuilder::makeGraph()
{
  SBGraph graph;
  VarSymbolTable symbols = _mmo_class.syms();

  IdentList variables = _mmo_class.variables();
  size_t max_dim = 1;
  // Get max dim defined in the model.
  foreach_(Name var_name, variables)
  {
    VarInfo variable = symbols[var_name].get();
    if (!isConstant(var_name, symbols) && !isBuiltIn(var_name, symbols) && !isDiscrete(var_name, symbols) && !isParameter(var_name, symbols)) {
      Option<ExpList> indexes = variable.indices(); 
      if (indexes && indexes->size() > max_dim) {
        max_dim = indexes->size();
      }
    }
  }

  int set_vertex_offset = 1;
  // Build unknown nodes.
  foreach_(Name var_name, variables)
  {
    VarInfo variable = symbols[var_name].get();
    if (!isConstant(var_name, symbols) && !isBuiltIn(var_name, symbols) && !isDiscrete(var_name, symbols) && !isParameter(var_name, symbols)) {
      SBG::Set vertex_dom = buildSet(variable, set_vertex_offset, max_dim);
      _U.push_back(addVertex(var_name, vertex_dom, graph));
      set_vertex_offset += vertex_dom.size();
    }
  }

  // Build equation nodes.
  EquationList eqs = _mmo_class.equations().equations();
  int id = 1;
  foreach_(Equation eq, eqs)
  {
    SBG::Set vertex_dom;
    if (is<ForEq>(eq)) {   
      ForEq for_eq = get<ForEq>(eq);
      vector<Equation>  for_eqs = for_eq.elements();
      foreach_(Equation for_el, for_eqs)  
      {
        string eq_name = "eq_" + to_string(id++);
        vertex_dom = buildSet(eq, eq_name, set_vertex_offset, max_dim);
        set_vertex_offset += vertex_dom.size();
        addEquation(for_el, eq_name, vertex_dom, graph);
      }
    } else if (is<Equality>(eq)) {
      string eq_name = "eq_" + to_string(id++);
      vertex_dom = buildSet(eq, eq_name, set_vertex_offset, max_dim);
      set_vertex_offset += vertex_dom.size();
      addEquation(eq, eq_name, vertex_dom, graph);
    } else {
      ERROR("Only causalization of for and equality equations");
    }
  }

  // Build edges.
  int set_edge_offset = 1;
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
      SBG::Set unk_dom = unknown_vertex.vs_();       
      MatchingExps matching_exps(unknown_vertex.name);
      Apply(matching_exps, left);
      Apply(matching_exps, right);
      set<Expression> matched_exps = matching_exps.matchedExps();
      cout << "Matched exps for: " << unknown_vertex.name << " in " << eq_desc.second << endl;
      cout << "Equation dom: " << dom << endl;
      foreach_(Expression exp, matched_exps)
      {
        cout << "Expression: " << exp << endl;
        MatchingMaps maps = generatePWLMaps(exp, dom, unk_dom, set_edge_offset, eq_vertex.name, max_dim); 
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
