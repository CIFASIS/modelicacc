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

#include "causalize/sbg_implementation/modelica_sbg.hpp"
#include "util/debug.hpp"
#include "util/sbg_interface.hpp"

#include <algorithm>
#include <variant>

namespace Modelica {

namespace Causalize {

// Constructors/Destructors ----------------------------------------------------

ModelicaSBG::ModelicaSBG() : _vertex_offset(0), _edge_offset(0), _arity(0) {}

// Getters ---------------------------------------------------------------------

const SetVertices& ModelicaSBG::set_vertices() const { return _set_vertices; }

const SetEdges& ModelicaSBG::set_edges() const { return _set_edges; }

SetEdges& ModelicaSBG::set_edges() { return _set_edges; }

SetVertex& ModelicaSBG::setVertex(int id)
{
  for (SetVertex& sv : _set_vertices) {
    if (sv.node_id() == id) {
      return sv;
    }
  }

  ERROR("ModelicaSBG::setVertex: set-vertex ", id, " doesn't exist");
}

const SetVertex& ModelicaSBG::setVertex(int id) const
{
  for (const SetVertex& sv : _set_vertices) {
    if (sv.node_id() == id) {
      return sv;
    }
  }

  ERROR("ModelicaSBG::setVertex: set-vertex ", id, " doesn't exist");
}

SetEdge& ModelicaSBG::setEdge(int id)
{
  for (SetEdge& se : _set_edges) {
    if (se.edge_id() == id) {
      return se;
    }
  }

  ERROR("ModelicaSBG::setEdge: set-edge ", id, " doesn't exist");
}

const SetEdge& ModelicaSBG::setEdge(int id) const
{
  for (const SetEdge& se : _set_edges) {
    if (se.edge_id() == id) {
      return se;
    }
  }

  ERROR("ModelicaSBG::setEdge: set-edge ", id, " doesn't exist");
}

// Setters ---------------------------------------------------------------------

void ModelicaSBG::set_arity(std::size_t arity) { _arity = arity; }

void ModelicaSBG::addSetVertex(SetVertex sv)
{
  if (sv.set().cardinal() > 0) {
    int max_node_id = _set_vertices.size() + 1;
    sv.set_node_id(max_node_id);
    _set_vertices.push_back(sv);
    SBG::LIB::Set translated = sv.set();
    translated = translated.translate(sv.translation());
    _vertex_offset = maxDimPerimetral(translated);
  }
}

int ModelicaSBG::addSetVertex(
  SBG::LIB::Set elems, std::string name
)
{
  int max_node_id = _set_vertices.size() + 1;
  SetVertex sv{max_node_id, elems};
  sv.set_name(name);
  sv.set_translation(SBG::LIB::IntTuple{elems.arity(), _vertex_offset});

  SBG::LIB::Set translated = sv.set();
  translated = translated.translate(sv.translation());
  _vertex_offset = maxDimPerimetral(translated);
  _set_vertices.push_back(sv);

  return max_node_id;
}

int ModelicaSBG::addSetVertex(
  SBG::LIB::Set elems, std::string name, VertexInfo info
)
{
  int node_id = addSetVertex(elems, name);
  SetVertex& sv = setVertex(node_id);
  sv.set_info(info);

  return node_id;
}

void ModelicaSBG::addSetEdge(SetEdge se)
{
  if (se.domain().cardinal() > 0) {
    int max_edge_id = _set_edges.size() + 1;
    se.set_edge_id(max_edge_id);

    SBG::LIB::Set translated = se.domain();
    translated = translated.translate(se.translation());
    _edge_offset = maxDimPerimetral(translated);
    _set_edges.push_back(se);
  }
}

int ModelicaSBG::addSetEdge(
  int var_id, int eq_id, SBG::LIB::Set domain, AST::Expression access
  , std::string name
)
{
  std::size_t arity = domain.arity();
  return addSetEdge(
    var_id, eq_id
    , domain, SBG::LIB::Expression{arity}, SBG::LIB::Expression{arity}
    , access, name
  );
}

int ModelicaSBG::addSetEdge(
  int var_id, int eq_id, SBG::LIB::Set domain, SBG::LIB::IntTuple t
  , AST::Expression access, std::string name
)
{
  std::size_t arity = domain.arity();
  int max_edge_id = _set_edges.size();
  SetEdge se{max_edge_id, domain};
  se.set_translation(t);
  se.set_var_id(var_id);
  se.set_eq_id(eq_id);
  se.set_map1(SBG::LIB::Expression{arity});
  se.set_map2(SBG::LIB::Expression{arity});
  se.set_name(name);
  se.set_access(access);

  SBG::LIB::Set translated = se.domain();
  translated = translated.translate(se.translation());
  _edge_offset = maxDimPerimetral(translated);
  _set_edges.push_back(se);

  return max_edge_id;
}

int ModelicaSBG::addSetEdge(
  int var_id, int eq_id, SBG::LIB::Set domain
  , SBG::LIB::Expression map1, SBG::LIB::Expression map2
  , AST::Expression access, std::string name
)
{
  int max_edge_id = _set_edges.size();
  SetEdge se{max_edge_id, domain};
  se.set_var_id(var_id);
  se.set_eq_id(eq_id);
  se.set_map1(map1);
  se.set_map2(map2);
  se.set_translation(SBG::LIB::IntTuple{domain.arity(), _edge_offset});
  se.set_name(name);
  se.set_access(access);

  SBG::LIB::Set translated = se.domain();
  translated = translated.translate(se.translation());
  _edge_offset = maxDimPerimetral(translated);
  _set_edges.push_back(se);

  return max_edge_id;
}

// Non-member functions --------------------------------------------------------

EquationAccess getAccess(
  const ModelicaSBG& modelica_sbg, const SetEdge& se, const SBG::LIB::Set& s
)
{
  // Get equation set-vertex referenced by the set-edge.
  SetVertex eq_sv = modelica_sbg.setVertex(se.eq_id());

  // Get adjusted indices of the equation.
  EquationInfo eq_info = std::get<EquationInfo>(eq_sv.info());
  std::vector<Name> counters;
  for (const Index& index : eq_info.indices().indexes()) {
    counters.push_back(index.name());
  }

  return {eq_info, toModelicaIndices(s, se.translation(), counters)};
}

EquationAccess getAccess(
  const ModelicaSBG& modelica_sbg, const SetEdge& se, const SBG::LIB::Set& s
  , const SBG::LIB::Expression& expr
)
{
  // Get equation set-vertex referenced by the set-edge.
  SetVertex eq_sv = modelica_sbg.setVertex(se.eq_id());

  // Get adjusted indices of the equation.
  EquationInfo eq_info = std::get<EquationInfo>(eq_sv.info());
  std::vector<Name> counters;
  for (const Index& index : eq_info.indices().indexes()) {
    counters.push_back(index.name());
  }

  return {eq_info, toModelicaIndices(s, se.translation(), counters, expr)};
}

}  // namespace Causalize

}  // namespace Modelica
