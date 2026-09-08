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

#ifndef MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_AST_VISITORS_VARIABLE_RENAMER_HPP_
#define MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_AST_VISITORS_VARIABLE_RENAMER_HPP_

/**
 * @file
 * @brief Visitor that given a reference to a variable, it adds to it a prefix.
 *
 * It will be used in the vertical sorting SBG builder to create guess and
 * residual variables.
 */

#include <ast/expression.hpp>
#include <util/table.hpp>

#include <boost/variant/static_visitor.hpp>

#include <string>

namespace Modelica {

namespace Causalize {

class VariableRenamer : public boost::static_visitor<AST::Expression> {
public:
  VariableRenamer(std::string prefix);

  AST::Expression operator()(AST::Integer v) const;
  AST::Expression operator()(AST::Boolean v) const;
  AST::Expression operator()(AST::String v) const;
  AST::Expression operator()(AST::AddAll v) const;
  AST::Expression operator()(AST::Name v) const;
  AST::Expression operator()(AST::Real v) const;
  AST::Expression operator()(AST::SubEnd v) const;
  AST::Expression operator()(AST::SubAll v) const;
  AST::Expression operator()(AST::BinOp) const;
  AST::Expression operator()(AST::UnaryOp) const;
  AST::Expression operator()(AST::Brace) const;
  AST::Expression operator()(AST::Bracket) const;
  AST::Expression operator()(AST::Call) const;
  AST::Expression operator()(AST::FunctionExp) const;
  AST::Expression operator()(AST::ForExp) const;
  AST::Expression operator()(AST::IfExp) const;
  AST::Expression operator()(AST::Named) const;
  AST::Expression operator()(AST::Output) const;
  AST::Expression operator()(AST::Reference) const;
  AST::Expression operator()(AST::Range) const;

private:
  std::string _prefix; ///< Prefix added to create new variable.
};

} // namespace Causalize

} // namespace Modelica

#endif // MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_AST_VISITORS_VARIABLE_RENAMER_HPP_
