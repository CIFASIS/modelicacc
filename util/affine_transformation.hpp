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

#ifndef MODELICACC_UTIL_AFFINE_TRANSFORMATION_HPP_
#define MODELICACC_UTIL_AFFINE_TRANSFORMATION_HPP_ 

#include "ast/expression.hpp"
#include "util/affine_diag_transformation.hpp"

#include <variant>
#include <sstream>

namespace Modelica {

namespace Util {

enum class AffTransKind { Diagonal };

namespace detail {

class AffineTransformation {
public:
  /**
   * @brief Builds a zero filled matrix of dimension nxn, with a zero-filled
   * vector of dimension n. 
   */
  AffineTransformation(std::size_t n);

  /**
   * @brief Works as operator[] for the matrix of the transformation.
   */
  AST::Integer& matrix(std::size_t i, std::size_t j);
  const AST::Integer& matrix(std::size_t i, std::size_t j) const;

  /**
   * @brief Works as operator[] for the vector of the transformation.
   */
  AST::Integer& translation(std::size_t i);
  const AST::Integer& translation(std::size_t i) const;

  std::ostringstream toSBGFormat() const;

private:
  using AffTransfImpl = std::variant<AffineDiagTransformation>;

  AffTransfImpl _impl;
}; 

} // namespace detail

} // namespace Util

using CompactTransformation = Util::detail::AffineTransformation;

} // namespace Modelica

#endif // MODELICACC_UTIL_AFFINE_TRANSFORMATION_HPP_ 
