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

#ifndef MODELICACC_UTIL_AST_VISITORS_AFFINE_EXPR_HPP_
#define MODELICACC_UTIL_AST_VISITORS_AFFINE_EXPR_HPP_

#include "ast/expression.hpp"
#include "util/affine_expr.hpp"
#include "util/table.hpp"

#include <boost/variant/static_visitor.hpp>

namespace Modelica {

/**
 * @brief Returns (if possible) the associated affine expression with the input
 * AST::Expression. 
 */
class AffineExprVisitor
  : public boost::static_visitor<Util::AffineExpr> {
public:
  /**
   * @brief The second argument is present to keep track of the order of
   * counters. For example, "i" is the counter for the first dimensions,
   * "j" for the second one and so on.
   */
  explicit AffineExprVisitor(VarSymbolTable symbols
    , std::vector<std::string> order);
  Util::AffineExpr operator()(Integer v) const;
  Util::AffineExpr operator()(Boolean v) const;
  Util::AffineExpr operator()(AddAll v) const;
  Util::AffineExpr operator()(String v) const;
  Util::AffineExpr operator()(Name v) const;
  Util::AffineExpr operator()(Real v) const;
  Util::AffineExpr operator()(SubEnd v) const;
  Util::AffineExpr operator()(SubAll v) const;
  Util::AffineExpr operator()(BinOp) const;
  Util::AffineExpr operator()(UnaryOp) const;
  Util::AffineExpr operator()(Brace) const;
  Util::AffineExpr operator()(Bracket) const;
  Util::AffineExpr operator()(Call) const;
  Util::AffineExpr operator()(FunctionExp) const;
  Util::AffineExpr operator()(ForExp) const;
  Util::AffineExpr operator()(IfExp) const;
  Util::AffineExpr operator()(Named) const;
  Util::AffineExpr operator()(Output) const;
  Util::AffineExpr operator()(Reference) const;
  Util::AffineExpr operator()(Range) const;

protected:
  VarSymbolTable _symbols;
  std::vector<std::string> _order;
};

}  // namespace Modelica

#endif // MODELICACC_UTIL_AST_VISITORS_AFFINE_EXPR_HPP_
