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

#ifndef MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_HYPER_RECTANGLE_HPP_
#define MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_HYPER_RECTANGLE_HPP_

#include "ast/expression.hpp"
#include "causalize/sbg_implementation/affine_transformation.hpp"

#include <sstream>
#include <vector>

namespace Modelica {

namespace Causalize {

namespace detail {

class HyperRectangle {
public:
  HyperRectangle();

  std::size_t arity() const;

  void addDimension(AST::Integer start, AST::Integer step, AST::Integer end);
  /**
   * @brief Returns a string-like result that is parsable by the SBG parser.
   */
  std::ostringstream toSBGFormat() const;

  void offset(AST::Integer offset);

  void cartesianProduct(const HyperRectangle& other);

  AST::Integer maxDimSize() const;

private:
  std::vector<AST::Integer> _starts;
  std::vector<AST::Integer> _steps;
  std::vector<AST::Integer> _ends;
};

std::ostringstream& operator<<(std::ostringstream& out
  , const HyperRectangle& rect);

/**
 * @brief Calculates the required affine diagonal transformation to obtain the
 * hyper-rectangle \p to from the hyper-rectangle \p from.
 */
//AffineTransformation transform(HyperRectangle from, HyperRectangle to);

} // namespace detail

} // namespace Causalize

} // namespace Modelica

#endif // MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_HYPER_RECTANGLE_HPP_
