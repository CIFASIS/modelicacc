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

#include "causalize/sbg_implementation/causal_model.hpp"
#include "util/debug.hpp"

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// ModelicaCC group of causalized equations ------------------------------------
////////////////////////////////////////////////////////////////////////////////

SortedAlgebraicLoop::SortedAlgebraicLoop(EqVarMatchs matchs)
  : _matchs(matchs) {}

const EqVarMatchs& SortedAlgebraicLoop::matchs() const
{
  return _matchs;
}

bool SortedAlgebraicLoop::empty() const { return _matchs.empty(); }

void SortedAlgebraicLoop::pushBack(EqVarMatch match)
{
  _matchs.push_back(match);
}

std::ostream& operator<<(std::ostream& out, const SortedAlgebraicLoop& loop)
{
  for (const EqVarMatch& var : loop.matchs()) {
    out << var << "\n";
  }

  return out;
}

////////////////////////////////////////////////////////////////////////////////
// ModelicaCC causalized model -------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

std::size_t CausalModel::size() const { return _loops.size(); }

SortedAlgebraicLoop CausalModel::operator[](std::size_t k) const
{
  ERROR_UNLESS(k < _loops.size(), "CausalModel::operator[]: index "
    , k, " out of range");
  return _loops[k];
}

void CausalModel::pushBack(SortedAlgebraicLoop loop)
{
  if (!loop.empty()) {
    _loops.push_back(loop);
  }
}

std::ostream& operator<<(std::ostream& out, const CausalModel& causal_model)
{
  for (std::size_t j = 0; j < causal_model.size(); ++j) {
    out << causal_model[j] << "\n";
  }

  return out;
}

}  // namespace Causalize

}  // namespace Modelica
