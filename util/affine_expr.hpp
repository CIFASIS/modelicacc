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

#ifndef MODELICACC_UTIL_AFFINE_EXPR_HPP_
#define MODELICACC_UTIL_AFFINE_EXPR_HPP_ 

#include "ast/expression.hpp"

#include <string>
#include <unordered_map>

namespace Modelica {

namespace Util {

/**
 * @brief Represents and affine expression involving an ordered collection of
 * of variables.
 */
class AffineExpr {
public:
  AffineExpr() = default;
  AffineExpr(const std::vector<std::string>& variables);

  std::vector<AST::Integer> slopes() const;
  const AST::Integer& offset() const;

  std::size_t arity() const;

  void set_slope(std::string var_name, AST::Integer slope);
  void set_offset(AST::Integer offset);

  AffineExpr operator-() const;
  AffineExpr operator+(const AffineExpr& other) const;
  AffineExpr operator-(const AffineExpr& other) const;
  AffineExpr operator*(AST::Integer scalar) const;
  AffineExpr operator*(const AffineExpr& other) const;

  bool isConstant() const;

private:
  std::vector<std::string> _order;
  std::unordered_map<std::string, AST::Integer> _slopes;
  AST::Integer _offset;
};

} // namespace Util

} // namespace Modelica

#endif // MODELICACC_UTIL_AFFINE_EXPR_HPP_ 
