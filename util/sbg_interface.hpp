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
 * @brief Module created to handle the conversion from SBG structures to
 * Modelica code. 
 */

#ifndef MODELICACC_UTIL_SBG_INTERFACE_HPP_
#define MODELICACC_UTIL_SBG_INTERFACE_HPP_

#include "ast/expression.hpp"

#include <sbg/expression.hpp>
#include <sbg/set.hpp>

#include <tuple>
#include <vector>

namespace Modelica {

/*
 * @brief A collection of elements, and the corresponding indices to access
 * them.
 */
using Access = std::pair<SBG::LIB::Set, AST::Indexes>;
using Accesses = std::vector<Access>;

/**
 * @brief Returns a collection of indices that describe the same elements of
 * the current _set using the names of \p counters, and the applies the
 * translation \p -t. For example, if _set is [1000:1500], \p t = 999
 * and counters = ["i"], it returns: (i, [1:501]).
 * It will be used to convert from SBG structures to Modelica code.
 */
Accesses toModelicaIndices(const SBG::LIB::Set& s, const SBG::LIB::IntTuple& t
  , const std::vector<AST::Name>& counters);

Accesses toModelicaIndices(const SBG::LIB::Set& s, const SBG::LIB::IntTuple& t
  , const std::vector<AST::Name>& counters, const SBG::LIB::Expression& expr);

} // namespace Modelica

#endif // MODELICACC_UTIL_SBG_INTERFACE_HPP_
