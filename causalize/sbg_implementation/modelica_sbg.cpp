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

namespace Modelica {

namespace Causalize {

// Constructors/Destructors ----------------------------------------------------

ModelicaSBG::ModelicaSBG(std::size_t arity) : _arity(arity) {}

// Getters ---------------------------------------------------------------------

std::size_t ModelicaSBG::arity() const { return _arity; }

const SetVertices& ModelicaSBG::set_vertices() const { return _set_vertices; }

const SetEdges& ModelicaSBG::set_edges() const { return _set_edges; }

SetVertex ModelicaSBG::setVertex(int id) const
{
  for (const SetVertex& sv : _set_vertices) {
    if (sv.node_id() == id) {
      return sv;
    }
  }

  ERROR("ModelicaSBG::setVertex: set-vertex ", id, " doesn't exist");
  return SetVertex{-1};
}

// Setters ---------------------------------------------------------------------

void ModelicaSBG::addSetVertex(SetVertex sv) { _set_vertices.push_back(sv); }

void ModelicaSBG::addSetEdge(SetEdge se)
{
  if (se.domain().cardinal() > 0) {
    _set_edges.push_back(se);
  }
}

// Non-member functions --------------------------------------------------------

EquationAccess getAccess(
  const ModelicaSBG& modelica_sbg, const SetEdge& se, const CompactSet& s
)
{
  // Get equation set-vertex referenced by the set-edge.
  SetVertex eq_sv = modelica_sbg.setVertex(se.eq_id());

  // Get adjusted indices of the equation.
  EquationInfo eq_info = eq_sv.info().value();
  std::vector<Name> counters;
  for (const Index& index : eq_info.indices().indexes()) {
    counters.push_back(index.name());
  }

  return {eq_info, toModelicaIndices(s, se.translation(), counters)};
}

}  // namespace Causalize

}  // namespace Modelica
