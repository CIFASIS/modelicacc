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

#include "causalize/sbg_implementation/ast_visitors/variable_renamer.hpp"
#include "util/debug.hpp"

#include <boost/variant/get.hpp>

namespace Modelica {

namespace Causalize {

VariableRenamer::VariableRenamer(std::string prefix) : _prefix(prefix) {}

AST::Expression VariableRenamer::operator()(AST::Integer v) const
{
  ERROR("VariableRenamer: trying to rename an Integer");
  return 0;
}

AST::Expression VariableRenamer::operator()(AST::Boolean v) const
{
  ERROR("VariableRenamer: trying to rename a Boolean");
  return 0;
}

AST::Expression VariableRenamer::operator()(AST::AddAll v) const
{
  ERROR("VariableRenamer: trying to rename a AddAll");
  return 0;
}

AST::Expression VariableRenamer::operator()(AST::String v) const
{
  ERROR("VariableRenamer: trying to rename a String");
  return 0;
}

AST::Expression VariableRenamer::operator()(AST::Name v) const
{
  ERROR("VariableRenamer: trying to rename a Name");
  return 0;
}

AST::Expression VariableRenamer::operator()(AST::Real v) const
{
  ERROR("VariableRenamer: trying to rename a Real");
  return 0;
}

AST::Expression VariableRenamer::operator()(AST::SubAll v) const
{
  ERROR("VariableRenamer: trying to rename a SubAll");
  return 0;
}

AST::Expression VariableRenamer::operator()(AST::SubEnd v) const
{
  ERROR("VariableRenamer: trying to rename a SubEnd");
  return 0;
}

AST::Expression VariableRenamer::operator()(AST::BinOp v) const
{
  ERROR("VariableRenamer: trying to rename a BinOp");
  return 0;
}

AST::Expression VariableRenamer::operator()(AST::UnaryOp v) const
{
  ERROR("VariableRenamer: trying to rename a UnaryOp");
  return 0;
}

AST::Expression VariableRenamer::operator()(AST::IfExp v) const
{
  ERROR("VariableRenamer: trying to rename a IfExp");
  return 0;
}

AST::Expression VariableRenamer::operator()(AST::Range v) const
{
  ERROR("VariableRenamer: trying to rename a Range");
  return 0;
}

AST::Expression VariableRenamer::operator()(AST::Brace v) const
{
  ERROR("VariableRenamer: trying to rename a Brace");
  return 0;
}

AST::Expression VariableRenamer::operator()(AST::Bracket v) const
{
  ERROR("VariableRenamer: trying to rename a Bracket");
  return 0;
}

AST::Expression VariableRenamer::operator()(AST::Call v) const
{
  ERROR("VariableRenamer: trying to rename a Call");
  return 0;
}

AST::Expression VariableRenamer::operator()(AST::FunctionExp v) const
{
  ERROR("VariableRenamer: trying to rename a FunctionExp");
  return 0;
}

AST::Expression VariableRenamer::operator()(AST::ForExp v) const
{
  ERROR("VariableRenamer: trying to rename a ForExp");
  return 0;
}

AST::Expression VariableRenamer::operator()(AST::Named v) const
{
  ERROR("VariableRenamer: trying to rename a Named");
  return 0;
}

AST::Expression VariableRenamer::operator()(AST::Output v) const
{
  ERROR("VariableRenamer: trying to rename a Output");
  return 0;
}

AST::Expression VariableRenamer::operator()(Reference v) const
{
  AST::Ref ref = v.ref();
  ERROR_UNLESS(ref.size() == 1, "VariableRenamer: conversion of dotted "
    , "references not implemented");
  AST::Name v_name = get<0>(ref[0]);
  AST::RefTuple prefix_tuple{_prefix + v_name, get<1>(ref[0])};
  return AST::Reference{AST::Ref{prefix_tuple}};
}

} // namespace Causalize

} // namespace Modelica
