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
 * @brief Module used to convert the values of for indices to a SBG set. 
 */

#ifndef MODELICACC_UTIL_AST_VISITORS_EQUATION_SBG_SET_HPP_
#define MODELICACC_UTIL_AST_VISITORS_EQUATION_SBG_SET_HPP_

#include "ast/equation.hpp"
#include "util/table.hpp"

#include <boost/variant/static_visitor.hpp>
#include <sbg/set.hpp>

namespace Modelica {

/**
 * @brief Given an equation, it returns (if possible) an associated SBG set
 * with a bijection from each accessed variable to an element of the set.
 * Precondition: each ForEq should contain only one element in its list.
 */
class EquationToSBGSet : public boost::static_visitor<SBG::LIB::Set> {
public:
  EquationToSBGSet(VarSymbolTable vtable, unsigned int max_dim);
  SBG::LIB::Set operator()(Connect eq);
  SBG::LIB::Set operator()(Equality eq);
  SBG::LIB::Set operator()(CallEq eq);
  SBG::LIB::Set operator()(ForEq eq);
  SBG::LIB::Set operator()(IfEq eq);
  SBG::LIB::Set operator()(WhenEq eq);

private:
  unsigned int _max_dim;
  VarSymbolTable _vtable;
  IndexList _counters;
};

}  // namespace Modelica

#endif // MODELICACC_UTIL_AST_VISITORS_EQUATION_SBG_SET_HPP_
