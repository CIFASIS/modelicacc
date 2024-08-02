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

#include <cmath>
#include <sstream>

#include <ast/queries.h>
#include <causalize/sbg_implementation/generate_sbg_input.h>
#include <util/ast_visitors/constant_expression.h>
#include <util/ast_visitors/eval_expression.h>
#include <util/ast_visitors/matching_exps.h>
#include <util/ast_visitors/pwl_map_values.h>
#include <util/logger.h>

using namespace Modelica;

namespace Causalize {
GenerateSBGInput::GenerateSBGInput(MMO_Class& mmo_class) : _mmo_class(mmo_class) {}

std::string GenerateSBGInput::fileName() { return _mmo_class.name() + "_sbg_input.sbg"; }

Integer GenerateSBGInput::getValue(Expression exp) const
{
  VarSymbolTable symbols = _mmo_class.syms();
  EvalExpression eval_exp(symbols);
  return Integer(Apply(eval_exp, exp));
}

void GenerateSBGInput::addDef(int node_id, int end, int dim, int step)
{
  std::ostringstream def;
  std::ostringstream dim_name;
  if (dim >= 0) {
    dim_name << "D" << dim;
  }
  def << "V" << node_id << dim_name.str() << " = "
      << "V" << node_id - 1 << dim_name.str() << "+" << end;
  _offsets.push_back(def.str());
  def.str("");
  def << "[V" << node_id - 1 << dim_name.str() << "+1:" << step << ":V" << node_id << dim_name.str() << "]";
  _V.push_back(def.str());
}

Integer GenerateSBGInput::getSize(const Index& idx) const
{
  OptExp exp = idx.exp();
  if (!exp || !is<Range>(exp.get())) {
    ERROR("Only Range expressions supported");
  }
  Range range = get<Range>(exp.get());
  /// @todo Handle step size.
  return getValue(range.end()) - getValue(range.start()) + 1;
}

Integer GenerateSBGInput::getSize(const IndexList& dom) const
{
  Integer ret = 1;
  for (const auto& idx : dom) {
    ret *= getSize(idx);
  }
  return ret;
}

Integer GenerateSBGInput::getMin(const Index& idx) const
{
  OptExp exp = idx.exp();
  if (!exp || !is<Range>(exp.get())) {
    ERROR("Only Range expressions supported");
  }
  Range range = get<Range>(exp.get());
  return getValue(range.start());
}

void GenerateSBGInput::addIndexRange(IndexList range, const std::string& eq_id, int node_id)
{
  Usage usage;
  foreach_(Index idx, range)
  {
    OptExp exp = idx.exp();
    if (!exp || !is<Range>(exp.get())) {
      ERROR("Only Range expressions supported");
    }
    Range idx_range = get<Range>(exp.get());
    Integer lower = getValue(idx_range.start());
    Integer step = 1;
    Integer upper = getValue(idx_range.end());
    if (idx_range.step()) {
      step = getValue(idx_range.step().get());
    }
    addDef(node_id, upper, -1, step);
    usage[idx.name()] = lower;
  }
  _eq_usage[eq_id] = usage;
}

/// @todo Handle for-for equations in the same way we do in QSS Solver.
void GenerateSBGInput::buildSet(Equation eq, const std::string& eq_id, int node_id, Indexes range)
{
  Usage usage;
  if (!range.indexes().empty()) {
    if (is<ForEq>(eq)) {
      ERROR("Nested for equations not implemented.");
      return;
    }
    addIndexRange(range.indexes(), eq_id, node_id);
  } else if (is<ForEq>(eq)) {
    ForEq for_eq = get<ForEq>(eq);
    addIndexRange(for_eq.range().indexes(), eq_id, node_id);
  } else {
    addDef(node_id, 1);
  }
}

/// @todo Add dimension padding and multiple dim support.
void GenerateSBGInput::buildSet(const VarInfo& variable, int node_id)
{
  Option<ExpList> dims = variable.indices();
  if (dims) {
    foreach_(Expression d, dims.get()) { addDef(node_id, getValue(d)); }
  } else {
    addDef(node_id, 1);
  }
}

void GenerateSBGInput::addOffset(int edge_id, const std::string& map, int offset, int v_id, int dim)
{
  std::ostringstream def;
  std::ostringstream dim_name;
  if (dim >= 0) {
    dim_name << "D" << dim;
  }
  std::ostringstream off_def;
  if (offset > 0) {
    off_def << " + " << offset;
  } else if (offset < 0) {
    off_def << " - " << offset;
  }
  def << "off" << map << edge_id << dim_name.str() << " = "
      << "r(V" << v_id - 1 << ",1) - r(E" << edge_id - 1 << ",1)" << off_def.str();
  _offsets.push_back(def.str());
}

void GenerateSBGInput::addEdgeDef(int edge_id, int end, int dim)
{
  std::ostringstream def;
  std::ostringstream dim_name;
  if (dim >= 0) {
    dim_name << "D" << dim;
  }
  def << "E" << edge_id << dim_name.str() << " = "
      << "E" << edge_id - 1 << dim_name.str() << "+" << end;
  _offsets.push_back(def.str());
  def.str("");
  def << "[E" << edge_id - 1 << dim_name.str() << "+1:1:E" << edge_id << dim_name.str() << "]";
  _E.push_back(def.str());
}

void GenerateSBGInput::generatePWLMaps(Expression exp, const std::string& eq_id, int edge_id)
{
  assert(is<Reference>(exp));
  const IndexList& dom = _eq_range[eq_id];
  Integer dom_size = getSize(dom);
  VarSymbolTable symbols = _mmo_class.syms();

  Reference occur = get<Reference>(exp);
  Ref names = occur.ref();
  assert(names.size() > 0);
  std::string node_name = get<0>(names[0]);
  int node_id = _var_nodes[node_name];
  int eq_node_id = _eq_nodes[eq_id];
  ExpList indexes = get<1>(occur.ref().front());
  addEdgeDef(edge_id, dom_size);
  for (Expression idx : indexes) {
    ConstantExpression constant_index(symbols);
    if (Apply(constant_index, idx)) {
      addOffset(edge_id, "M1", 0, node_id);
      _m1_slopes.push_back(1);
    } else {
      PWLMapValues pwl_map_values(symbols);
      Apply(pwl_map_values, idx);
      assert(pwl_map_values.slope() != 0);
      Usage usage = _eq_usage[eq_id];
      int range_init_value = usage[pwl_map_values.variable()];
      int map_first_value = 0;
      map_first_value = pwl_map_values.constant() + pwl_map_values.slope() * range_init_value;
      addOffset(edge_id, "M1", map_first_value, node_id);
      _m1_slopes.push_back(pwl_map_values.slope());
    }
  }
  if (indexes.empty()) {  // Scalar variable.
    addOffset(edge_id, "M1", 0, node_id);
    _m1_slopes.push_back(1);
  }
  std::for_each(dom.begin(), dom.end(), [this, edge_id, eq_node_id](auto) { addOffset(edge_id, "M2", 0, eq_node_id); });
  if (dom.empty()) {  // Scalar variable.
    addOffset(edge_id, "M2", 0, eq_node_id);
  }
}

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

void GenerateSBGInput::addVariableNodes()
{
  IdentList variables = _mmo_class.variables();
  VarSymbolTable symbols = _mmo_class.syms();

  _offsets.emplace_back("V0 = 0");
  _offsets.emplace_back("E0 = 0");
  // Build unknown nodes.
  foreach_(Name var_name, variables)
  {
    VarInfo variable = symbols[var_name].get();
    if (!isVariable(var_name, symbols)) {
      buildSet(variable, _node_id);
      _var_nodes.insert(std::make_pair(var_name, _node_id));
      _node_id++;
    }
  }
}

void GenerateSBGInput::addEquationInfo(const std::string& eq_name, Equality eq, IndexList indexes, int node_id)
{
  _eqs.insert(std::make_pair(eq_name, eq));
  _eq_range.insert(std::make_pair(eq_name, indexes));
  _eq_nodes.insert(std::make_pair(eq_name, node_id));
}

void GenerateSBGInput::addEquationNodes()
{
  // Build equation nodes.
  EquationList eqs = _mmo_class.equations().equations();
  foreach_(Equation eq, eqs)
  {
    if (is<ForEq>(eq)) {
      ForEq for_eq = get<ForEq>(eq);
      std::vector<Equation> for_eqs = for_eq.elements();
      Indexes range = for_eq.range();
      foreach_(Equation for_el, for_eqs)
      {
        ERROR_UNLESS(is<Equality>(for_el), "Only causalization of for and equality equations");
        std::string eq_name = "eq_" + std::to_string(_node_id);
        buildSet(for_el, eq_name, _node_id, range);
        addEquationInfo(eq_name, get<Equality>(for_el), range.indexes(), _node_id);
        _node_id++;
      }
    } else if (is<Equality>(eq)) {
      std::string eq_name = "eq_" + std::to_string(_node_id);
      buildSet(eq, eq_name, _node_id);
      addEquationInfo(eq_name, get<Equality>(eq), IndexList(), _node_id);
      _node_id++;
    } else {
      ERROR("Only causalization of for and equality equations");
    }
  }
}

void GenerateSBGInput::addEdges()
{
  VarSymbolTable symbols = _mmo_class.syms();

  foreach_(auto eq_desc, _eqs)
  {
    Equality eq = eq_desc.second;
    Expression left = eq.left();
    Expression right = eq.right();
    foreach_(const auto& node, _var_nodes)
    {
      Name var_name = node.first;
      MatchingExps matching_exps(var_name, isState(var_name, symbols));
      Apply(matching_exps, left);
      Apply(matching_exps, right);
      std::set<Expression> matched_exps = matching_exps.matchedExps();
      LOG << "Matched exps for: " << var_name << " in " << eq << std::endl;
      foreach_(Expression exp, matched_exps)
      {
        LOG << "Expression: " << exp << std::endl;
        generatePWLMaps(exp, eq_desc.first, _edge_id);
        _edge_id++;
      }
    }
  }
}

void GenerateSBGInput::buildFromModel()
{
  setup();
  addVariableNodes();
  addEquationNodes();
  addEdges();
  generateSBGInput();
}

void GenerateSBGInput::generateEdgeMap(const std::string& map_name, const std::string& map_idx, bool fixed_slopes)
{
  _sbg_input << map_name << " %= <<";
  unsigned long size = 1;
  for (std::string def : _E) {
    int slope = fixed_slopes ? 1 : _m1_slopes[size - 1];
    _sbg_input << "{" << def << "} -> " << slope << "*x+off" << map_idx << size << ((size < _E.size()) ? " , " : "");
    size++;
  }
  _sbg_input << ">>;" << std::endl;
}

void GenerateSBGInput::generateSBGInput()
{
  for (std::string def : _offsets) {
    _sbg_input << def << std::endl;
  }
  _sbg_input << std::endl;
  _sbg_input << "matchSCCTS(" << std::endl;
  _sbg_input << "V %= {";
  unsigned long size = 1;
  for (std::string def : _V) {
    _sbg_input << def << ((size < _V.size()) ? " , " : "");
    size++;
  }
  _sbg_input << "};" << std::endl;

  _sbg_input << "Vmap %= <<";
  size = 1;
  for (std::string def : _V) {
    _sbg_input << "{" << def << "} -> 0*x+" << size << ((size < _V.size()) ? " , " : "");
    size++;
  }
  _sbg_input << ">>;" << std::endl;
  generateEdgeMap("map1", "M1");
  const bool FIXED_SLOPES = true;
  generateEdgeMap("map2", "M2", FIXED_SLOPES);
  _sbg_input << "Emap %= <<";
  size = 1;
  for (std::string def : _E) {
    _sbg_input << "{" << def << "} -> 0*x+" << size << ((size < _E.size()) ? " , " : "");
    size++;
  }
  _sbg_input << ">>;" << std::endl;
  _sbg_input << ", 1)" << std::endl;
  _sbg_input.close();
}

}  // namespace Causalize
