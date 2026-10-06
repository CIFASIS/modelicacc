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

#include "flatter/generate_sbg_file.hpp"
#include "ast/ast_types.hpp"
#include "ast/queries.hpp"
#include "util/logger.hpp"
#include "util/ast_visitors/flatter_for.hpp"
#include "util/ast_visitors/is_connect.hpp"
#include "util/ast_visitors/matching_exps.hpp"
#include "util/ast_visitors/matching_exps_equation.hpp"
#include "util/sbg/ast_visitors/equation_info_visitor.hpp"
#include "util/sbg/ast_visitors/equation_sbg_set.hpp"
#include "util/sbg/ast_visitors/sbg_expr_visitor.hpp"
#include "util/sbg/ast_visitors/sbg_expr_visitor.hpp"

#include <sbgraph/eval/base_type.hpp>
#include <sbgraph/eval/file_evaluator.hpp>
#include <sbgraph/eval/pretty_print.hpp>
#include <sbgraph/sbg/integer.hpp>
#include <sbgraph/sbg/rational.hpp>
#include <sbgraph/sbg/set.hpp>

namespace Modelica {

namespace Flatter {

////////////////////////////////////////////////////////////////////////////////
// SBG generation return structure ---------------------------------------------
////////////////////////////////////////////////////////////////////////////////

SBGGenerationResult::SBGGenerationResult(
  MMO_Class& mmo_class
  , ModelicaSBG modelica_sbg
  , SBG::LIB::SBG bipartite_sbg
) : _mmo_class(mmo_class), _modelica_sbg(modelica_sbg), _sbg(bipartite_sbg) {}

const MMO_Class& SBGGenerationResult::mmo_class() const
{
  return _mmo_class;
}

const ModelicaSBG& SBGGenerationResult::modelica_sbg() const
{
  return _modelica_sbg;
}

const SBG::LIB::SBG& SBGGenerationResult::sbg() const
{
  return _sbg;
}

////////////////////////////////////////////////////////////////////////////////
// Generate SBG Input ----------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

// Constructors/Destructors ----------------------------------------------------

GenerateSBGInput::GenerateSBGInput(MMO_Class& mmo_class)
    : _mmo_class(mmo_class), _max_dim(0) {}

// Getters ---------------------------------------------------------------------

std::string GenerateSBGInput::fileName()
{
  return _mmo_class.name() + "_sbg_file.sbg";
}

// Add variable vertices -------------------------------------------------------

void GenerateSBGInput::addSetVertex(
  const AST::Name& var_name, const VarInfo& var_info
)
{
  // Use variable definition information.
  SBG::LIB::Set var_set = varInfoToSBGSet(var_info, _mmo_class.syms());

  // Fill remaining dimensions
  for (std::size_t k = var_set.arity(); k < _max_dim; ++k) {
    var_set = var_set.cartesianProduct(SBG::LIB::Set{1, 1, 1});
  }

  // Save variable set-vertex
  _modelica_sbg.addSetVertex(var_set, var_name, var_info);
}

void GenerateSBGInput::addVariableNodes()
{
  AST::IdentList variables = _mmo_class.variables();
  VarSymbolTable symbols = _mmo_class.syms();

  for (const auto& [var_name, var_info] : symbols) {
    VarInfo variable = symbols[var_name].get();
    addSetVertex(var_name, var_info);
  }
}

// Add edges -------------------------------------------------------------------

SBG::LIB::Expression GenerateSBGInput::createEdgeVarMap(
  const AST::Equation& eq, const SetEdge& eq_se, const SetVertex& var_sv
  , const AST::Expression& access
) const
{
  // Get counters of the loop in order.
  EqInfoVisitor eq_info_visit{_max_dim};
  EquationInfo eq_info = Apply(eq_info_visit, eq);
  Counters counters;
  for (const Index& counter : eq_info.indices().indexes()) {
    counters.push_back(counter.name());
  }

  // Transform subscripts to SBG expression.
  SBG::LIB::Expression sbg_expr = referenceToExpression(
    access, _mmo_class.syms(), counters
  );

  // Get translation of set-edge and variable set-vertex.
  SBG::LIB::Rational zero{0};
  SBG::LIB::IntTuple domain_trans = eq_se.translation();
  SBG::LIB::IntTuple var_nodes_trans = var_sv.translation();
  SBG::LIB::Expression translation;
  for (std::size_t k = 0; k < _max_dim; ++k) {
    SBG::LIB::Int offset = var_nodes_trans[k] - domain_trans[k];
    if (eq_info.scalar() || var_sv.set().cardinal() == 1) {
      offset = offset + domain_trans[k];
    }
    translation = translation.cartesianProduct(
      SBG::LIB::Expression{zero, offset}
    );
  }

  return sbg_expr + translation;
}

std::pair<SetVertex, SetVertex> GenerateSBGInput::endpoints(
  const AST::Equation& eq
)
{
  VarSymbolTable symbols = _mmo_class.syms();
  SetVertex left_sv{-1};
  SetVertex right_sv{-1};
  _matched_exprs = std::set<AST::Expression>{};
  SetVertices svs = _modelica_sbg.set_vertices();
  for (const SetVertex& sv : svs) {
    AST::Name var_name = sv.name();
    MatchingExpsEquation matching_exprs{var_name, isState(var_name, symbols)};
    bool matched = Apply(matching_exprs, eq);
    if (matched) {
      auto jth_matched = matching_exprs.matching_exprs().matchedExps();
      _matched_exprs.insert(jth_matched.begin(), jth_matched.end());
      if (jth_matched.size() == 2) {
        auto it = jth_matched.begin();
        left_sv = sv;
        _edges_to_left[left_sv.node_id()] = *it;
        right_sv = sv;
        ++it;
        _edges_to_right[right_sv.node_id()] = *it;
        break;
      } else {
        if (left_sv.node_id() == - 1) {
          left_sv = sv;
          _edges_to_left[left_sv.node_id()] = *(jth_matched.begin());
        } else {
          right_sv = sv;
          _edges_to_right[right_sv.node_id()] = *(jth_matched.begin());
          break;
        }
      }
    }
  }
  ERROR_UNLESS(_matched_exprs.size() == 2, "GenerateSBGInput::endpoints:"
    , " connect should involve exactly two variables ", eq, " instead of "
    , _matched_exprs.size());

  return {left_sv, right_sv};
}

void GenerateSBGInput::addSetEdge(const AST::Equation& eq)
{
  // Get connected set-vertices by this set-edge.
  auto [left_sv, right_sv] = endpoints(eq);

  // Get set-edge domain.
  EquationToSBGSet domain_visit{_mmo_class.syms(), _max_dim};
  SBG::LIB::Set domain = Apply(domain_visit, eq);

  // Create edge.
  std::string name = left_sv.name() + "-" + right_sv.name();
  int se_id = _modelica_sbg.addSetEdge(
    left_sv.node_id(), right_sv.node_id(), domain, AST::Expression{}, name
  );

  // Set maps.
  SetEdge& se = _modelica_sbg.setEdge(se_id);
  se.set_map1(createEdgeVarMap(
    eq, se, left_sv, _edges_to_left[left_sv.node_id()])
  );
  se.set_map2(createEdgeVarMap(
    eq, se, right_sv, _edges_to_right[right_sv.node_id()])
  );
}

void GenerateSBGInput::addSetEdges()
{
  // Flatter equations.
  AST::EquationList eq_list;
  for (const Equation& eq : _mmo_class.equations().equations()) {
    FlatterForVisitor flatter_for;
    AST::EquationList jth_flatter = Apply(flatter_for, eq);
    eq_list.insert(eq_list.end(), jth_flatter.begin(), jth_flatter.end());
  }

  // Distinguish Connect equations.
  IsConnect is_connect;
  AST::EquationList connect_list;
  for (const AST::Equation& eq : eq_list) {
    if (Apply(is_connect, eq)) {
      connect_list.push_back(eq);
    } else {
      _not_connect.push_back(eq);
    }
  }

  // Add set-edge.
  VarSymbolTable symbols = _mmo_class.syms();
  for (const Equation& eq : connect_list) {
    addSetEdge(eq);
  }
}

// Generate SBG program --------------------------------------------------------

void GenerateSBGInput::setup()
{
  AST::IdentList variables = _mmo_class.variables();
  VarSymbolTable symbols = _mmo_class.syms();

  _sbg_file.open(fileName());
  _max_dim = 1;

  // Get maximum dimension  between the arrays of variables defined in the model
  for (AST::Name var_name : variables) {
    VarInfo variable = symbols[var_name].get();
    if (isVariable(var_name, symbols)) {
      Option<AST::ExpList> indexes = variable.indices();
      if (indexes && (indexes->size() > _max_dim)) {
        _max_dim = indexes->size();
      }
    }
  }

  _modelica_sbg.set_arity(_max_dim);
}

SBGGenerationResult GenerateSBGInput::buildFromModel()
{
  // Write SBG program to _sbg_file
  setup();
  addVariableNodes();
  addSetEdges();
  generateSBGInput();

  // Evaluate SBG program to obtain bipartite SBG
  SBG::LIB::SBG g;
  SBG::Eval::ProgramIO eval_result = SBG::Eval::parseEvalFile(fileName());
  for (SBG::Eval::ExprResult ev : eval_result.exprs()) {
    SBG::Eval::ExprBaseType e = std::get<1>(ev);
    if (std::holds_alternative<SBG::LIB::SBG>(e)) {
      g = std::get<SBG::LIB::SBG>(e);
    }
  }

  SBG::LIB::Set positive_degree = g.map1().image().cup(g.map2().image());
  g.eraseVertices(g.V().difference(positive_degree));

  return SBGGenerationResult{_mmo_class, _modelica_sbg, g};
}

void GenerateSBGInput::generateVSet()
{
  _sbg_file << "V: ";
  SBG::LIB::Set V;
  for (const SetVertex& sv : _modelica_sbg.set_vertices()) {
    SBG::LIB::Set jth_set = sv.set();
    jth_set = jth_set.translate(sv.translation());
    V = V.disjointCup(jth_set);
  }
  _sbg_file << V << std::endl;
}

void GenerateSBGInput::generateVMap()
{
  _sbg_file << "Vmap: <<";
  SetVertices svs = _modelica_sbg.set_vertices();
  std::size_t size = svs.size();
  std::size_t j = 1;
  for (const SetVertex& sv : svs) {
    _sbg_file << sv.toSBGFormat() << " -> ";
    for (std::size_t k = 0; k + 1 < _max_dim; ++k) {
      _sbg_file << "|0*x+" << j;
    }
    _sbg_file << "|0*x+" << j;
    _sbg_file << ((j < size) ? "|, " : "|");
    ++j;
  }
  _sbg_file << ">>" << std::endl;
}

void GenerateSBGInput::generateMap1()
{
  _sbg_file << "map1: <<";
  SetEdges ses = _modelica_sbg.set_edges();
  std::size_t size = ses.size();
  std::size_t j = 1;
  for (const SetEdge& se : ses) {
    _sbg_file << se.domainToSBGFormat() << " -> ";
    _sbg_file << se.map1ToSBGFormat();
    _sbg_file << ((j < size) ? ", " : "");
    ++j;
  }
  _sbg_file << ">>" << std::endl;
}

void GenerateSBGInput::generateMap2()
{
  _sbg_file << "map2: <<";
  SetEdges ses = _modelica_sbg.set_edges();
  std::size_t size = ses.size();
  std::size_t j = 1;
  for (const SetEdge& se : ses) {
    _sbg_file << se.domainToSBGFormat() << " -> ";
    _sbg_file << se.map2ToSBGFormat();
    _sbg_file << ((j < size) ? ", " : "");
    ++j;
  }
  _sbg_file << ">>" << std::endl;
}

void GenerateSBGInput::generateEMap()
{
  _sbg_file << "Emap: <<";
  SetEdges ses = _modelica_sbg.set_edges();
  std::size_t size = ses.size();
  std::size_t j = 1;
  for (const SetEdge& se : ses) {
    _sbg_file << se.domainToSBGFormat() << " -> ";
    for (std::size_t k = 0; k + 1 < _max_dim; ++k) {
      _sbg_file << "|0*x+" << j;
    }
    _sbg_file << "|0*x+" << j;
    _sbg_file << ((j < size) ? "|, " : "|");
    ++j;
  }
  _sbg_file << ">>" << std::endl;
}

void GenerateSBGInput::generateSBGInput()
{
  _sbg_file << "dims = " << _max_dim << ";\n";

  generateVSet();
  generateVMap();
  generateMap1();
  generateMap2();
  generateEMap();
  _sbg_file << ";";

  _sbg_file.close();
}

}  // namespace Flatter

}  // namespace Modelica
