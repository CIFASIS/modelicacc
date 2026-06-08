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

#ifndef MODELICACC_UTIL_TRANSLATION_HPP_
#define MODELICACC_UTIL_TRANSLATION_HPP_ 

#include "ast/expression.hpp"

#include <sstream>
#include <vector>

namespace Modelica {

/**
 * @brief Encodes geometric translation, i.e. the vector of an affine
 * transformation.
 */
class Translation {
public:
  Translation() = default;
  /**
   * @brief Returns the identity translation of dimension n.
   */
  Translation(std::size_t n);

  /**
   * @brief Returns the identity translation of dimension n.
   */
  Translation(std::size_t n, AST::Integer value);

  std::size_t arity() const;

  AST::Integer& operator[](std::size_t i);
  const AST::Integer& operator[](std::size_t i) const;

  Translation operator-(const Translation& other) const;

private:
  std::size_t _dimension;
  std::vector<AST::Integer> _translation;
};

} // namespace Modelica

#endif // MODELICACC_UTIL_TRANSLATION_HPP_ 
