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

#ifndef MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_SET_EDGE_HPP_
#define MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_SET_EDGE_HPP_

#include "ast/expression.hpp"
#include "causalize/sbg_implementation/equation_info.hpp"
#include "util/affine_transformation.hpp"
#include "util/compact_set.hpp"

#include <iosfwd>
#include <sstream>
#include <string>
#include <vector>

namespace Modelica {

namespace Causalize {

/**
 * @class SetEdge
 * @brief Interface class between usages of variables in equations of a model,
 * and set-edges of the corresponding causalization SBG. 
 */
class SetEdge {
public:
  SetEdge(int edge_id, std::size_t arity);
  SetEdge(int edge_id, CompactSet domain);

  int edge_id() const;
  std::string name() const;
  int var_id() const;
  int eq_id() const;
  const Translation& translation() const;
  const CompactSet& domain() const;
  const CompactTransformation& map1() const;
  const CompactTransformation& map2() const;
  const Expression& access() const;
  std::size_t arity() const;

  void set_edge_id(int edge_id);
  void set_name(std::string name);
  void set_var_id(int var_id);
  void set_eq_id(int eq_id);
  void set_translation(Translation translation);
  void set_domain(CompactSet domain);
  void set_map1(CompactTransformation map1);
  void set_map2(CompactTransformation map2);
  void set_access(Expression access);

  std::string domainToSBGFormat() const;
  std::string map1ToSBGFormat() const;
  std::string map2ToSBGFormat() const;
  std::string toSBGFormat() const;

  /**
   * @brief Returns the maximum coordinate of the perimeter of _domain.
   */
  AST::Integer maxDimPerimetral();

  /**
   * @brief Returns a new set-edge with the same values, except for the domain
   * which is restricted to \p new_domain.
   */
  SetEdge restrict(CompactSet new_domain) const;

private:
  int _edge_id;
  std::string _name;
  int _var_id;
  int _eq_id;
  Translation _translation; ///< Avoids domain values collision
  CompactSet _domain;
  CompactTransformation _map1; ///< Map to equations nodes
  CompactTransformation _map2; ///< Map to variables nodes
  Expression _access; ///< Expression of the access to the variable
};

std::ostream& operator<<(std::ostream& out, const SetEdge& sv);

using SetEdges = std::vector<SetEdge>;

} // namespace Causalize

} // namespace Modelica

#endif // MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_SET_EDGE_HPP_
