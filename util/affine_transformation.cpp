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

#include "util/affine_transformation.hpp"

namespace Modelica {

namespace Util {

namespace detail {

AffineTransformation::AffineTransformation(std::size_t n) : _impl(n)
{
  AffTransKind kind{AffTransKind::Diagonal};
  switch (kind) {
    case AffTransKind::Diagonal: {
      _impl = AffineDiagTransformation{n};
      break;
    }

    default: {
      break;
    }
  }
}

AST::Integer& AffineTransformation::matrix(std::size_t i, std::size_t j)
{
  return std::visit([i, j](auto& a) -> AST::Integer& { return a.matrix(i, j); }
    , _impl);
}

const AST::Integer& AffineTransformation::matrix(std::size_t i, std::size_t j)
  const
{
  return std::visit([i, j](const auto& a) -> const AST::Integer&
  {
    return a.matrix(i, j);
  }, _impl);
}

AST::Integer& AffineTransformation::translation(std::size_t i)
{
  return std::visit([i](auto& a) -> AST::Integer& { return a.translation(i); }
    , _impl);
}

const AST::Integer& AffineTransformation::translation(std::size_t i) const
{
  return std::visit([i](const auto& a) -> const AST::Integer&
  {
    return a.translation(i);
  }, _impl);
}

void AffineTransformation::setRow(std::size_t i, const AffineExpr& expr)
{
  return std::visit([&](auto& a) { return a.setRow(i, expr); }, _impl);
}

std::string AffineTransformation::toSBGFormat() const
{
  return std::visit([](const auto& a) { return a.toSBGFormat(); }, _impl);
}

} // namespace detail

} // namespace Util

} // namespace Modelica
