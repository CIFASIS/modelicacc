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
 * @brief Flattens arrays of equations. That is, it returns a list of equations
 * that doesn't nest loops, and where each loop only contains a single scalar
 * equation inside.
 */

#ifndef MODELICACC_UTIL_AST_VISITORS_FLATTER_FOR_HPP_ 
#define MODELICACC_UTIL_AST_VISITORS_FLATTER_FOR_HPP_ 

#include "ast/equation.hpp"
#include "ast/expression.hpp"

#include <boost/variant/static_visitor.hpp>

namespace Modelica {

class FlatterForVisitor : public boost::static_visitor<AST::EquationList> {
public:
  FlatterForVisitor() = default;

  AST::EquationList operator()(AST::Connect eq);
  AST::EquationList operator()(AST::Equality eq);
  AST::EquationList operator()(AST::CallEq eq);
  AST::EquationList operator()(AST::IfEq eq);
  AST::EquationList operator()(AST::WhenEq eq);
  AST::EquationList operator()(AST::ForEq eq);

private:
  AST::EquationList bounds(AST::Equation eq);

  AST::IndexList _bounds;
};

}  // namespace Modelica

#endif // MODELICACC_UTIL_AST_VISITORS_FLATTER_FOR_HPP_
