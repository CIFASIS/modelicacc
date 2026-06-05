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

#include "ast/queries.hpp"
#include "causalize/sbg_implementation/tearing.hpp"
#include "util/debug.hpp"

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
// Tearing variables detector --------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

TearingDetector::TearingDetector(AlgebraicLoopsInfo loops_info)
  : _loops_info(loops_info) {}

CompactSet image(CompactSet domain, CompactTransformation trans)
{
  SBG::LIB::Map m{domain.set(), trans.expr()};
  return CompactSet{m.image()};
}

void TearingDetector::detectVariable(SetEdge se, CompactSet jth_tear)
{
  // Get variable information that is accessed by the set-edge
  Name name;
  std::size_t arity = se.domain().arity();
  Translation sv_translation{arity};
  for (const SetVertex& sv : _loops_info.set_vertices()) {
    if (sv.node_id() == se.var_id()) {
      name = sv.name();
      sv_translation = sv.translation();
      break;
    }
  }

  // Convert jth_tear to bracket expression and save it to result
  CompactSet img = image(se.domain(), se.map2());
  std::vector<Indexes> indices = img.toModelicaIndices(sv_translation
    , std::vector<AST::Name>{arity, ""});
  for (const Indexes& indexes : indices) {
    ExpList expr_list;
    for (const Index& index : indexes.indexes()) {
      ERROR_UNLESS(index.exp().has_value(), "TearingDetector::detectVariable: "
        , "empty index");
      expr_list.push_back(index.exp().value());
    }
    Bracket subscripts{ExpListList{1, expr_list}};
    _tearing.insert(TearingVariable{name, subscripts});
  }
}

TearingVariables TearingDetector::detect()
{
  SBG::LIB::DirectedSBG dsbg = misc::buildTearingSBG(_loops_info.scc_result());
  CompactSet mfvs{SBG::LIB::MinFeedbackVertexSet{}.calculate(dsbg)};

  for (const SetEdge& se : _loops_info.set_edges()) {
    CompactSet jth_domain = se.domain();
    jth_domain.intersection(mfvs);
    if (jth_domain.cardinal() > 0) {
      detectVariable(se, jth_domain);
    }
  }

  return _tearing;
}

} // namespace Causalize

} // namespace Modelica
