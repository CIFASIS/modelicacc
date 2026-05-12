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
#include "causalize/sbg_implementation/affine_transformation.hpp"
#include "causalize/sbg_implementation/hyper_rectangle.hpp"

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
  SetEdge(int edge_id);
  SetEdge(int edge_id, detail::HyperRectangle domain);

  int edge_id() const;
  std::size_t arity() const;
  std::string name() const;

  void set_edge_id(int edge_id);
  void set_name(std::string name);

  std::ostringstream domainToSBGFormat() const;

  /**
   * @brief Returns the maximum size between dimensions of the domain. That is,
   * if the array of equations was declared:
   * for i1 in range1, ..., ik in rangek loop it returns
   * max{rangei : 0 < i < k+1}.
   */
  AST::Integer maxDimSize();

private:
  int _edge_id;
  std::string _name;
  detail::HyperRectangle _domain;
  detail::AffineTransformation _map1;
  detail::AffineTransformation _map2;
};

std::ostream& operator<<(std::ostream& out, const SetEdge& sv);

} // namespace Causalize

} // namespace Modelica

#endif // MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_SET_EDGE_HPP_
