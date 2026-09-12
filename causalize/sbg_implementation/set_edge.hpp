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

/**
 * @file
 * @brief Interface between SBG library and the Modelica mapping from equations
 * to variables.
 */

#ifndef MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_SET_EDGE_HPP_
#define MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_SET_EDGE_HPP_

#include "ast/expression.hpp"
#include "causalize/sbg_implementation/equation_info.hpp"

#include <sbg/expression.hpp>
#include <sbg/integer.hpp>
#include <sbg/set.hpp>

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
  SetEdge(int edge_id, SBG::LIB::Set domain);

  int edge_id() const;
  std::string name() const;
  int var_id() const;
  int eq_id() const;
  const SBG::LIB::IntTuple& translation() const;
  const SBG::LIB::Set& domain() const;
  const SBG::LIB::Expression& map1() const;
  const SBG::LIB::Expression& map2() const;
  const AST::Expression& access() const;
  std::size_t arity() const;

  void set_edge_id(int edge_id);
  void set_name(std::string name);
  void set_var_id(int var_id);
  void set_eq_id(int eq_id);
  void set_translation(SBG::LIB::IntTuple translation);
  void set_domain(SBG::LIB::Set domain);
  void set_map1(SBG::LIB::Expression map1);
  void set_map2(SBG::LIB::Expression map2);
  void set_access(AST::Expression access);

  std::string domainToSBGFormat() const;
  std::string map1ToSBGFormat() const;
  std::string map2ToSBGFormat() const;
  std::string toSBGFormat() const;

  SBG::LIB::Set translatedDomain() const;

  /**
   * @brief Returns the maximum coordinate of the perimeter of _domain.
   */
  AST::Integer maxDimPerimetral();

  /**
   * @brief Returns a new set-edge with the same values for each field, except
   * that _domain is intersected with the result of the translation of
   * \p new_domain with -_translation.
   */
  SetEdge restrict(SBG::LIB::Set new_domain) const;

private:
  int _edge_id;
  std::string _name;
  int _var_id;
  int _eq_id;
  SBG::LIB::IntTuple _translation; ///< Avoids domain values collision
  SBG::LIB::Set _domain;
  SBG::LIB::Expression _map1; ///< Map to equations nodes
  SBG::LIB::Expression _map2; ///< Map to variables nodes
  AST::Expression _access; ///< Expression of the access to the variable
};

std::ostream& operator<<(std::ostream& out, const SetEdge& sv);

using SetEdges = std::vector<SetEdge>;

} // namespace Causalize

} // namespace Modelica

#endif // MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_SET_EDGE_HPP_
