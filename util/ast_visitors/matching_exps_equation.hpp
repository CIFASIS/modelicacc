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
 * @brief Given a variable name, it detects the occurrences of that variable
 * in a given equation.
 */

#ifndef MODELICACC_UTIL_AST_VISITORS_MATCHING_EXPS_EQUATION_HPP_ 
#define MODELICACC_UTIL_AST_VISITORS_MATCHING_EXPS_EQUATION_HPP_ 

#include "ast/equation.hpp"
#include "ast/expression.hpp"
#include "util/ast_visitors/matching_exps.hpp"

#include <boost/variant/static_visitor.hpp>

namespace Modelica {

class MatchingExpsEquation : public boost::static_visitor<bool> {
public:
  MatchingExpsEquation(AST::Name var_name, bool state_var);

  const MatchingExps& matching_exprs() const;

  bool operator()(AST::Connect eq);
  bool operator()(AST::Equality eq);
  bool operator()(AST::CallEq eq);
  bool operator()(AST::IfEq eq);
  bool operator()(AST::WhenEq eq);
  bool operator()(AST::ForEq eq);

private:
  mutable MatchingExps _matching_exprs;
};

}  // namespace Modelica

#endif // MODELICACC_UTIL_AST_VISITORS_MATCHING_EXPS_EQUATION_HPP_
