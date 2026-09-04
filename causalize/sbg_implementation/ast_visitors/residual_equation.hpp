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

#ifndef MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_AST_VISITORS_RESIDUAL_EQUATION_HPP_
#define MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_AST_VISITORS_RESIDUAL_EQUATION_HPP_

#include "ast/equation.hpp"
#include "ast/expression.hpp"
#include "util/table.hpp"

#include <boost/variant/static_visitor.hpp>

namespace Modelica {

namespace Causalize {

/**
 * @brief Given the original equation, it constructs the modified equation that
 * uses the residual of the original variable. 
 */
class ResidualEqVisitor : public boost::static_visitor<AST::Equation> {
public:
  ResidualEqVisitor(AST::Expression expr);
  AST::Equation operator()(AST::Connect eq);
  AST::Equation operator()(AST::Equality eq);
  AST::Equation operator()(AST::CallEq eq);
  AST::Equation operator()(AST::ForEq eq);
  AST::Equation operator()(AST::IfEq eq);
  AST::Equation operator()(AST::WhenEq eq);

private:
  AST::Expression _original_expr;
};

} // namespace Causalize

}  // namespace Modelica

#endif // MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_AST_VISITORS_RESIDUAL_EQUATION_HPP_
