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
 * @brief Visitor to get an EquationInfo structure from a flattened Modelica
 * equation.
 */

#ifndef MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_AST_VISITORS_EQUATION_INFO_VISITOR_HPP_
#define MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_AST_VISITORS_EQUATION_INFO_VISITOR_HPP_

#include "ast/equation.hpp"
#include "util/sbg/equation_info.hpp"

#include <boost/variant/static_visitor.hpp>

namespace Modelica {

namespace Causalize {

/**
 * @brief Creates the EquationInfo associated to a flattened Modelica equation.
 */
class EqInfoVisitor : public boost::static_visitor<EquationInfo> {
public:
  EqInfoVisitor(unsigned int max_dim);

  EquationInfo operator()(const AST::Connect& eq) const;
  EquationInfo operator()(const AST::Equality& eq) const;
  EquationInfo operator()(const AST::CallEq& eq) const;
  EquationInfo operator()(const AST::ForEq& eq) const;
  EquationInfo operator()(const AST::IfEq& eq) const;
  EquationInfo operator()(const AST::WhenEq& eq) const;

private:
  unsigned int _max_dim;
};

} // namespace Causalize

}  // namespace Modelica

#endif // MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_AST_VISITORS_EQUATION_INFO_VISITOR_HPP_
