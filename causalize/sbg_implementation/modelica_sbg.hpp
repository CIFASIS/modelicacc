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

#ifndef MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_MODELICA_SBG_HPP_
#define MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_MODELICA_SBG_HPP_

#include "causalize/sbg_implementation/set_edge.hpp"
#include "causalize/sbg_implementation/set_vertex.hpp"

namespace Modelica {

namespace Causalize {

/**
 * @class ModelicaSBG
 * @brief Interface class between the SBG::LIB::BipartiteSBG structure and
 * the generated result of this module. It keeps track of the relations
 * between Modelica structures (equations and variables) and the corresponding
 * SBG values.
 */
class ModelicaSBG {
public:
  ModelicaSBG() = default;
  ModelicaSBG(std::size_t arity);

  std::size_t arity() const;
  const SetVertices& set_vertices() const;
  const SetEdges& set_edges() const;

  void addSetVertex(SetVertex sv);
  void addSetVertices(const SetVertices& svs);
  void addSetEdge(SetEdge se);

  /**
   * @brief Returns the  set-vertex identified by \p id.
   */
  SetVertex setVertex(int id) const;

private:
  std::size_t _arity;
  SetVertices _set_vertices;
  SetEdges _set_edges;
};

std::ostream& operator<<(std::ostream& out, const ModelicaSBG& bsbg);

} // namespace Causalize

} // namespace Modelica

#endif // MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_MODELICA_SBG_HPP_
