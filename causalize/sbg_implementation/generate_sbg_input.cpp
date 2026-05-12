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

#include <ast/queries.hpp>
#include <causalize/sbg_implementation/generate_sbg_input.hpp>
#include <util/ast_visitors/constant_expression.hpp>
#include <util/ast_visitors/eval_expression.hpp>
#include <util/ast_visitors/matching_exps.hpp>
#include <util/ast_visitors/affine_transform.hpp>
#include <util/logger.hpp>

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
  : _mmo_class(mmo_class), _vertex_offset(1) {}

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
  for (const Name& var_name : variables) {
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

detail::HyperRectangle GenerateSBGInput::indicesToHyperRect(
  const IndexList& indices) const
{
  detail::HyperRectangle result;

  for (const Index& index : indices) {
    OptExp expr = index.exp();
    if (!expr) {
      ERROR("GenerateSBGInput::indicesToHyperRect: empty index\n");
    } 
    else if (!is<Range>(expr.get())) {
      ERROR("GenerateSBGInput::indicesToHyperRect: only Range expressions "
        , "supported\n");
    }

    Range expr_range = get<Range>(expr.get());
    Integer start = getValue(expr_range.start());
    Integer step = 1;
    Integer end = getValue(expr_range.end());
    if (expr_range.step()) {
      step = getValue(expr_range.step().get());
    }

    result.addDimension(start, step, end);
  }

  if (result.arity() < _max_dim) {
    // TODO: fill remaining dimensions
  }

  return result;
}

void GenerateSBGInput::buildForEqSet(ForEq eq, SetVertex set_vertex)
{
  ForEq for_eq = get<ForEq>(eq);
  SetVertex set_vertex_copy = set_vertex;
  IndexList indices = for_eq.range().indexes();
  set_vertex_copy.cartesianProduct(SetVertex{_node_id
    , indicesToHyperRect(indices)});
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
  for (const Equation& eq : eqs)
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

void GenerateSBGInput::generateExpression(const SetVertex& sv
  , const Expression& expr, EquationInfo& eq_info
  , detail::HyperRectangle domain)
{
  assert(is<Reference>(expr));

  Reference occur = get<Reference>(expr);
  Ref names = occur.ref();
  assert(names.size() > 0);
  ExpList indexes = get<1>(names.front());

  SetEdge se{_edge_id, domain};
  for (Expression index : indexes) {
    //AffineTransformVisitor affine_visitor(symbols);
    //AffineTransformation A = Apply(affine_visitor, indexes);
  }
  // add to map1 or map2 if it is a state variable or not 

  // update _edge_offset;

  ++_edge_id;
}

void GenerateSBGInput::addEdges()
{
  VarSymbolTable symbols = _mmo_class.syms();

  for (auto& eq_info_pair : _equations_info) {
    const EquationInfo& eq_info = eq_info_pair.second;
    const Equality eq = eq_info.equality();
    Expression left = eq.left();
    Expression right = eq.right();
    detail::HyperRectangle domain = indicesToHyperRect(eq_info.indices());

    for (const SetVertex& sv : _set_vertices) {
      Name var_name = sv.name();
      MatchingExps matching_exprs(var_name, isState(var_name, symbols));
      Apply(matching_exprs, left);
      Apply(matching_exprs, right);
      std::set<Expression> matched_exprs = matching_exprs.matchedExps();
      LOG << "Matched exprs for: " << var_name << " in " << eq << std::endl;
      for (const Expression& expr : matched_exprs) {
        LOG << "Expression: " << expr << std::endl;
        //generatePWLMaps(sv, expr, eq_info, domain);
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
