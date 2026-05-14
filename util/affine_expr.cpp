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

#include "util/affine_expr.hpp"
#include "util/debug.hpp"

namespace Modelica {

namespace Util {

AffineExpr::AffineExpr(AST::Integer v) : _offset(v) {}

AffineExpr::AffineExpr(std::string var_name)
{
  _slopes[var_name] = 1;
}

AffineExpr AffineExpr::operator-() const
{
  AffineExpr result;

  for (const auto& [var_name, slope] : _slopes) {
    result._slopes[var_name] = -slope;
  }
  result._offset = -_offset;

  return result;
}

AffineExpr AffineExpr::operator+(const AffineExpr& other) const
{
  AffineExpr result;

  for (const auto& [var_name, slope] : _slopes) {
    result._slopes[var_name] =  slope + other._slopes.at(var_name);
  }
  result._offset = _offset + other._offset;

  return result;
}

AffineExpr AffineExpr::operator-(const AffineExpr& other) const
{
  AffineExpr result;

  for (const auto& [var_name, slope] : _slopes) {
    result._slopes[var_name] =  slope - other._slopes.at(var_name);
  }
  result._offset = _offset - other._offset;

  return result;
}

AffineExpr AffineExpr::operator*(AST::Integer scalar) const
{
  AffineExpr result;

  for (const auto& [var_name, slope] : _slopes) {
    result._slopes[var_name] =  scalar*slope;
  }
  result._offset = scalar*_offset;

  return result;
}

AffineExpr AffineExpr::operator*(const AffineExpr& other) const
{
  if (isConstant()) {
    return other * this->_offset;
  } else if (other.isConstant()) {
    return *this * other._offset; 
  } else {
    ERROR("AffineExpr::operator*: result is not an AffineExpr");
  }

  return AffineExpr{};
}

bool AffineExpr::isConstant() const
{
  for (const auto& [var_name, slope] :  _slopes) {
    if (slope != 0) {
      return false;
    }
  }

  return true;
}

} // namespace Util

} // namespace Modelica
