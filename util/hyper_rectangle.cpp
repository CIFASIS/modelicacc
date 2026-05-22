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

#include "util/hyper_rectangle.hpp"
#include "util/ast_visitors/eval_expression.hpp"
#include "util/debug.hpp"

#include <boost/variant/get.hpp>

#include <iostream>
#include <utility>

namespace Modelica {

namespace Util {

namespace detail {

// Constructors/destructors ----------------------------------------------------

HyperRectangle::HyperRectangle() {}

HyperRectangle::HyperRectangle(AST::Integer start, AST::Integer step
  , AST::Integer end)
{
  _starts.push_back(start);
  _steps.push_back(step);
  _ends.push_back(end);
}

// Getters ---------------------------------------------------------------------

std::size_t HyperRectangle::arity() const { return _starts.size(); }

// Operators -------------------------------------------------------------------

void HyperRectangle::setUnion(const HyperRectangle& other)
{
  if (_starts.empty()) {
    _starts = other._starts;
    _steps = other._steps;
    _ends = other._ends;
    return;
  }

  if (other._starts.empty()) {
    return;
  }

  if (_starts == other._starts && _steps == other._steps
    && _ends == other._ends) {
    return;
  }

  ERROR("HyperRectangle::union: union of ", toSBGFormat(), " and "
    , other.toSBGFormat(), " not supported");
}

void HyperRectangle::cartesianProduct(const HyperRectangle& other)
{
  _starts.insert(_starts.end(), other._starts.begin(), other._starts.end());
  _steps.insert(_steps.end(), other._steps.begin(), other._steps.end());
  _ends.insert(_ends.end(), other._ends.begin(), other._ends.end());
}

std::string HyperRectangle::toSBGFormat() const
{
  std::ostringstream out;

  unsigned int arity = _starts.size();
  for (unsigned int k = 0; k < arity; ++k) {
    out << "[" << _starts[k] << ":" << _steps[k] << ":" << _ends[k] << "]";
    if (k + 1 < arity) {
      out << "x";
    }
  }

  return out.str();
}

// Methods ---------------------------------------------------------------------

void HyperRectangle::reflection()
{
  ERROR("HyperRectangle::reflection: not supported yet");
}

void HyperRectangle::translate(const Translation& t)
{
  std::size_t arity = _starts.size();
  ERROR_UNLESS(t.arity() == arity, "HyperRectangle::translate: dimensions of "
    , "hyper-rectangle and translation are different");

  for (std::size_t k = 0; k < arity; ++k) {
    _starts[k] += t[k];
    _ends[k] += t[k];
  }
}

void HyperRectangle::scale(AST::Integer factor)
{
  for (std::size_t k = 0; k < arity(); ++k) {
    _starts[k] *= factor;
    _steps[k] *= factor;
    _ends[k] *= factor;
  }
}

AST::Integer HyperRectangle::maxDimSize() const
{
  AST::Integer maximum = 0;

  unsigned int arity = _starts.size();
  for (unsigned int k = 0; k < arity; ++k) {
    maximum = std::max(maximum, (_ends[k] - _starts[k])/_steps[k]);
  }

  return maximum;
}

} // namespace detail

} // namespace Util

} // namespace Modelica
