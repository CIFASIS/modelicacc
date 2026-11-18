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

/**
 * @file
 * @brief Handles the conversion of a Modelica variable access to a SBG
 * expression.
 */

#ifndef MODELICACC_UTIL_SBG_AST_VISITORS_ACCESS_TO_SBG_EXPR_HPP_
#define MODELICACC_UTIL_SBG_AST_VISITORS_ACCESS_TO_SBG_EXPR_HPP_

#include "ast/expression.hpp"
#include "util/table.hpp"

#include <boost/variant/static_visitor.hpp>
#include <sbgraph/sbg/expression.hpp>

namespace Modelica {

using Counters = std::vector<std::string>;

/**
 * @brief Converts a Modelica access to a SBG expression. For example, for a[i]
 * it will return 1*i+0.
 */
class AccessToSBGExpr
  : public boost::static_visitor<SBG::LIB::Expression> {
public:
  AccessToSBGExpr(const VarSymbolTable& symbols, const Counters& counters);

  SBG::LIB::Expression operator()(AST::Integer v) const;
  SBG::LIB::Expression operator()(AST::Boolean v) const;
  SBG::LIB::Expression operator()(AST::AddAll v) const;
  SBG::LIB::Expression operator()(AST::String v) const;
  SBG::LIB::Expression operator()(AST::Name v) const;
  SBG::LIB::Expression operator()(AST::Real v) const;
  SBG::LIB::Expression operator()(AST::SubEnd v) const;
  SBG::LIB::Expression operator()(AST::SubAll v) const;
  SBG::LIB::Expression operator()(AST::BinOp) const;
  SBG::LIB::Expression operator()(AST::UnaryOp) const;
  SBG::LIB::Expression operator()(AST::Brace) const;
  SBG::LIB::Expression operator()(AST::Bracket) const;
  SBG::LIB::Expression operator()(AST::Call) const;
  SBG::LIB::Expression operator()(AST::FunctionExp) const;
  SBG::LIB::Expression operator()(AST::ForExp) const;
  SBG::LIB::Expression operator()(AST::IfExp) const;
  SBG::LIB::Expression operator()(AST::Named) const;
  SBG::LIB::Expression operator()(AST::Output) const;
  SBG::LIB::Expression operator()(AST::Reference) const;
  SBG::LIB::Expression operator()(AST::Range) const;

private:
  unsigned int _max_dim;
  const VarSymbolTable& _symbols;
  const Counters& _counters;
};

}  // namespace Modelica

#endif // MODELICACC_UTIL_SBG_AST_VISITORS_ACCESS_TO_SBG_EXPR_HPP_
