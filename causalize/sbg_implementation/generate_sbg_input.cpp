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
#include "util/logger.hpp"
#include "util/ast_visitors/equation_sbg_set.hpp"
#include "util/ast_visitors/eval_expression.hpp"
#include "util/ast_visitors/eval_integer.hpp"
#include "util/ast_visitors/matching_exps.hpp"
#include "util/ast_visitors/sbg_expr_visitor.hpp"
#include "util/ast_visitors/sbg_set_visitor.hpp"

#include <eval/file_evaluator.hpp>
#include <eval/pretty_print.hpp>
#include <sbg/set.hpp>
#include <util/time_profiler.hpp>

#include <algorithm>
#include <cmath>
#include <sstream>
#include <tuple>
#include <variant>

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// SBG generation return structure ---------------------------------------------
////////////////////////////////////////////////////////////////////////////////

SBGGenerationResult::SBGGenerationResult(
  MMO_Class& mmo_class
  , ModelicaSBG modelica_bsbg
  , SBG::LIB::BipartiteSBG bipartite_sbg
) : _mmo_class(mmo_class), _modelica_bsbg(modelica_bsbg)
    , _bipartite_sbg(bipartite_sbg) {}

const MMO_Class& SBGGenerationResult::mmo_class() const
{
  return _mmo_class;
}

const ModelicaSBG& SBGGenerationResult::modelica_bsbg() const
{
  return _modelica_bsbg;
}

const SBG::LIB::BipartiteSBG& SBGGenerationResult::bipartite_sbg() const
{
  return _bipartite_sbg;
}

////////////////////////////////////////////////////////////////////////////////
// Generate SBG Input ----------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

// Constructors/Destructors ----------------------------------------------------

GenerateSBGInput::GenerateSBGInput(MMO_Class& mmo_class)
    : _mmo_class(mmo_class), _max_dim(0) {}

// Getters ---------------------------------------------------------------------

std::string GenerateSBGInput::fileName() { return _mmo_class.name() + "_sbg_input.sbg"; }

// Add variable vertices -------------------------------------------------------

void GenerateSBGInput::addVariableSet(const VarInfo& variable, const Name& name)
{
  SBG::LIB::Set var_set;
  std::size_t var_dimensions = 0;
  Option<ExpList> dimensions = variable.indices();
  VarSymbolTable symbols = _mmo_class.syms();
  EvalInteger eval_int{symbols};
  if (dimensions) {
    std::size_t k = 0;
    for (const Expression& dimension : dimensions.value()) {
      Integer value = Apply(eval_int, dimension);
      if (k == 0) {
        var_set = SBG::LIB::Set{1, 1, value};
      } else {
        var_set = var_set.cartesianProduct(SBG::LIB::Set{1, 1, value});
      }
      ++k;
    }
    var_dimensions = dimensions.value().size();
  } else {
    var_set = SBG::LIB::Set{1, 1, 1};
    var_dimensions = 1;
  }

  // Fill remaining dimensions
  for (std::size_t k = var_dimensions; k < _max_dim; ++k) {
    var_set = var_set.cartesianProduct(SBG::LIB::Set{1, 1, 1});
  }

  // Save variable set-vertex
  _modelica_bsbg.addSetVertex(var_set, name, variable);
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

namespace {

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
    result.insert(result.end(), flattened_for_eq.begin(), flattened_for_eq.end());
  } else {
    result.push_back(eq);
  }

  return result;
}

}  // namespace

EquationList GenerateSBGInput::flatterForEqs() const
{
  EquationList result;

  EquationList eqs = _mmo_class.equations().equations();
  for (const Equation& eq : eqs) {
    EquationList flattened_eq = flatterEq(eq);
    result.insert(result.end(), flattened_eq.begin(), flattened_eq.end());
  }

  return result;
}

namespace {

Indexes getIndices(Equation eq, unsigned int max_dim)
{
  if (is<ForEq>(eq)) {
    return get<ForEq>(eq).range();
  } else {
    IndexList scalar_indices;
    for (std::size_t k = 0; k < max_dim; ++k) {
      scalar_indices.emplace_back("*dummy_" + std::to_string(k), Expression{0});
    }
    return Indexes{scalar_indices};
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

}  // namespace

void GenerateSBGInput::addEquationNodes()
{
  EquationList eqs = flatterForEqs();
  for (const Equation& eq : eqs) {
    EquationToSBGSet eq_to_sbg_set{_mmo_class.syms(), _max_dim};
    SBG::LIB::Set eq_vertices = Apply(eq_to_sbg_set, eq);
    std::string name = "eq_"
      + std::to_string(_modelica_bsbg.set_vertices().size() + 1);
    EquationInfo eq_info{
      getIndices(eq, _max_dim), getEquality(eq), !is<ForEq>(eq)
     };
    _modelica_bsbg.addSetVertex(eq_vertices, name, eq_info);
  }
}

// Add edges -------------------------------------------------------------------

SBG::LIB::Expression GenerateSBGInput::createMap1(
  const SetEdge& eq_se, const SetVertex& eq_sv
) const
{
  SBG::LIB::IntTuple domain_trans = eq_se.translation();
  SBG::LIB::IntTuple eq_nodes_trans = eq_sv.translation();
  SBG::LIB::Expression map1;
  for (std::size_t k = 0; k < _max_dim; ++k) {
    SBG::LIB::Int h = eq_nodes_trans[k] - domain_trans[k];
    map1 = map1.cartesianProduct(SBG::LIB::Expression{
      SBG::LIB::Rational{1}, SBG::LIB::Rational{h}
    });
  }
  return map1;
}

namespace {

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

}  // namespace

SBG::LIB::Expression GenerateSBGInput::createMap2(
  const SetEdge& eq_se, const SetVertex& var_sv
) const
{
  const SBG::LIB::IntTuple& var_trans = var_sv.translation();

  // Get expression of subscripts.
  Ref ref = getReference(eq_se.access()).ref();
  assert(ref.size() > 0);
  ERROR_UNLESS(ref.size() == 1, "GenerateSBGInput::createMap2: conversion of "
    , "dotted references not implemented");
  ExpList indexes = get<1>(ref.front());

  // Get order of counters.
  EquationInfo eq_info = std::get<EquationInfo>(
    _modelica_bsbg.setVertex(eq_se.eq_id()).info()
  );
  AST::Indexes counters = eq_info.indices();
  std::vector<std::string> order;
  for (const Index& counter : counters.indexes()) {
    order.push_back(counter.name());
  }

  SBG::LIB::Rational zero{0};
  SBG::LIB::Expression t;
  if (indexes.empty()) {  // Access to scalar variable.
    for (std::size_t k = 0; k < _max_dim; ++k) {
      SBG::LIB::Expression var_expr{zero, SBG::LIB::Rational{var_trans[k] + 1}};
      t = t.cartesianProduct(var_expr);
    }
  } else {  // Access to array variable.
    std::size_t k = 0;
    SBG::LIB::IntTuple domain_trans = eq_se.translation();
    ExprVisitor affine_expr_visitor(_mmo_class.syms(), order);
    for (Expression index : indexes) {
      SBG::LIB::Expression kth_expr = Apply(affine_expr_visitor, index);
      if (kth_expr.isConstant()) {
        SBG::LIB::Expression var_expr{zero, SBG::LIB::Rational{var_trans[k]}};
        t = t.cartesianProduct(kth_expr + var_expr);
      } else {
        SBG::LIB::Expression var_expr{
          zero, SBG::LIB::Rational{var_trans[k] - domain_trans[k]}
        };
        t = t.cartesianProduct(kth_expr + var_expr);
      }
      ++k;
    }
  }

  return t;
}

void GenerateSBGInput::addEdge(
  const SetVertex& eq_sv, const SetVertex& sv, const EquationInfo& eq_info
)
{
  if (sv.isVariable()) {
    Name var_name = sv.name();
    const Equality eq = get<Equality>(eq_info.equation());
    VarSymbolTable symbols = _mmo_class.syms();
    MatchingExps matching_exprs(var_name, isState(var_name, symbols));
    Apply(matching_exprs, eq.left());
    Apply(matching_exprs, eq.right());
    std::set<Expression> matched_exprs = matching_exprs.matchedExps();
    LOG << "Matched exprs for: " << var_name << " in " << eq << std::endl;

    SBG::LIB::Set eq_nodes = eq_sv.set();
    for (const Expression& expr : matched_exprs) {
      LOG << "Expression: " << expr << std::endl;
      std::stringstream ss;
      ss << expr;
      std::string name = eq_sv.name() + "-" + ss.str();

      int se_id = _modelica_bsbg.addSetEdge(
        sv.node_id(), eq_sv.node_id(), eq_nodes, expr, name
      );
      SetEdge& se = _modelica_bsbg.setEdge(se_id);
      se.set_map1(createMap1(se, eq_sv));
      se.set_map2(createMap2(se, sv));
    }
  }
}

void GenerateSBGInput::addEdges()
{
  SetVertices svs = _modelica_bsbg.set_vertices();
  for (const SetVertex& eq_sv : svs) {
    if (eq_sv.isEquation()) {
      EquationInfo eq_info = std::get<EquationInfo>(eq_sv.info());
      ERROR_UNLESS(is<Equality>(eq_info.equation())
        , "GenerateSBGInput::addEdges: only equality equations supported");

      // Get vertices of variables that appear in this array of equations.
      for (const SetVertex& sv : svs) {
        addEdge(eq_sv, sv, eq_info);
      }
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

  _modelica_bsbg.set_arity(_max_dim);
}

SBGGenerationResult GenerateSBGInput::buildFromModel()
{
  SBG::Util::Internal::TimeProfiler profiler{"Horizontal sorting SBG builder"};

  // Write SBG program to _sbg_input
  setup();
  addVariableNodes();
  addEquationNodes();
  addEdges();
  generateSBGInput();

  // Evaluate SBG program to obtain bipartite SBG
  SBG::LIB::BipartiteSBG g;
  SBG::Eval::ProgramIO eval_result = SBG::Eval::parseEvalFile(fileName());
  for (SBG::Eval::ExprResult ev : eval_result.exprs()) {
    SBG::Eval::ExprBaseType e = std::get<1>(ev);
    if (std::holds_alternative<SBG::LIB::BipartiteSBG>(e)) {
      g = std::get<SBG::LIB::BipartiteSBG>(e);
    }
  }

  return SBGGenerationResult{_mmo_class, _modelica_bsbg, g};
}

void GenerateSBGInput::generateVSet()
{
  _sbg_input << "V: ";
  SBG::LIB::Set V;
  for (const SetVertex& sv : _modelica_bsbg.set_vertices()) {
    SBG::LIB::Set jth_set = sv.set();
    jth_set = jth_set.translate(sv.translation());
    V = V.disjointCup(jth_set);
  }
  _sbg_input << V << std::endl;
}

void GenerateSBGInput::generateVMap()
{
  _sbg_input << "Vmap: <<";
  SetVertices svs = _modelica_bsbg.set_vertices();
  std::size_t size = svs.size();
  std::size_t j = 1;
  for (const SetVertex& sv : svs) {
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
  SetEdges ses = _modelica_bsbg.set_edges();
  std::size_t size = ses.size();
  std::size_t j = 1;
  for (const SetEdge& se : ses) {
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
  SetEdges ses = _modelica_bsbg.set_edges();
  std::size_t size = ses.size();
  std::size_t j = 1;
  for (const SetEdge& se : ses) {
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
  SetEdges ses = _modelica_bsbg.set_edges();
  std::size_t size = ses.size();
  std::size_t j = 1;
  for (const SetEdge& se : ses) {
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
  SetVertices svs = _modelica_bsbg.set_vertices();
  SBG::LIB::Set X;
  for (const SetVertex& sv : svs) {
    if (sv.isEquation()) {
      SBG::LIB::Set jth_set = sv.set();
      jth_set = jth_set.translate(sv.translation());
      X = X.disjointCup(jth_set);
    }
  }
  _sbg_input << X << std::endl;

  _sbg_input << "Y: ";
  SBG::LIB::Set Y;
  for (const SetVertex& sv : svs) {
    if (sv.isVariable()) {
      SBG::LIB::Set jth_set = sv.set();
      jth_set = jth_set.translate(sv.translation());
      Y = Y.disjointCup(jth_set);
    }
  }
  _sbg_input << Y;
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

}  // namespace Causalize

}  // namespace Modelica
