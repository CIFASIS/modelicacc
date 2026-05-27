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
#include "sbg/set.hpp"

#include <sstream>
#include <vector>

namespace Modelica {

class CompactSet {
public:
  CompactSet();
  CompactSet(AST::Integer start, AST::Integer step, AST::Integer end);
  CompactSet(SBG::LIB::Set s);

  std::size_t arity() const;

  bool operator==(const CompactSet& other) const;

  /**
   * @brief Returns a string-like result that is parsable by the SBG parser.
   */
  std::string toSBGFormat() const;

  std::size_t cardinal() const;
  void setUnion(const CompactSet& other);
  void intersection(const CompactSet& other);
  void cartesianProduct(const CompactSet& other);
  void reflection();
  void translate(const Translation& t);
  void scale(Integer factor);

  /**
   * @brief 
   */
  AST::Integer maxDimPerimetral() const;

private:
  SBG::LIB::Set _set;
};

} // namespace Modelica

#endif // MODELICACC_UTIL_COMPACT_SET_HPP_
