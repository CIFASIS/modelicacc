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
#include "util/ast_visitors/affine_expr_visitor.hpp"
#include "util/ast_visitors/constant_expression.hpp"
#include "util/ast_visitors/eval_expression.hpp"
#include "util/ast_visitors/matching_exps.hpp"
#include "util/logger.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <tuple>

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// Generate SBG Input ----------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

// Constructors/Destructors ----------------------------------------------------

GenerateSBGInput::GenerateSBGInput(MMO_Class& mmo_class)
  : _mmo_class(mmo_class), _vertex_offset(0) {}

// Getters ---------------------------------------------------------------------

std::string GenerateSBGInput::fileName() { return _mmo_class.name()
  + "_sbg_input.sbg"; }

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
      set_vertex.addDimension(1, 1, d_val);
    }
    set_vertex.set_translation(Translation{_max_dim, _vertex_offset});
    _vertex_offset += set_vertex.maxDimSize() + 1;
  } else {
    set_vertex.addDimension(1, 1, 1);
    set_vertex.set_translation(Translation{_max_dim, _vertex_offset});
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

  for (const Name& var_name : variables) {
    VarInfo variable = symbols[var_name].get();
    if (isVariable(var_name, symbols)) {
      buildSet(variable, var_name);
    }
  }
}

// Add equations vertices ------------------------------------------------------

void GenerateSBGInput::buildEqualitySet(Equality eq, SetVertex& set_vertex)
{
  set_vertex.set_translation(Translation{_max_dim, _vertex_offset});
  _vertex_offset += set_vertex.maxDimSize() + 1;

  // Fill remaining dimensions
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

void GenerateSBGInput::buildForEqSet(ForEq eq, SetVertex set_vertex
  , IndexList indices)
{
  ForEq for_eq = get<ForEq>(eq);
  IndexList nested_indices = for_eq.range().indexes();
  indices.insert(indices.end(), nested_indices.begin(), nested_indices.end()); 
  set_vertex.cartesianProduct(SetVertex{_node_id
    , indicesToCompactSet(indices,  _mmo_class.syms())});
  for (const Equation& jth_eq : for_eq.elements()) {
    if (is<Equality>(jth_eq)) {
      Equality equality = get<Equality>(jth_eq);
      buildEqualitySet(equality, set_vertex);

      for (std::size_t k = indices.size(); k < _max_dim; ++k) {
        indices.emplace_back("dummy", Expression{0});
      }
      _equations_info[set_vertex.node_id()]
        = EquationInfo{indices, equality};
    } else if (is<ForEq>(jth_eq)) {
      buildForEqSet(get<ForEq>(jth_eq), set_vertex, indices);
    } else {
      ERROR("GenerateSBGInput::buildForEqSet: only equalities and for loops "
        , "supported\n");
    }
  }
}

void GenerateSBGInput::addEquationNodes()
{
  IndexList scalar_indices;
  for (std::size_t k = 0; k < _max_dim; ++k) {
    scalar_indices.emplace_back("dummy", Expression{0});
  }

  EquationList eqs = _mmo_class.equations().equations();
  for (const Equation& eq : eqs) {
    SetVertex set_vertex{_node_id};
    IndexList indices;
    if (is<ForEq>(eq)) {
      ForEq for_eq = get<ForEq>(eq);
      buildForEqSet(for_eq, set_vertex, indices);
    } else if (is<Equality>(eq)) {
      Equality equality = get<Equality>(eq);
      set_vertex.addDimension(1, 1, 1);
      buildEqualitySet(equality, set_vertex);
      _equations_info[set_vertex.node_id()]
        = EquationInfo{scalar_indices, equality};
    } else {
      ERROR("GenerateSBGInput::addEquationNodes: only equalities and for loops "
        , "supported\n");
    }
  }
}

// Add edges -------------------------------------------------------------------

CompactTransformation GenerateSBGInput::toTransformation(const Expression& expr
  , const IndexList& indices)
{
  assert(is<Reference>(expr));
  Reference occurrence = get<Reference>(expr);
  Ref names = occurrence.ref();
  assert(names.size() > 0);
  ExpList indexes = get<1>(names.front());

  std::vector<std::string> order;
  for (const Index& index : indices) {
    order.push_back(index.name());
  }

  CompactTransformation t{_max_dim};
  std::size_t i = 0;
  for (Expression index : indexes) {
    AffineExprVisitor affine_expr_visitor(_mmo_class.syms(), order);
    Util::AffineExpr ith_expr = Apply(affine_expr_visitor, index);
    t.setRow(i, ith_expr);
    ++i;
  }

  return t;
}

void GenerateSBGInput::addMaps(Expression expr, CompactSet eq_nodes
  , Translation eq_nodes_trans, const IndexList& indices)
{
  Translation domain_trans{_max_dim, _edge_offset};
  CompactSet domain = eq_nodes.translate(domain_trans);
  SetEdge se{_edge_id, domain};

  CompactTransformation map1(_max_dim);
  for (std::size_t k = 0; k < _max_dim; ++k) {
    map1.translation(k) = eq_nodes_trans[k] - domain_trans[k];
  }
  se.set_map1(map1);
  se.set_map2(toTransformation(expr, indices));
  _set_edges.push_back(se);

  _edge_offset += domain.maxDimSize() + 1;
  ++_edge_id;
}

void GenerateSBGInput::addEdges()
{
  VarSymbolTable symbols = _mmo_class.syms();

  for (auto& [eq_id, eq_info] : _equations_info) {
    const Equality eq = eq_info.equality();
    Expression left = eq.left();
    Expression right = eq.right();

    // Get vertices that represent this array of equations
    CompactSet eq_nodes;
    Translation eq_nodes_trans;
    for (const SetVertex& sv : _set_vertices) {
      if (sv.node_id() == eq_id) {
        eq_nodes = sv.set();
        eq_nodes_trans = sv.translation();
      } 
    }

    // Get vertices of variables that appear in this array of equations
    for (const SetVertex& sv : _set_vertices) {
      Name var_name = sv.name();
      if (var_name.substr(0, 3) == "eq_") {
        continue;
      }
      MatchingExps matching_exprs(var_name, isState(var_name, symbols));
      Apply(matching_exprs, left);
      Apply(matching_exprs, right);
      std::set<Expression> matched_exprs = matching_exprs.matchedExps();
      LOG << "Matched exprs for: " << var_name << " in " << eq << std::endl;
      for (const Expression& expr : matched_exprs) {
        LOG << "Expression: " << expr << std::endl;
        addMaps(expr, eq_nodes, eq_nodes_trans, eq_info.indices());
      }
    }
  }
}

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
  addEdges();
  for (auto s : _set_edges)
    std::cout << s << "\n";
  generateSBGInput();
}

void GenerateSBGInput::generateSBGInput()
{
  _sbg_input << "dims = " << _max_dim << ";\n";

  _sbg_input << "causalize(" << std::endl;

  _sbg_input << "V: {";
  unsigned long size = 1;
  for (const SetVertex& sv : _set_vertices) {
    _sbg_input << sv.toSBGFormat().str()
      << ((size < _set_vertices.size()) ? ", " : "");
    ++size;
  }
  _sbg_input << "}" << std::endl;

  _sbg_input << "Vmap: <<";
  size = 1;
  for (const SetVertex& sv : _set_vertices) {
    _sbg_input << "{" << sv.toSBGFormat().str() << "} -> ";
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

  // TODO: X and Y sets of bipartite SBG

  _sbg_input << ", 1);" << std::endl;

  _sbg_input.close();
}

} // namespace Causalize

} // namespace Modelica
