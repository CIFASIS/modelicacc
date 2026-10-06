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

#include "util/sbg/ast_visitors/equation_info_visitor.hpp"
#include "ast/expression.hpp"

namespace {

/**
 * @brief Fills the remaining dimensions with dummies counters.
 */
Modelica::AST::IndexList fill(
  unsigned int max_dim, Modelica::AST::IndexList index_list
)
{
  Modelica::AST::IndexList result = index_list;
  for (std::size_t k = index_list.size(); k < max_dim; ++k) {
    result.emplace_back(
      "*dummy_" + std::to_string(k), Modelica::AST::Expression{0}
    );
  }
  return result;
}

} // namespace

namespace Modelica {

EqInfoVisitor::EqInfoVisitor(unsigned int max_dim) : _max_dim(max_dim) {}

EquationInfo EqInfoVisitor::operator()(const AST::Connect& eq) const
{
  return EquationInfo{AST::Indexes{fill(_max_dim, AST::IndexList{})}, eq, true};
}

EquationInfo EqInfoVisitor::operator()(const AST::Equality& eq) const
{
  return EquationInfo{AST::Indexes{fill(_max_dim, AST::IndexList{})}, eq, true};
}

EquationInfo EqInfoVisitor::operator()(const AST::CallEq& eq) const
{
  return EquationInfo{AST::Indexes{fill(_max_dim, AST::IndexList{})}, eq, true};
}

EquationInfo EqInfoVisitor::operator()(const AST::ForEq& eq) const
{
  return EquationInfo{
    fill(_max_dim, eq.range().indexes()), eq.elements().front(), false
  };
}

EquationInfo EqInfoVisitor::operator()(const AST::IfEq& eq) const
{
  return EquationInfo{AST::Indexes{fill(_max_dim, AST::IndexList{})}, eq, true};
}

EquationInfo EqInfoVisitor::operator()(const AST::WhenEq& eq) const
{
  return EquationInfo{AST::Indexes{fill(_max_dim, AST::IndexList{})}, eq, true};
}

} // namespace Modelica
