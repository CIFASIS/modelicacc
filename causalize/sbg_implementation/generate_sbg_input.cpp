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

#include "causalize/sbg_implementation/generate_sbg_input.hpp"
#include "ast/queries.hpp"
#include "util/affine_transformation.hpp"
#include "util/affine_expr.hpp"
#include "util/logger.hpp"
#include "util/ast_visitors/affine_expr_visitor.hpp"
#include "util/ast_visitors/compact_set_visitor.hpp"
#include "util/ast_visitors/equation_compact_set.hpp"
#include "util/ast_visitors/eval_expression.hpp"
#include "util/ast_visitors/eval_integer.hpp"
#include "util/ast_visitors/matching_exps.hpp"

#include "eval/file_evaluator.hpp"
#include "eval/pretty_print.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <tuple>
#include <variant>

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// Auxiliary definitions -------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

SBGGenerationInfo::SBGGenerationInfo(MMO_Class& mmo_class, unsigned int max_dim
  , std::vector<SetVertex>& set_vertices, std::vector<SetEdge>& set_edges
  , std::map<int, EquationInfo>& equations_info
  , SBG::LIB::BipartiteSBG bipartite_sbg)
  : _mmo_class(mmo_class), _max_dim(max_dim), _set_vertices(set_vertices)
    , _set_edges(set_edges), _equations_info(equations_info)
    , _bipartite_sbg(bipartite_sbg) {}

const MMO_Class& SBGGenerationInfo::mmo_class() const { return _mmo_class; }

const unsigned int& SBGGenerationInfo::max_dim() const { return _max_dim; }

const std::vector<SetVertex>& SBGGenerationInfo::set_vertices() const
{
  return _set_vertices;
}

const std::vector<SetEdge>& SBGGenerationInfo::set_edges() const
{
  return _set_edges;
}

const std::map<int, EquationInfo>& SBGGenerationInfo::equations_info() const
{
  return _equations_info;
}

const SBG::LIB::BipartiteSBG& SBGGenerationInfo::bipartite_sbg() const
{
  return _bipartite_sbg;
}

////////////////////////////////////////////////////////////////////////////////
// Generate SBG Input ----------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

// Constructors/Destructors ----------------------------------------------------

GenerateSBGInput::GenerateSBGInput(MMO_Class& mmo_class)
  : _mmo_class(mmo_class), _vertex_offset(0), _edge_offset(0) {}

// Getters ---------------------------------------------------------------------

std::string GenerateSBGInput::fileName() { return _mmo_class.name()
  + "_sbg_input.sbg"; }

const Modelica::MMO_Class& GenerateSBGInput::mmo_class() const
{
  return _mmo_class;
}

const unsigned int& GenerateSBGInput::max_dim() const { return _max_dim; }

const std::vector<SetVertex>& GenerateSBGInput::set_vertices() const
{
  return _set_vertices;
}

const std::vector<SetEdge>& GenerateSBGInput::set_edges() const
{
  return _set_edges;
}

const std::map<int, EquationInfo>& GenerateSBGInput::equations_info() const
{
  return _equations_info;
}

// Add variable vertices -------------------------------------------------------

void GenerateSBGInput::addVariableSet(const VarInfo& variable
  , const Name& name)
{
  CompactSet var_set;
  std::size_t var_dimensions = 0;
  Option<ExpList> dimensions = variable.indices();
  EvalInteger eval_int{_mmo_class.syms()};
  if (dimensions) {
    std::size_t k = 0;
    for (const Expression& dimension : dimensions.value()) {
      Integer value = Apply(eval_int, dimension);
      if (k == 0) {
        var_set = CompactSet{1, 1, value};
      } else {
        var_set.cartesianProduct(CompactSet{1, 1, value});
      }
      ++k;
    }
    var_dimensions = dimensions.value().size();
  } else {
    var_set = CompactSet{1, 1, 1};
    var_dimensions = 1;
  }

  // Fill remaining dimensions
  SetVertex set_vertex{_node_id, var_set};
  for (std::size_t k = var_dimensions; k < _max_dim; ++k) {
    set_vertex.cartesianProduct(CompactSet{1, 1, 1});
  }

  // Translate to avoid repeating nodes values
  set_vertex.set_translation(Translation{_max_dim, _vertex_offset});
  _vertex_offset += set_vertex.maxDimPerimetral();

  // Save variable set-vertex
  set_vertex.set_name(name);
  _set_vertices.push_back(set_vertex);
  ++_node_id;
}

void GenerateSBGInput::addVariableNodes()
{
  IdentList variables = _mmo_class.variables();
  VarSymbolTable symbols = _mmo_class.syms();

  for (const Name& var_name : variables) {
    VarInfo variable = symbols[var_name].get();
    if (isVariable(var_name, symbols)) {
      addVariableSet(variable, var_name);
    }
  }
}

// Add equations vertices ------------------------------------------------------

namespace detail {

EquationList flatterEq(Equation eq);

EquationList flatterForEq(ForEq for_eq)
{
  EquationList result;

  for (const Equation& eq : for_eq.elements()) {
    EquationList flattened_eq = flatterEq(eq);
    for (const Equation& nested_eq : flattened_eq) {
      result.push_back(ForEq{for_eq.range(), EquationList{1, nested_eq}});
    }
  }

  return result;
}

EquationList flatterEq(Equation eq)
{
  EquationList result;

  if (is<ForEq>(eq)) {
    EquationList flattened_for_eq = flatterForEq(get<ForEq>(eq));
    result.insert(result.end(), flattened_for_eq.begin()
      , flattened_for_eq.end());
  } else {
    result.push_back(eq);
  }

  return result;
}

} // namespace detail

EquationList GenerateSBGInput::flatterForEqs() const
{
  EquationList result;

  EquationList eqs = _mmo_class.equations().equations();
  for (const Equation& eq : eqs) {
    EquationList flattened_eq = detail::flatterEq(eq);
    result.insert(result.end(), flattened_eq.begin(), flattened_eq.end());
  }

  return result;
}

IndexList getIndices(Equation eq, unsigned int max_dim)
{
  if (is<ForEq>(eq)) {
    return get<ForEq>(eq).range().indexes();
  } else {
    IndexList scalar_indices;
    for (std::size_t k = 0; k < max_dim; ++k) {
      scalar_indices.emplace_back("*dummy_" + std::to_string(k), Expression{0});
    }
    return scalar_indices;
  }
}

/**
 * @brief Currently we only handle equalities.
 */
Equality getEquality(Equation eq)
{
  if (is<Equality>(eq)) {
    return get<Equality>(eq);
  } else if (is<ForEq>(eq)) {
    return getEquality(get<ForEq>(eq).elements().front());
  }

  ERROR("GenerateSBGInput::getEquality: only for eqs and equalities supported");
  return Equality{};
}

void GenerateSBGInput::addEquationNodes()
{
  EquationList eqs = flatterForEqs();
  for (const Equation& eq : eqs) {
    EquationCompactSet eq_to_compact_set{_mmo_class.syms(), _max_dim};
    SetVertex set_vertex{_node_id, Apply(eq_to_compact_set, eq)};

    // Translate to avoid repeating nodes values
    set_vertex.set_translation(Translation{_max_dim, _vertex_offset});
    _vertex_offset += set_vertex.maxDimPerimetral();

    // Save equation set-vertex
    set_vertex.set_name("eq_" + std::to_string(_node_id));
    _equations_info[_node_id]
      = EquationInfo{getIndices(eq, _max_dim), getEquality(eq)};
    _set_vertices.push_back(set_vertex);
    ++_node_id;
  }
}

// Add edges -------------------------------------------------------------------

CompactTransformation GenerateSBGInput::createMap1(const CompactSet& eq_nodes
  , const Translation& eq_nodes_trans) const
{
  Translation domain_trans{_max_dim, _edge_offset};
  CompactTransformation map1{_max_dim};
  for (std::size_t k = 0; k < _max_dim; ++k) {
    map1.matrix(k, k) = 1;
    map1.translation(k) = eq_nodes_trans[k] - domain_trans[k];
  }
  return map1;
}

CompactTransformation GenerateSBGInput::createMap2(const Reference& reference
  , const IndexList& counters, const Translation& var_trans) const
{
  // Get expression of subscripts 
  Ref ref = reference.ref();
  assert(ref.size() > 0);
  ERROR_UNLESS(ref.size() == 1, "GenerateSBGInput::createMap2: conversion of "
    , "dotted references not implemented");
  ExpList indexes = get<1>(ref.front());

  // Get order of counters
  std::vector<std::string> order;
  for (const Index& counter : counters) {
    order.push_back(counter.name());
  }

  CompactTransformation t{_max_dim};
  if (indexes.empty()) { // Access to scalar variable
    for (std::size_t k = 0; k < _max_dim; ++k) {
      Util::AffineExpr kth_expr{order};
      t.setRow(k, kth_expr + (var_trans[k] + 1));
    }
  } else { // Access to array variable
    std::size_t k = 0;
    Translation domain_trans{_max_dim, _edge_offset};
    AffineExprVisitor affine_expr_visitor(_mmo_class.syms(), order);
    for (Expression index : indexes) {
      Util::AffineExpr kth_expr = Apply(affine_expr_visitor, index);
      if (kth_expr.isConstant()) {
        t.setRow(k, kth_expr + var_trans[k]);
      } else {
        t.setRow(k, kth_expr + (var_trans[k] - domain_trans[k]));
      }
      ++k;
    }
  }

  return t;
}

void GenerateSBGInput::addMaps(SetEdge se, const SetVertex& eq_sv
  , const SetVertex& var_sv, const Reference& reference)
{
  CompactSet eq_nodes = eq_sv.set();
  CompactSet domain = eq_nodes;
  domain.translate(se.translation());
  se.set_domain(domain);

  se.set_map1(createMap1(eq_nodes, eq_sv.translation()));
  se.set_map2(createMap2(reference, _equations_info[eq_sv.node_id()].indices()
    , var_sv.translation()));
  _set_edges.push_back(se);

  _edge_offset += eq_nodes.maxDimPerimetral();
  ++_edge_id;
}

/**
 * @brief Gets a reference to a variable or the derivative of a variable.
 */
Reference getReference(Expression expr)
{
  if (is<Reference>(expr)) {
    return get<Reference>(expr);
  } else if (is<Call>(expr)) {
    Call call = get<Call>(expr);
    ERROR_UNLESS(call.name() == "der", "getReference: expression ", expr
      , " is not a variable or a derivative");
    ExpList args = call.args();
    ERROR_UNLESS(args.size() == 1, "getReference: der applied to more than "
      , "argument in ", args);
    return getReference(args.front());
  }

  ERROR("getReference: expression ", expr, " is not a reference");
  return Reference{};
}

void GenerateSBGInput::addEdge(const SetVertex& eq_sv, const SetVertex& sv
  , const EquationInfo& eq_info)
{
  if (sv.isVariable()) {
    Name var_name = sv.name();
    const Equality eq = get<Equality>(eq_info.equation());
    MatchingExps matching_exprs(var_name, isState(var_name, _mmo_class.syms()));
    Apply(matching_exprs, eq.left());
    Apply(matching_exprs, eq.right());
    std::set<Expression> matched_exprs = matching_exprs.matchedExps();
    LOG << "Matched exprs for: " << var_name << " in " << eq << std::endl;

    CompactSet eq_nodes = eq_sv.set();
    for (const Expression& expr : matched_exprs) {
      LOG << "Expression: " << expr << std::endl;
      std::stringstream ss;
      ss << expr;
      std::string name = eq_sv.name() + "-" + ss.str();

      SetEdge se{_edge_id, _max_dim};
      se.set_name(name);
      se.set_var_id(sv.node_id());
      se.set_eq_id(eq_sv.node_id());
      se.set_translation(Translation{_max_dim, _edge_offset});
      se.set_access(expr);
      se.set_translation(Translation{_max_dim, _edge_offset});

      addMaps(se, eq_sv, sv, getReference(expr));
    }
  }
}

void GenerateSBGInput::addEdges()
{
  for (auto& [eq_id, eq_info] : _equations_info) {
    ERROR_UNLESS(is<Equality>(eq_info.equation())
      , "GenerateSBGInput::addEdges: only equality equations supported");

    // Get vertices that represent this array of equations
    SetVertex eq_sv{-1};
    for (const SetVertex& sv : _set_vertices) {
      if (sv.node_id() == eq_id) {
        eq_sv = sv;
        break;
      } 
    }

    // Get vertices of variables that appear in this array of equations
    for (const SetVertex& sv : _set_vertices) {
      addEdge(eq_sv, sv, eq_info);
    }
  }
}

// Generate SBG program --------------------------------------------------------

void GenerateSBGInput::setup()
{
  IdentList variables = _mmo_class.variables();
  VarSymbolTable symbols = _mmo_class.syms();

  _sbg_input.open(fileName());
  _max_dim = 1;
  _node_id = 1;
  _edge_id = 1;

  // Get maximum dimension  between the arrays of variables defined in the model
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

SBGGenerationInfo GenerateSBGInput::buildFromModel()
{
  // Write SBG program to _sbg_input
  setup();
  addVariableNodes();
  addEquationNodes();
  addEdges();
  generateSBGInput();

  // Evaluate SBG program to obtain bipartite SBG
  SBG::LIB::BipartiteSBG g;
  SBG::Eval::ProgramIO eval_result = SBG::Eval::parseEvalFile(fileName()); 
  for (const SBG::Eval::ExprResult& ev : eval_result.exprs()) {
    SBG::Eval::ExprBaseType e = std::get<1>(ev);
    if (std::holds_alternative<SBG::LIB::BipartiteSBG>(e)) {
      g = std::get<SBG::LIB::BipartiteSBG>(e);
    }
  }

  return SBGGenerationInfo{_mmo_class, _max_dim, _set_vertices, _set_edges
    , _equations_info, g};
}

void GenerateSBGInput::generateVSet()
{
  _sbg_input << "V: ";
  CompactSet V;
  for (const SetVertex& sv : _set_vertices) {
    CompactSet jth_set = sv.set();
    jth_set.translate(sv.translation());
    V.setUnion(jth_set);
  }
  _sbg_input << V.toSBGFormat() << std::endl;
}

void GenerateSBGInput::generateVMap()
{
  _sbg_input << "Vmap: <<";
  std::size_t size = _set_vertices.size();
  std::size_t j = 1;
  for (const SetVertex& sv : _set_vertices) {
    _sbg_input << sv.toSBGFormat() << " -> ";
    for (std::size_t k = 0; k + 1 < _max_dim; ++k) {
      _sbg_input << "|0*x+" << j;
    }
    _sbg_input << "|0*x+" << j;
    _sbg_input << ((j < size) ? "|, " : "|");
    ++j;
  }
  _sbg_input << ">>" << std::endl;
}

void GenerateSBGInput::generateMap1()
{
  _sbg_input << "map1: <<";
  std::size_t size = _set_edges.size();
  std::size_t j = 1;
  for (const SetEdge& se : _set_edges) {
    _sbg_input << se.domainToSBGFormat() << " -> ";
    _sbg_input << se.map1ToSBGFormat();
    _sbg_input << ((j < size) ? ", " : "");
    ++j;
  }
  _sbg_input << ">>" << std::endl;
}

void GenerateSBGInput::generateMap2()
{
  _sbg_input << "map2: <<";
  std::size_t size = _set_edges.size();
  std::size_t j = 1;
  for (const SetEdge& se : _set_edges) {
    _sbg_input << se.domainToSBGFormat() << " -> ";
    _sbg_input << se.map2ToSBGFormat();
    _sbg_input << ((j < size) ? ", " : "");
    ++j;
  }
  _sbg_input << ">>" << std::endl;
}

void GenerateSBGInput::generateEMap()
{
  _sbg_input << "Emap: <<";
  std::size_t size = _set_edges.size();
  std::size_t j = 1;
  for (const SetEdge& se : _set_edges) {
    _sbg_input << se.domainToSBGFormat() << " -> ";
    for (std::size_t k = 0; k + 1 < _max_dim; ++k) {
      _sbg_input << "|0*x+" << j;
    }
    _sbg_input << "|0*x+" << j;
    _sbg_input << ((j < size) ? "|, " : "|");
    ++j;
  }
  _sbg_input << ">>" << std::endl;
}

void GenerateSBGInput::generatePartition()
{
  _sbg_input << "X: ";
  CompactSet X;
  for (const SetVertex& sv : _set_vertices) {
    if (sv.isEquation()) {
      CompactSet jth_set = sv.set();
      jth_set.translate(sv.translation());
      X.setUnion(jth_set);
    }
  }
  _sbg_input << X.toSBGFormat() << std::endl;

  _sbg_input << "Y: ";
  CompactSet Y;
  for (const SetVertex& sv : _set_vertices) {
    if (sv.isVariable()) {
      CompactSet jth_set = sv.set();
      jth_set.translate(sv.translation());
      Y.setUnion(jth_set);
    }
  }
  _sbg_input << Y.toSBGFormat();
}

void GenerateSBGInput::generateSBGInput()
{
  _sbg_input << "dims = " << _max_dim << ";\n";

  generateVSet();
  generateVMap();
  generateMap1();
  generateMap2();
  generateEMap();
  generatePartition();
  _sbg_input << ";";

  _sbg_input.close();
}

} // namespace Causalize

} // namespace Modelica
