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

#ifndef MODELICACC_UTIL_AST_VISITORS_EQUATION_COMPACT_SET_HPP_
#define MODELICACC_UTIL_AST_VISITORS_EQUATION_COMPACT_SET_HPP_

#include "ast/equation.hpp"
#include "util/compact_set.hpp"
#include "util/table.hpp"

#include <boost/variant/static_visitor.hpp>

namespace Modelica {

/**
 * @brief Given an equation, it returns (if possible) an associated compact set
 * with a bijection from each accessed variable to an element of the set.
 * Precondition: each ForEq should contain only one element in its list.
 */
class EquationCompactSet : public boost::static_visitor<CompactSet> {
public:
  EquationCompactSet(VarSymbolTable vtable, unsigned int max_dim);
  CompactSet operator()(Connect eq);
  CompactSet operator()(Equality eq);
  CompactSet operator()(CallEq eq);
  CompactSet operator()(ForEq eq);
  CompactSet operator()(IfEq eq);
  CompactSet operator()(WhenEq eq);

private:
  unsigned int _max_dim;
  VarSymbolTable _vtable;
  IndexList _counters;
};

}  // namespace Modelica

#endif // MODELICACC_UTIL_AST_VISITORS_EQUATION_COMPACT_SET_HPP_
