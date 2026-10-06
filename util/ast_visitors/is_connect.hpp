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
 * @brief Given an equation it determines if it involves a Connect equation.
 *
 * ForEq must be flattened.
 */

#ifndef MODELICACC_UTIL_AST_VISITORS_IS_CONNECT_HPP_
#define MODELICACC_UTIL_AST_VISITORS_IS_CONNECT_HPP_

#include "ast/equation.hpp"
#include "util/table.hpp"

#include <boost/variant/static_visitor.hpp>

namespace Modelica {

class IsConnect : public boost::static_visitor<bool> {
public:
  IsConnect() = default;

  bool operator()(AST::Connect eq);
  bool operator()(AST::Equality eq);
  bool operator()(AST::CallEq eq);
  bool operator()(AST::ForEq eq);
  bool operator()(AST::IfEq eq);
  bool operator()(AST::WhenEq eq);
};

}  // namespace Modelica

#endif // MODELICACC_UTIL_AST_VISITORS_IS_CONNECT_HPP_
