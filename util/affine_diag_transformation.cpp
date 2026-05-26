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

#include "util/affine_diag_transformation.hpp"
#include "util/debug.hpp"

#include <iostream>

namespace Modelica {

namespace Util {

namespace detail {

AffineDiagTransformation::AffineDiagTransformation(std::size_t n)
  : _dimension(n), _diag(n, 0), _translation(n, 0) {}

AST::Integer& AffineDiagTransformation::matrix(std::size_t i, std::size_t j)
{
  ERROR_UNLESS(i < _dimension && j < _dimension,
    "AffineDiagTransformation::matrix: ranges out of bounds");

  if (i == j) {
    return _diag[i];
  }

  ERROR("AffineDiagTransformation::matrix: only diagonal access\n");
}

const AST::Integer& AffineDiagTransformation::matrix(std::size_t i
  , std::size_t j) const
{
  ERROR_UNLESS(i < _dimension && j < _dimension,
    "AffineDiagTransformation::matrix: ranges out of bounds");

  if (i == j) {
    return _diag[i];
  }

  ERROR("AffineDiagTransformation::matrix: only diagonal access\n");
}

AST::Integer& AffineDiagTransformation::translation(std::size_t i)
{
  ERROR_UNLESS(i < _dimension,
    "AffineDiagTransformation::translation: range out of bounds");

  return _translation[i];
}

const AST::Integer& AffineDiagTransformation::translation(std::size_t i) const
{
  ERROR_UNLESS(i < _dimension,
    "AffineDiagTransformation::translation: range out of bounds");

  return _translation[i];
}

void AffineDiagTransformation::setRow(std::size_t i, const AffineExpr& expr)
{
  ERROR_UNLESS(_dimension == expr.arity() && i < _dimension
    , "AffineDiagTransformation::setRow: dimensions don't match");

  _diag[i] = expr.slopes()[i];
  _translation[i] = expr.offset();
}

std::string AffineDiagTransformation::toSBGFormat() const
{
  std::ostringstream out;

  out << "|";
  for (std::size_t k = 0; k < _dimension; ++k) {
    out << _diag[k] << "*x+" << _translation[k];
    if (k < _dimension - 1) {
      out << "|";
    }
  }
  out << "|";

  return out.str();
}

} // namespace detail

} // namespace Util

} // namespace Modelica
