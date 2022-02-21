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

void MatchingGraphBuilder::addDims(size_t max_dim, size_t exp_dim, ORD_REALS& constant, ORD_REALS& slope)
{
  ORD_REALS::iterator constant_it = constant.end();
  ORD_REALS::iterator slope_it = slope.end();
  if (max_dim > exp_dim) {
    for (size_t i = exp_dim; i < max_dim; i++) {
      constant_it = constant.insert(constant_it, 0);
      slope_it = slope.insert(slope_it, 0);
    }
  }
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
  return createSet(variable_intervals);
}

SBG::Set MatchingGraphBuilder::buildSet(Equation eq, std::string eq_id, int offset, size_t max_dim)
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
  return createSet(equation_intervals);
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
  SBG::UNORD_MI atom_sets = dom.asets(); 
  foreach_(MultiInterval dom_intervals, atom_sets) {
    foreach_(Interval inter, dom_intervals.inters()) {
      Real end = inter.card() + offset - 1;
      edge_set_intervals.addInter(Interval(offset, inter.step(), end));
    }
    addDims(max_dim, dom_intervals.inters().size(), edge_set_intervals, offset);
  }
  return createSet(edge_set_intervals);
}

SetVertexDesc MatchingGraphBuilder::addVertex(std::string vertex_name, SBG::Set set, SBGraph& graph)
{
  SetVertex V(vertex_name, num_vertices(graph) + 1, set, 0);
  SetVertexDesc v = boost::add_vertex(graph); 
  graph[v] = V;
  return v;
}

void MatchingGraphBuilder::addEquation(Equation eq, std::string id, SBG::Set set, SBGraph& graph)
{
  if (!is<Equality>(eq)) {
    ERROR("Only Equality equations supported.");
  }
  _F.push_back(std::make_pair(addVertex(id, set, graph),get<Equality>(eq)));  
}

PWLMap MatchingGraphBuilder::buildPWLMap(ORD_REALS constants, ORD_REALS slopes, SBG::Set dom)
{
  LMap lmap(slopes, constants);

  OrdCT<Set> doms;
  doms.insert(doms.end(), dom);
  
  OrdCT<LMap> maps;
  maps.insert(maps.end(), lmap); 
  PWLMap map(doms, maps);
  return map;
}

MatchingGraphBuilder::MatchingMaps MatchingGraphBuilder::generatePWLMaps(Expression exp, SBG::Set dom, SBG::Set unk_dom, int offset, std::string eq_id, size_t max_dim)
{
  assert(is<Reference>(exp));
  VarSymbolTable symbols = _mmo_class.syms();
  PWLMapValues pwl_map_values(symbols);
  ORD_REALS constant_pwl_map_u;
  ORD_REALS::iterator constant_pwl_map_u_it = constant_pwl_map_u.begin();
  ORD_REALS slope_pwl_map_u;
  ORD_REALS::iterator slope_pwl_map_u_it = slope_pwl_map_u.begin();
  ORD_REALS constant_pwl_map_f;
  ORD_REALS::iterator constant_pwl_map_f_it = constant_pwl_map_f.begin();
  ORD_REALS slope_pwl_map_f;
  ORD_REALS::iterator slope_pwl_map_f_it = slope_pwl_map_f.begin();
  
  Reference occur = get<Reference>(exp);   
  ExpList indexes = get<1>(occur.ref().front());
  SBG::ORD_INTS init_elems = unk_dom.minElem();
  assert(init_elems.size() == indexes.size() || indexes.size() == 0);
  SBG::ORD_INTS::iterator min_elem = init_elems.begin();
  SBG::Set map_dom = generateMapDom(dom, unk_dom, offset, max_dim);
  int map_offset = offset - 1;

  foreach_(Expression idx, indexes)
  {
    SBG::INT set_vertex_init = *min_elem;
    Apply(pwl_map_values, idx);
    Usage usage = _eq_usage[eq_id];
    //int set_vertex_offset = pwl_map_values.slope() * offset;
    //int set_vertex_offset = (offset - set_vertex_init) + map_first_value;
    int range_init_value = usage[pwl_map_values.variable()];
    int map_first_value = pwl_map_values.constant() + pwl_map_values.slope() * range_init_value + set_vertex_init - 1;
    int off = map_first_value - pwl_map_values.slope() * offset;
    constant_pwl_map_u_it = constant_pwl_map_u.insert(constant_pwl_map_u_it, off);
    slope_pwl_map_u_it = slope_pwl_map_u.insert(slope_pwl_map_u_it, pwl_map_values.slope());
    min_elem++; 
  }
//  BOOST_FOREACH (REAL c, constant_pwl_map_u)
//    std::cout << c << " ";
//  std::cout << "\n"; 
  if (indexes.empty()) { // Scalar variable.
    SBG::INT set_vertex_init = *min_elem -1;
    constant_pwl_map_u_it = constant_pwl_map_u.insert(constant_pwl_map_u_it, - map_offset + set_vertex_init);
    slope_pwl_map_u_it = slope_pwl_map_u.insert(slope_pwl_map_u_it, 1);
    addDims(max_dim, 1, constant_pwl_map_u, slope_pwl_map_u);
  } else { 
    addDims(max_dim, indexes.size(), constant_pwl_map_u, slope_pwl_map_u);
  }
  for (SBG::INT init : dom.minElem()) {
    SBG::INT set_vertex_init = init -1;
    constant_pwl_map_f_it = constant_pwl_map_f.insert(constant_pwl_map_f_it, -map_offset + set_vertex_init);
    slope_pwl_map_f_it = slope_pwl_map_f.insert(slope_pwl_map_f_it, 1);
  }
  addDims(max_dim, dom.minElem().size(), constant_pwl_map_f, slope_pwl_map_f);
  return std::make_pair(buildPWLMap(constant_pwl_map_f, slope_pwl_map_f, map_dom), buildPWLMap(constant_pwl_map_u, slope_pwl_map_u, map_dom));
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
      set_vertex_offset += vertex_dom.card();
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
      std::vector<Equation>  for_eqs = for_eq.elements();
      foreach_(Equation for_el, for_eqs)  
      {
        std::string eq_name = "eq_" + std::to_string(id++);
        vertex_dom = buildSet(eq, eq_name, set_vertex_offset, max_dim);
        set_vertex_offset += vertex_dom.card();
        addEquation(for_el, eq_name, vertex_dom, graph);
      }
    } else if (is<Equality>(eq)) {
      std::string eq_name = "eq_" + std::to_string(id++);
      vertex_dom = buildSet(eq, eq_name, set_vertex_offset, max_dim);
      set_vertex_offset += vertex_dom.card();
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
    SBG::Set dom = eq_vertex.range(); 
    foreach_(SetVertexDesc unknown_vertex_desc, _U)
    {
      SetVertex unknown_vertex = graph[unknown_vertex_desc];
      SBG::Set unk_dom = unknown_vertex.range();       
      MatchingExps matching_exps(unknown_vertex.name(), isState(unknown_vertex.name(), symbols));
      Apply(matching_exps, left);
      Apply(matching_exps, right);
      std::set<Expression> matched_exps = matching_exps.matchedExps();
      std::cout << "Matched exps for: " << unknown_vertex.name() << " in " << eq_desc.second << std::endl;
      std::cout << "Equation dom: " << dom << std::endl;
      foreach_(Expression exp, matched_exps)
      {
        std::cout << "Expression: " << exp << std::endl;
        MatchingMaps maps = generatePWLMaps(exp, dom, unk_dom, set_edge_offset, eq_vertex.name(), max_dim); 
        set_edge_offset += dom.card();
        std::string edge_name = "E_" + std::to_string(edge_id++);
        std::cout << "MapF: " << maps.first << std::endl;
        std::cout << "MapU: " << maps.second << std::endl;
        SetEdge edge(edge_name, num_edges(graph) + 1, maps.first, maps.second, 0);
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
