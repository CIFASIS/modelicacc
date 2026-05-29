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
#include "util/affine_transformation.hpp"
#include "util/compact_set.hpp"

#include <iosfwd>
#include <sstream>
#include <string>
#include <vector>

namespace Modelica {

namespace Causalize {

/**
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
  const CompactSet& domain() const;
  std::size_t arity() const;
  const Expression& access() const;

  void set_edge_id(int edge_id);
  void set_name(std::string name);
  void set_var_id(int var_id);
  void set_eq_id(int eq_id);
  void set_domain(CompactSet domain);
  void set_map1(CompactTransformation map1);
  void set_map2(CompactTransformation map2);
  void set_access(Expression access);

  std::string domainToSBGFormat() const;
  std::string map1ToSBGFormat() const;
  std::string map2ToSBGFormat() const;
  std::string toSBGFormat() const;

  AST::Integer maxDimPerimetral();

private:
  int _edge_id;
  std::string _name;
  int _var_id;
  int _eq_id;
  CompactSet _domain;
  CompactTransformation _map1;
  CompactTransformation _map2;
  Expression _access; ///< Expression of the access to the variable
};

std::ostream& operator<<(std::ostream& out, const SetEdge& sv);

} // namespace Causalize

} // namespace Modelica

#endif // MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_SET_EDGE_HPP_
