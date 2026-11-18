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

#include "util/ast_visitors/matching_exps_equation.hpp"
#include "util/debug.hpp"

namespace Modelica {

MatchingExpsEquation::MatchingExpsEquation(AST::Name var_name, bool state_var)
  : _matching_exprs(var_name, state_var) {}

const MatchingExps& MatchingExpsEquation::matching_exprs() const
{
  return _matching_exprs;
}

bool MatchingExpsEquation::operator()(AST::Connect eq)
{
  bool left = Apply(_matching_exprs, eq.left());
  bool right = Apply(_matching_exprs, eq.right());
  return left || right;
}

bool MatchingExpsEquation::operator()(AST::Equality eq)
{
  bool left = Apply(_matching_exprs, eq.left());
  bool right = Apply(_matching_exprs, eq.right());
  return left || right;
}

bool MatchingExpsEquation::operator()(AST::CallEq eq)
{
  ERROR("MatchingExpsEquation: CallEq not implemented");
  return false;
}

bool MatchingExpsEquation::operator()(AST::IfEq eq)
{
  ERROR("MatchingExpsEquation: IfEq not implemented");
  return false;
}

bool MatchingExpsEquation::operator()(AST::WhenEq eq)
{
  ERROR("MatchingExpsEquation: WhenEq not implemented");
  return false;
}

bool MatchingExpsEquation::operator()(AST::ForEq eq)
{
  bool result = false;
  for (const AST::Equation& jth_eq : eq.elements()) {
    bool jth_result = ApplyThis(jth_eq);
    result = result || jth_result;
  }
  return result;
}

}  // namespace Modelica
