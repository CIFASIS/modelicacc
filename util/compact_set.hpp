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

#ifndef MODELICACC_UTIL_COMPACT_SET_HPP_
#define MODELICACC_UTIL_COMPACT_SET_HPP_

#include "ast/expression.hpp"
#include "util/affine_transformation.hpp"
#include "util/table.hpp"
#include "util/translation.hpp"

#include <sbg/expression.hpp>
#include <sbg/set.hpp>

#include <sstream>
#include <vector>

namespace Modelica {

class CompactSet {
public:
  CompactSet();
  CompactSet(AST::Integer start, AST::Integer step, AST::Integer end);
  CompactSet(SBG::LIB::Set s);

  const SBG::LIB::Set& set() const;
  std::size_t arity() const;

  bool operator==(const CompactSet& other) const;

  std::size_t cardinal() const;
  void setUnion(const CompactSet& other);
  void intersection(const CompactSet& other);
  void cartesianProduct(const CompactSet& other);
  void reflection();
  void translate(const Translation& t);
  void scale(Integer factor);

  /**
   * @brief Returns the maximum coordinate between dimensions of any point of
   * the set. For example, maxDimPerimetral({(1, 10), (15, 2)}) = 15. 
   */
  AST::Integer maxDimPerimetral() const;

  /**
   * @brief Returns a string-like result that is parsable by the SBG parser.
   * It will be used by GenerateSBGInput, during the SBG generation from the
   * Modelica model.
   */
  std::string toSBGFormat() const;

private:
  SBG::LIB::Set _set;
};

// Non-member functions --------------------------------------------------------

/*
 * @brief A collection of elements, and the corresponding indices to access
 * them.
 */
using Access = std::pair<CompactSet, Indexes>;
using Accesses = std::vector<Access>;

/**
 * @brief Returns a collection of indices that describe the same elements of
 * the current _set using the names of \p counters, and the applies the
 * translation \p -t. For example, if _set is [1000:1500], \p t = 999
 * and counters = ["i"], it returns: (i, [1:501]).
 * It will be used by HorizontalSorting during causalization to traduce back
 * the SBG to Modelica code.
 */
Accesses toModelicaIndices(const CompactSet& s, const Translation& t
  , const std::vector<Name>& counters);

Accesses toModelicaIndices(const CompactSet& s, const Translation& t
  , const std::vector<Name>& counters, const SBG::LIB::Expression& expr);

} // namespace Modelica

#endif // MODELICACC_UTIL_COMPACT_SET_HPP_
