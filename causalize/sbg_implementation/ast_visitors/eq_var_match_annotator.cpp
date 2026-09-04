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

AST::Equation EqVarMatchAnnotator::operator()(AST::Connect eq)
{
  ERROR("EqVarMatchAnnotator: Connect not yet supported");
  return AST::Equation{};
}

AST::Equation EqVarMatchAnnotator::operator()(AST::Equality eq)
{
  AST::Equality result = eq;
  std::ostringstream strm;
  strm << _var_list[_annotated_index];
  result.set_comment(AST::Comment{
    AST::StringComment{AST::StringList{strm.str()}}
  });
  return result;
}

AST::Equation EqVarMatchAnnotator::operator()(AST::CallEq eq)
{
  ERROR("EqVarMatchAnnotator: trying to convert a CallEq");
  return AST::Equation{};
}

AST::Equation EqVarMatchAnnotator::operator()(AST::ForEq eq)
{
  AST::EquationList eq_list;
  for (const AST::Equation& jth_eq : eq.elements()) {
    eq_list.push_back(ApplyThis(jth_eq));
    ++_annotated_index;
  }
  return AST::ForEq{eq.range(), eq_list};
}

AST::Equation EqVarMatchAnnotator::operator()(AST::IfEq eq)
{
  ERROR("EqVarMatchAnnotator: trying to convert an IfEq");
  return AST::Equation{};
}

AST::Equation EqVarMatchAnnotator::operator()(AST::WhenEq eq)
{
  ERROR("EqVarMatchAnnotator: trying to convert a WhenEq");
  return AST::Equation{};
}

} // namespace Causalize

} // namespace Modelica
