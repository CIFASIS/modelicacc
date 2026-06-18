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

#ifndef MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_SET_VERTEX_HPP_
#define MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_SET_VERTEX_HPP_

#include "ast/expression.hpp"
#include "causalize/sbg_implementation/equation_info.hpp"
#include "util/compact_set.hpp"

#include <iosfwd>
#include <optional>
#include <sstream>
#include <string>

namespace Modelica {

namespace Causalize {

/**
 * @brief Additional information attached to a set-vertex. Used to save
 * information in the equation vertices.
 */
using VertexInfo = std::optional<EquationInfo>;

/**
 * @brief Interface class between arrays of variables and equations of a model,
 * and set-vertices of the corresponding causalization SBG.
 */
class SetVertex {
public:
  SetVertex(int node_id);
  SetVertex(int node_id, CompactSet s);

  int node_id() const;
  std::size_t arity() const;
  std::string name() const;
  const CompactSet& set() const;
  const Translation& translation() const;
  const VertexInfo& info() const;

  void set_node_id(int node_id);
  void set_name(std::string name);
  void set_translation(Translation translation);
  void set_info(VertexInfo info);

  bool isVariable() const;
  bool isEquation() const;

  /**
   * @brief Prints the corresponding array in SBG program format.
   */
  std::ostream& print(std::ostream& out) const;
  std::string toSBGFormat() const;

  void cartesianProduct(const CompactSet& s);
  void cartesianProduct(const SetVertex& other);

  /**
   * @brief Returns the maximum coordinate of the perimeter of _set.
   */
  AST::Integer maxDimPerimetral() const;

private:
  int _node_id;
  std::string _name;
  CompactSet _set;
  Translation _translation;
  VertexInfo _info;
};

std::ostream& operator<<(std::ostream& out, const SetVertex& sv);

using SetVertices = std::vector<SetVertex>;

} // namespace Causalize

} // namespace Modelica

#endif // MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_SET_VERTEX_HPP_
