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

#ifndef MODELICACC_UTIL_HYPER_RECTANGLE_HPP_
#define MODELICACC_UTIL_HYPER_RECTANGLE_HPP_

#include "ast/expression.hpp"
#include "util/affine_transformation.hpp"
#include "util/table.hpp"
#include "util/translation.hpp"

#include <sstream>
#include <vector>

namespace Modelica {

namespace Util {

namespace detail {

class HyperRectangle {
public:
  HyperRectangle();
  HyperRectangle(AST::Integer start, AST::Integer step, AST::Integer end);

  std::size_t arity() const;

  /**
   * @brief Returns a string-like result that is parsable by the SBG parser.
   */
  std::string toSBGFormat() const;

  HyperRectangle translate(const Translation& t) const;
  void cartesianProduct(const HyperRectangle& other);

  AST::Integer maxDimSize() const;

private:
  std::vector<AST::Integer> _starts;
  std::vector<AST::Integer> _steps;
  std::vector<AST::Integer> _ends;
};

} // namespace detail

} // namespace Util

using CompactSet = Util::detail::HyperRectangle;

} // namespace Modelica

#endif // MODELICACC_UTIL_HYPER_RECTANGLE_HPP_
