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
 * @brief Module that converts an AST::Expression to a SBG expression.
 */

#ifndef MODELICACC_UTIL_AST_VISITORS_SBG_EXPR_VISITOR_HPP_
#define MODELICACC_UTIL_AST_VISITORS_SBG_EXPR_VISITOR_HPP_

#include "ast/expression.hpp"
#include "util/table.hpp"

#include <boost/variant/static_visitor.hpp>
#include <sbg/expression.hpp>

namespace Modelica {

/**
 * @brief Returns (if possible) the associated SBG expression to a Modelica
 * expression.
 */
class ExprVisitor
  : public boost::static_visitor<SBG::LIB::Expression> {
public:
  /**
   * @brief The second argument is present to keep track of the order of
   * counters. For example, "i" is the counter for the first dimensions,
   * "j" for the second one and so on.
   */
  explicit ExprVisitor(VarSymbolTable symbols
    , std::vector<std::string> order);
  SBG::LIB::Expression operator()(Integer v) const;
  SBG::LIB::Expression operator()(Boolean v) const;
  SBG::LIB::Expression operator()(AddAll v) const;
  SBG::LIB::Expression operator()(String v) const;
  SBG::LIB::Expression operator()(Name v) const;
  SBG::LIB::Expression operator()(Real v) const;
  SBG::LIB::Expression operator()(SubEnd v) const;
  SBG::LIB::Expression operator()(SubAll v) const;
  SBG::LIB::Expression operator()(BinOp) const;
  SBG::LIB::Expression operator()(UnaryOp) const;
  SBG::LIB::Expression operator()(Brace) const;
  SBG::LIB::Expression operator()(Bracket) const;
  SBG::LIB::Expression operator()(Call) const;
  SBG::LIB::Expression operator()(FunctionExp) const;
  SBG::LIB::Expression operator()(ForExp) const;
  SBG::LIB::Expression operator()(IfExp) const;
  SBG::LIB::Expression operator()(Named) const;
  SBG::LIB::Expression operator()(Output) const;
  SBG::LIB::Expression operator()(Reference) const;
  SBG::LIB::Expression operator()(Range) const;

protected:
  VarSymbolTable _symbols;
  std::vector<std::string> _order;
};

}  // namespace Modelica

#endif // MODELICACC_UTIL_AST_VISITORS_SBG_EXPR_VISITOR_HPP_
