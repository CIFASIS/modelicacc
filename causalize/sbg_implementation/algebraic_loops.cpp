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

#include "causalize/sbg_implementation/algebraic_loops.hpp"
#include "util/debug.hpp"

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// ModelicaCC algebraic loop ---------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

AlgebraicLoop::AlgebraicLoop(AST::EquationList equations)
  : _equations(equations) {}

const AST::EquationList& AlgebraicLoop::equations() const { return _equations; }

bool AlgebraicLoop::isEmpty() const { return _equations.empty(); }

void AlgebraicLoop::pushBack(AST::Equation equation)
{
  _equations.push_back(equation);
}

void AlgebraicLoop::concatenation(AlgebraicLoop other)
{
  _equations.insert(
    _equations.end(), other._equations.begin(), other._equations.end()
  );
}

std::ostream& operator<<(std::ostream& out, const AlgebraicLoop& loop)
{
  for (const AST::Equation& eq : loop.equations()) {
    out << eq << "\n";
  }

  return out;
}

////////////////////////////////////////////////////////////////////////////////
// ModelicaCC algebraic loops --------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

std::size_t AlgebraicLoops::size() const { return _loops.size(); }

AlgebraicLoop AlgebraicLoops::operator[](std::size_t k) const
{
  ERROR_UNLESS(k < _loops.size(), "AlgebraicLoops::operator[]: index ", k
    , " out of range");
  return _loops[k];
}

void AlgebraicLoops::pushBack(AlgebraicLoop loop)
{
  if (!loop.isEmpty()) {
    _loops.push_back(loop);
  }
}

void AlgebraicLoops::reverse() { std::reverse(_loops.begin(), _loops.end()); }

void AlgebraicLoops::concatenation(AlgebraicLoops other)
{
  _loops.insert(_loops.end(), other.begin(), other.end());
}

std::ostream& operator<<(std::ostream& out, const AlgebraicLoops& loops)
{
  for (const AlgebraicLoop& loop : loops) {
    out << loop << "\n";
  }

  return out;
}

}  // namespace Causalize

}  // namespace Modelica
