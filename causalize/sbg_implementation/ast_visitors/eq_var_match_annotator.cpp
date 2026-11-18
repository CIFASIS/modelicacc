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

#include "causalize/sbg_implementation/ast_visitors/eq_var_match_annotator.hpp"
#include "ast/modification.hpp"
#include "util/debug.hpp"

#include <sstream>
#include <string>

namespace Modelica {

namespace Causalize {

EqVarMatchAnnotator::EqVarMatchAnnotator(AST::ExpList var_list)
  : _var_list(var_list), _annotated_index(0) {}

std::ostringstream EqVarMatchAnnotator::operator()(AST::Connect eq)
{
  ERROR("EqVarMatchAnnotator: Connect not yet supported");
  return std::ostringstream{};
}

std::ostringstream EqVarMatchAnnotator::operator()(AST::Equality eq)
{
  std::ostringstream result;
  result << eq << " \"" << _var_list[_annotated_index] << "\";";
  return result;
}

std::ostringstream EqVarMatchAnnotator::operator()(AST::CallEq eq)
{
  ERROR("EqVarMatchAnnotator: trying to convert a CallEq");
  return std::ostringstream{};
}

std::ostringstream EqVarMatchAnnotator::operator()(AST::ForEq eq)
{
  std::ostringstream result;
  std::size_t elems_sz = eq.elements().size();
  std::size_t j = 1;
  result << "for " << eq.range() << " loop\n";
  BEGIN_BLOCK;
  for (const AST::Equation& jth_eq : eq.elements()) {
    result << INDENT << ApplyThis(jth_eq).str();
    if (j < elems_sz) {
      result << "\n";
    }
    ++_annotated_index;
    ++j;
  }
  END_BLOCK;
  result << "\nend for;";
  return result;
}

std::ostringstream EqVarMatchAnnotator::operator()(AST::IfEq eq)
{
  ERROR("EqVarMatchAnnotator: trying to convert an IfEq");
  return std::ostringstream{};
}

std::ostringstream EqVarMatchAnnotator::operator()(AST::WhenEq eq)
{
  ERROR("EqVarMatchAnnotator: trying to convert a WhenEq");
  return std::ostringstream{};
}

} // namespace Causalize

} // namespace Modelica
