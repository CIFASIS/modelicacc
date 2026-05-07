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

#include <algorithm>
#include <cmath>
#include <sstream>

#include <ast/queries.hpp>
#include <causalize/sbg_implementation/generate_sbg_input.hpp>
#include <util/ast_visitors/constant_expression.hpp>
#include <util/ast_visitors/eval_expression.hpp>
#include <util/ast_visitors/matching_exps.hpp>
#include <util/ast_visitors/pwl_map_values.hpp>
#include <util/logger.hpp>

namespace Modelica {

namespace Causalize {

// Constructors/Destructors ----------------------------------------------------

GenerateSBGInput::GenerateSBGInput(MMO_Class& mmo_class)
  : _mmo_class(mmo_class), _vertex_offset(1) {}

// Getters ---------------------------------------------------------------------

std::string GenerateSBGInput::fileName() { return _mmo_class.name() + "_sbg_input.sbg"; }

// Add variable vertices -------------------------------------------------------

Integer GenerateSBGInput::getValue(Expression expr) const
{
  VarSymbolTable symbols = _mmo_class.syms();
  EvalExpression eval_expr(symbols);
  return Integer(Apply(eval_expr, expr));
}

void GenerateSBGInput::buildSet(const VarInfo& variable, const Name& name)
{
  Option<ExpList> dims = variable.indices();
  SetVertex set_vertex{_node_id};
  if (dims) {
    for (const Expression& d : dims.get()) {
      Integer d_val = getValue(d);
      set_vertex.addDimension(_vertex_offset, 1, _vertex_offset + d_val - 1);
    }
    _vertex_offset += set_vertex.maxDimSize() + 1;
  } else {
    set_vertex.addDimension(_vertex_offset, 1, _vertex_offset);
    ++_vertex_offset;
  }

  set_vertex.set_name(name);
  _set_vertices.push_back(set_vertex);
  ++_node_id;
}

void GenerateSBGInput::addVariableNodes()
{
  IdentList variables = _mmo_class.variables();
  VarSymbolTable symbols = _mmo_class.syms();

  // Build unknown nodes.
  foreach_(Name var_name, variables)
  {
    VarInfo variable = symbols[var_name].get();
    if (isVariable(var_name, symbols)) {
      buildSet(variable, var_name);
    }
  }
}

// Add equations vertices ------------------------------------------------------

void GenerateSBGInput::buildEqualitySet(Equality eq
  , SetVertex set_vertex)
{
  set_vertex.offset(_vertex_offset);
  _vertex_offset += set_vertex.maxDimSize() + 1;

  for (auto k = set_vertex.arity(); k < _max_dim; ++k) {
    set_vertex.addDimension(_vertex_offset, 1, _vertex_offset);
  }
  ++_vertex_offset;

  set_vertex.set_node_id(_node_id);
  std::string name = "eq_" + std::to_string(_node_id);
  set_vertex.set_name(name);
  _set_vertices.push_back(set_vertex);
  ++_node_id;
}

SetVertex GenerateSBGInput::indicesDefinition(const IndexList& indices)
{
  SetVertex set_vertex{_node_id};

  for (const Index& index : indices) {
    OptExp expr = index.exp();
    if (!expr || !is<Range>(expr.get())) {
      ERROR("GenerateSBGInput::indicesDefinition: only Range expressions "
        , "supported\n");
    }
    Range expr_range = get<Range>(expr.get());
    Integer start = getValue(expr_range.start());
    Integer step = 1;
    Integer end = getValue(expr_range.end());
    if (expr_range.step()) {
      step = getValue(expr_range.step().get());
    }
    set_vertex.addDimension(start, step, end);
  }

  return set_vertex;
}

void GenerateSBGInput::buildForEqSet(ForEq eq, SetVertex set_vertex)
{
  ForEq for_eq = get<ForEq>(eq);
  SetVertex set_vertex_copy = set_vertex;
  IndexList indices = for_eq.range().indexes();
  set_vertex_copy.concat(indicesDefinition(indices));
  for (const Equation& jth_eq : for_eq.elements()) {
    if (is<Equality>(jth_eq)) {
      Equality equality = get<Equality>(jth_eq);
      buildEqualitySet(equality, set_vertex_copy);

      _equations_info[set_vertex_copy.node_id()]
        = EquationInfo{indices, equality};
    } else if (is<ForEq>(jth_eq)) {
      buildForEqSet(get<ForEq>(jth_eq), set_vertex_copy);
    } else {
      ERROR("GenerateSBGInput::buildForEqSet: only equalities and for loops "
        , "supported\n");
    }
  }
}

void GenerateSBGInput::addEquationNodes()
{
  // Build equation nodes.
  EquationList eqs = _mmo_class.equations().equations();
  foreach_(Equation eq, eqs)
  {
    SetVertex set_vertex{_node_id};
    if (is<ForEq>(eq)) {
      ForEq for_eq = get<ForEq>(eq);
      buildForEqSet(for_eq, set_vertex);
    } else if (is<Equality>(eq)) {
      Equality equality = get<Equality>(eq);
      buildEqualitySet(equality, set_vertex);
    } else {
      ERROR("GenerateSBGInput::addEquationNodes: only equalities and for loops "
        , "supported\n");
    }
  }
}

// Add edges -------------------------------------------------------------------

//Integer GenerateSBGInput::getSize(const Index& idx) const
//{
//  OptExp exp = idx.exp();
//  if (!exp || !is<Range>(exp.get())) {
//    ERROR("Only Range expressions supported");
//  }
//  Range range = get<Range>(exp.get());
//  /// @todo Handle step size.
//  return getValue(range.end()) - getValue(range.start()) + 1;
//}
//
//Integer GenerateSBGInput::getSize(const IndexList& dom) const
//{
//  Integer ret = 1;
//  for (const auto& idx : dom) {
//    ret *= getSize(idx);
//  }
//  return ret;
//}
//
//Integer GenerateSBGInput::getMin(const Index& idx) const
//{
//  OptExp exp = idx.exp();
//  if (!exp || !is<Range>(exp.get())) {
//    ERROR("Only Range expressions supported");
//  }
//  Range range = get<Range>(exp.get());
//  return getValue(range.start());
//}
//
//void GenerateSBGInput::addOffset(int edge_id, const std::string& map, int offset, int v_id, int dim)
//{
//  std::ostringstream def;
//  std::ostringstream dim_name;
//  if (dim >= 0) {
//    dim_name << "D" << dim;
//  }
//  std::ostringstream off_def;
//  if (offset > 0) {
//    off_def << " + " << offset;
//  } else if (offset < 0) {
//    off_def << " - " << offset;
//  }
//  def << "off" << map << edge_id << dim_name.str() << " = "
//      << "V" << v_id - 1 << " - E" << edge_id - 1 << off_def.str();
//  _offsets.push_back(def.str());
//}
//
//void GenerateSBGInput::addEdgeDef(int edge_id, int end, int dim)
//{
//  std::ostringstream def;
//  std::ostringstream dim_name;
//  if (dim >= 0) {
//    dim_name << "D" << dim;
//  }
//  def << "E" << edge_id << dim_name.str() << " = "
//      << "E" << edge_id - 1 << dim_name.str() << "+" << end;
//  _offsets.push_back(def.str());
//  def.str("");
//  def << "[E" << edge_id - 1 << dim_name.str() << "+1:1:E" << edge_id << dim_name.str() << "]";
//  _E.push_back(def.str());
//}
//
//void GenerateSBGInput::generatePWLMaps(Expression exp, const std::string& eq_id, int edge_id)
//{
//  assert(is<Reference>(exp));
//  const IndexList& dom = _eq_range[eq_id];
//  Integer dom_size = getSize(dom);
//  VarSymbolTable symbols = _mmo_class.syms();
//
//  Reference occur = get<Reference>(exp);
//  Ref names = occur.ref();
//  assert(names.size() > 0);
//  std::string node_name = get<0>(names[0]);
//  int node_id = _var_nodes[node_name];
//  int eq_node_id = _eq_nodes[eq_id];
//  ExpList indexes = get<1>(occur.ref().front());
//  addEdgeDef(edge_id, dom_size);
//  for (Expression idx : indexes) {
//    ConstantExpression constant_index(symbols);
//    if (Apply(constant_index, idx)) {
//      addOffset(edge_id, "M1", 0, node_id);
//      _m1_slopes.push_back(1);
//    } else {
//      PWLMapValues pwl_map_values(symbols);
//      Apply(pwl_map_values, idx);
//      assert(pwl_map_values.slope() != 0);
//      Usage usage = _eq_usage[eq_id];
//      int range_init_value = usage[pwl_map_values.variable()];
//      int map_first_value = 0;
//      map_first_value = pwl_map_values.constant() + pwl_map_values.slope() * range_init_value;
//      addOffset(edge_id, "M1", map_first_value, node_id);
//      _m1_slopes.push_back(pwl_map_values.slope());
//    }
//  }
//  if (indexes.empty()) {  // Scalar variable.
//    addOffset(edge_id, "M1", 0, node_id);
//    _m1_slopes.push_back(1);
//  }
//  std::for_each(dom.begin(), dom.end(), [this, edge_id, eq_node_id](auto) { addOffset(edge_id, "M2", 0, eq_node_id); });
//  if (dom.empty()) {  // Scalar variable.
//    addOffset(edge_id, "M2", 0, eq_node_id);
//  }
//}
//
//void GenerateSBGInput::generateEdgeMap(const std::string& map_name, const std::string& map_idx, bool fixed_slopes)
//{
//  _sbg_input << map_name << ": <<";
//  unsigned long size = 1;
//  for (std::string def : _E) {
//    int slope = fixed_slopes ? 1 : _m1_slopes[size - 1];
//    _sbg_input << "{" << def << "} -> |" << slope << "*x+off" << map_idx << size << ((size < _E.size()) ? "|, " : "|");
//    size++;
//  }
//  _sbg_input << ">>" << std::endl;
//}
//
//void GenerateSBGInput::addEdges()
//{
//  VarSymbolTable symbols = _mmo_class.syms();
//
//  foreach_(auto eq_desc, _eqs)
//  {
//    Equality eq = eq_desc.second;
//    Expression left = eq.left();
//    Expression right = eq.right();
//    foreach_(const auto& node, _var_nodes)
//    {
//      Name var_name = node.first;
//      MatchingExps matching_exps(var_name, isState(var_name, symbols));
//      Apply(matching_exps, left);
//      Apply(matching_exps, right);
//      std::set<Expression> matched_exps = matching_exps.matchedExps();
//      LOG << "Matched exps for: " << var_name << " in " << eq << std::endl;
//      foreach_(Expression exp, matched_exps)
//      {
//        LOG << "Expression: " << exp << std::endl;
//        generatePWLMaps(exp, eq_desc.first, _edge_id);
//        _edge_id++;
//      }
//    }
//  }
//}

// Build SBG -------------------------------------------------------------------

void GenerateSBGInput::setup()
{
  IdentList variables = _mmo_class.variables();
  VarSymbolTable symbols = _mmo_class.syms();

  _sbg_input.open(fileName());
  _max_dim = 1;
  _node_id = 1;
  _edge_id = 1;

  // Get max dim defined in the model.
  for (Name var_name : variables) {
    VarInfo variable = symbols[var_name].get();
    if (isVariable(var_name, symbols)) {
      Option<ExpList> indexes = variable.indices();
      if (indexes && (indexes->size() > _max_dim)) {
        _max_dim = indexes->size();
      }
    }
  }
}

void GenerateSBGInput::buildFromModel()
{
  setup();
  addVariableNodes();
  addEquationNodes();
  for (auto sv : _set_vertices)
    std::cout << sv << "\n";
  //addEdges();
  generateSBGInput();
}

void GenerateSBGInput::generateSBGInput()
{
  _sbg_input << "dims = " << _max_dim << ";\n";

  _sbg_input << "causalize(" << std::endl;

  _sbg_input << "V: {";
  unsigned long size = 1;
  for (const SetVertex& sv : _set_vertices) {
    _sbg_input << sv.printSet().str()
      << ((size < _set_vertices.size()) ? ", " : "");
    ++size;
  }
  _sbg_input << "}" << std::endl;

  _sbg_input << "Vmap: <<";
  size = 1;
  for (const SetVertex& sv : _set_vertices) {
    _sbg_input << "{" << sv.printSet().str() << "} -> ";
    for (std::size_t k = 0; k + 1 < _max_dim; ++k) {
      _sbg_input << "|0*x+" << size;
    }
    _sbg_input << "|0*x+" << size;
    _sbg_input << ((size < _set_vertices.size()) ? "|, " : "|");
    size++;
  }
  _sbg_input << ">>" << std::endl;

  //generateEdgeMap("map1", "M1");
  //const bool FIXED_SLOPES = true;
  //generateEdgeMap("map2", "M2", FIXED_SLOPES);
  //_sbg_input << "Emap: <<";
  //size = 1;
  //for (std::string def : _E) {
  //  _sbg_input << "{" << def << "} -> |0*x+" << size << ((size < _E.size()) ? "|, " : "|");
  //  size++;
  //}
  //_sbg_input << ">>" << std::endl;

  _sbg_input << ", 1);" << std::endl;

  _sbg_input.close();
}

} // namespace Causalize

} // namespace Modelica
