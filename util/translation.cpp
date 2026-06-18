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

#include "util/translation.hpp"
#include "util/debug.hpp"

#include <iostream>

namespace Modelica {

Translation::Translation() : _dimension(0), _translation() {}

Translation::Translation(std::size_t n) : _dimension(n), _translation(n, 0) {}

Translation::Translation(std::size_t n, AST::Integer value)
  : _dimension(n), _translation(n, value) {}

std::size_t Translation::arity() const { return _dimension; }

AST::Integer& Translation::operator[](std::size_t i)
{
  ERROR_UNLESS(i < _dimension, "Translation::operator[]: index ", i
    , "out of dimension ", _dimension);
  return _translation[i];
}

const AST::Integer& Translation::operator[](std::size_t i) const
{
  ERROR_UNLESS(i < _dimension, "Translation::operator[]: index ", i
    , "out of dimension ", _dimension);
  return _translation[i];
}

Translation Translation::operator-(const Translation& other) const
{
  ERROR_UNLESS(arity() == other.arity(), "Translation::operator-: dimensions "
    , "are mismatched");

  Translation result{arity()};
  result._dimension = arity();
  for (std::size_t k = 0; k < arity(); ++k) {
    result._translation[k] = operator[](k) - other[k];
  }
  return result;
}

} // namespace Modelica
