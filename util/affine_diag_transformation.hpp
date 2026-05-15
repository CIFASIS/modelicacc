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

#ifndef MODELICACC_UTIL_AFFINE_DIAG_TRANSFORMATION_HPP_
#define MODELICACC_UTIL_AFFINE_DIAG_TRANSFORMATION_HPP_ 

#include "ast/expression.hpp"
#include "util/affine_expr.hpp"
#include "util/translation.hpp"

#include <sstream>
#include <vector>

namespace Modelica {

namespace Util {

namespace detail {

class AffineDiagTransformation {
public:
  AffineDiagTransformation(std::size_t n);

  AST::Integer& matrix(std::size_t i, std::size_t j);
  const AST::Integer& matrix(std::size_t i, std::size_t j) const;

  AST::Integer& translation(std::size_t i);
  const AST::Integer& translation(std::size_t i) const;

  void setRow(std::size_t i, const AffineExpr& expr);

  std::ostringstream toSBGFormat() const;

private:
  std::size_t _dimension;
  std::vector<AST::Integer> _diag;
  Translation _translation;
};

} // namespace detail

} // namespace Util

} // namespace Modelica

#endif // MODELICACC_UTIL_AFFINE_DIAG_TRANSFORMATION_HPP_ 
