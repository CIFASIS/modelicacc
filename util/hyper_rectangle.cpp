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

namespace Modelica {

namespace Util {

namespace detail {

// Constructors/destructors ----------------------------------------------------

HyperRectangle::HyperRectangle() {}

// Getters ---------------------------------------------------------------------

std::size_t HyperRectangle::arity() const { return _starts.size(); }

// Setters ---------------------------------------------------------------------

void HyperRectangle::addDimension(AST::Integer start, AST::Integer step
  , AST::Integer end)
{
  _starts.push_back(start);
  _steps.push_back(step);
  _ends.push_back(end);
}

// Operators -------------------------------------------------------------------

HyperRectangle HyperRectangle::translate(const Translation& t) const
{
  std::size_t arity = _starts.size();
  ERROR_UNLESS(t.arity() == arity, "HyperRectangle::translate: dimensions of "
    , "hyper-rectangle and translation are different");

  HyperRectangle result = *this;

  for (std::size_t k = 0; k < arity; ++k) {
    result._starts[k] += t[k];
    result._ends[k] += t[k];
  }

  return result;
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

void HyperRectangle::cartesianProduct(const HyperRectangle& other)
{
  _starts.insert(_starts.end(), other._starts.begin(), other._starts.end());
  _steps.insert(_steps.end(), other._steps.begin(), other._steps.end());
  _ends.insert(_ends.end(), other._ends.begin(), other._ends.end());
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

CompactSet indicesToCompactSet(const IndexList& indices
  , const VarSymbolTable& symbols)
{
  CompactSet result;

  for (const Index& index : indices) {
    OptExp expr = index.exp();
    if (!expr) {
      ERROR("indicesToCompactSet: empty index");
    } 
    else if (!is<Range>(expr.get())) {
      ERROR("indicesToCompactSet: only Range expressions supported");
    }

    // TODO: consider decreasing Interval
    EvalExpression eval_expr(symbols);
    Range expr_range = get<Range>(expr.get());
    Integer start = Integer(Apply(eval_expr, expr_range.start()));
    Integer step = 1;
    Integer end = Integer(Apply(eval_expr, expr_range.end()));
    if (expr_range.step()) {
      step = Integer(Apply(eval_expr, expr_range.step().get()));
    }

    result.addDimension(start, step, end);
  }

  //if (result.arity() < _max_dim) {
  //  // TODO: fill remaining dimensions
  //}

  return result;
}

} // namespace Modelica
