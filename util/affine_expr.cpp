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

AffineExpr::AffineExpr(const std::vector<std::string>& order)
  : _order(order)
{
  for (const std::string& var_name : order) {
    _slopes[var_name] = 0;
  }
}

std::vector<AST::Integer> AffineExpr::slopes() const
{
  std::vector<AST::Integer> result; 

  for (const std::string& var_name : _order) {
    result.push_back(_slopes.at(var_name));
  }

  return result;
}

const AST::Integer& AffineExpr::offset() const { return _offset; }

std::size_t AffineExpr::arity() const { return _slopes.size(); }

AffineExpr AffineExpr::operator-() const
{
  AffineExpr result;

  for (const std::string& var_name : _order) {
    result._slopes[var_name] = -_slopes.at(var_name);
  }
  result._offset = -_offset;

  return result;
}

void AffineExpr::set_slope(std::string var_name, AST::Integer slope)
{
  _slopes[var_name] = slope;
}

void AffineExpr::set_offset(AST::Integer offset)
{
  _offset = offset;
}

AffineExpr AffineExpr::operator+(const AffineExpr& other) const
{
  ERROR_UNLESS(arity() == other.arity(), "AffineExpr::operator+: dimensions "
    , "don't match");

  AffineExpr result;

  for (const std::string& var_name : _order) {
    result._slopes[var_name] = _slopes.at(var_name)
      + other._slopes.at(var_name);
  }
  result._offset = _offset + other._offset;

  return result;
}

AffineExpr AffineExpr::operator-(const AffineExpr& other) const
{
  ERROR_UNLESS(arity() == other.arity(), "AffineExpr::operator-: dimensions "
    , "don't match");

  AffineExpr result;

  for (const std::string& var_name : _order) {
    result._slopes[var_name] = _slopes.at(var_name)
      - other._slopes.at(var_name);
  }
  result._offset = _offset - other._offset;

  return result;
}

AffineExpr AffineExpr::operator*(AST::Integer scalar) const
{
  AffineExpr result;

  for (const std::string& var_name : _order) {
    result._slopes[var_name] =  scalar*_slopes.at(var_name);
  }
  result._offset = scalar*_offset;

  return result;
}

AffineExpr AffineExpr::operator*(const AffineExpr& other) const
{
  ERROR_UNLESS(arity() == other.arity(), "AffineExpr::operator*: dimensions "
    , "don't match");

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
  for (const std::string& var_name :  _order) {
    if (_slopes.at(var_name) != 0) {
      return false;
    }
  }

  return true;
}

} // namespace Util

} // namespace Modelica
