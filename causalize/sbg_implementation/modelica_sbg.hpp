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

#include <string>
#include <tuple>

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
  ModelicaSBG();

  const SetVertices& set_vertices() const;
  const SetEdges& set_edges() const;
  SetEdges& set_edges();

  void set_arity(std::size_t arity);

  void addSetVertex(SetVertex sv);

  /**
   * @brief TODO
   * @result Identifier of the newly created set-vertex.
   */
  int addSetVertex(CompactSet elems, std::string name);

  int addSetVertex(
    CompactSet elems, std::string name, VertexInfo info
  );

  void addSetEdge(SetEdge se);

  /**
   * @brief TODO
   * @result Identifier of the newly created set-edge.
   */
  int addSetEdge(
    int var_id, int eq_id, CompactSet domain, AST::Expression access
    , std::string name
  );

  int addSetEdge(
    int var_id, int eq_id, CompactSet domain, Translation t
    , AST::Expression access, std::string name
  );

  int addSetEdge(
    int var_id, int eq_id, CompactSet domain, CompactTransformation map1
    , CompactTransformation map2, AST::Expression access, std::string name
  );

  /**
   * @brief Returns the set-vertex identified by \p id.
   */
  SetVertex& setVertex(int id);
  const SetVertex& setVertex(int id) const;

  /**
   * @brief Returns the set-vertex identified by \p id.
   */
  SetEdge& setEdge(int id);
  const SetEdge& setEdge(int id) const;

private:
  SetVertices _set_vertices;
  SetEdges _set_edges;
  Integer _vertex_offset;
  Integer _edge_offset;
  std::size_t _arity;
};

std::ostream& operator<<(std::ostream& out, const ModelicaSBG& bsbg);

// Non-member functions --------------------------------------------------------

using EquationAccess = std::pair<EquationInfo, Accesses>;

/**
 * @brief Given a set-edge, and a sub-group of elements of that set-edge, get
 * the corresponding equation expression, and Modelica indices to access them.
 */
EquationAccess getAccess(
  const ModelicaSBG& modelica_sbg, const SetEdge& se, const CompactSet& s
);

/**
 * @brief Given a set-edge, and a sub-group of elements of that set-edge, get
 * the corresponding equation expression, and Modelica sorted indices according
 * to \p expr.
 */
EquationAccess getAccess(
  const ModelicaSBG& modelica_sbg, const SetEdge& se, const CompactSet& s
  , const SBG::LIB::Expression& expr
);

} // namespace Causalize

} // namespace Modelica

#endif // MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_MODELICA_SBG_HPP_
