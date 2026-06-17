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

#include "causalize/sbg_implementation/tearing.hpp"
#include "ast/queries.hpp"
#include "causalize/sbg_implementation/set_edge.hpp"
#include "causalize/sbg_implementation/set_vertex.hpp"
#include "util/debug.hpp"
#include "util/affine_transformation.hpp"
#include "util/compact_set.hpp"
#include "util/translation.hpp"

#include <algorithms/mfvs/min_feedback_vertex_set.hpp>
#include <algorithms/misc/causalization_builders.hpp>
#include <sbg/directed_sbg.hpp>
#include <sbg/map.hpp>
#include <sbg/set.hpp>

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// Tearing variables -----------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

// TearingVariable -------------------------------------------------------------

TearingVariable::TearingVariable(AST::Name name, AST::Bracket subscripts)
  : _name(name), _subscripts(subscripts) {}

const AST::Name& TearingVariable::name() const { return _name; }

const AST::Bracket& TearingVariable::subscripts() const
{
  return _subscripts;
}

std::ostream& operator<<(std::ostream& out, const TearingVariable& var)
{
  out << var.name() << var.subscripts();
  return out;
}

// TearingVariables ------------------------------------------------------------

auto TearingVariables::begin() const { return _variables.begin(); }

auto TearingVariables::end() const { return _variables.end(); }

std::set<AST::Bracket>& TearingVariables::operator[](AST::Name name)
{
  return _variables[name];
}

const std::set<AST::Bracket>& TearingVariables::operator[](AST::Name name) const
{
  return _variables.at(name);
}

void TearingVariables::insert(TearingVariable variable)
{
  _variables[variable.name()].insert(variable.subscripts());
}

std::ostream& operator<<(std::ostream& out, const TearingVariables& vars)
{
  for (auto const& [name, access] : vars) {
    out << name;
    for (const AST::Bracket& subs : access) {
      out << subs;
    }
    out << "\n";
  }
  return out;
}

////////////////////////////////////////////////////////////////////////////////
// Tearing return structure ----------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

TearingResult::TearingResult(SBG::LIB::Set mfvs_result)
  : _mfvs_result(mfvs_result) {}

const SBG::LIB::Set& TearingResult::mfvs_result() const { return _mfvs_result; }

CompactSet image(CompactSet domain, CompactTransformation trans)
{
  SBG::LIB::Map m{domain.set(), trans.expr()};
  return CompactSet{m.image()};
}

void variableToModelica(const SetEdge& se
  , const SetVertices& set_vertices, CompactSet jth_tear
  , TearingVariables& modelica_vars)
{
  // Get variable information that is accessed by the set-edge
  AST::Name name;
  std::size_t arity = se.domain().arity();
  Translation sv_translation{arity};
  for (const SetVertex& sv : set_vertices) {
    if (sv.node_id() == se.var_id()) {
      name = sv.name();
      sv_translation = sv.translation();
      break;
    }
  }

  // Convert jth_tear to bracket expression and save it to result
  CompactSet img = image(se.domain(), se.map2());
  std::vector<AST::Indexes> indices = toModelicaIndices(img, sv_translation
    , std::vector<AST::Name>{arity, ""});
  for (const AST::Indexes& indexes : indices) {
    AST::ExpList expr_list;
    for (const AST::Index& index : indexes.indexes()) {
      ERROR_UNLESS(index.exp().has_value(), "TearingDetector::variableToModelica: "
        , "empty index");
      expr_list.push_back(index.exp().value());
    }
    AST::Bracket subscripts{ExpListList{1, expr_list}};
    modelica_vars.insert(TearingVariable{name, subscripts});
  }
}

TearingVariables TearingResult::toModelicaFormat(const SetVertices& set_vertices
  , const SetEdges& set_edges) const
{
  TearingVariables result;

  CompactSet mfvs{_mfvs_result};
  for (const SetEdge& se : set_edges) {
    CompactSet jth_domain = se.domain();
    jth_domain.intersection(mfvs);
    if (jth_domain.cardinal() > 0) {
      variableToModelica(se, set_vertices, jth_domain, result);
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
// Tearing variables detector --------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

TearingDetector::TearingDetector(AlgebraicLoopsInfo loops_info)
  : _loops_info(loops_info) {}

TearingResult TearingDetector::detect()
{
  SBG::LIB::DirectedSBG dsbg = misc::buildTearingSBG(_loops_info.scc_result());
  return TearingResult{SBG::LIB::MinFeedbackVertexSet{}.calculate(dsbg)};
}

} // namespace Causalize

} // namespace Modelica
