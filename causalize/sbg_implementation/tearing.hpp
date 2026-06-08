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

#ifndef MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_TEARING_HPP_
#define MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_TEARING_HPP_

#include "ast/expression.hpp"
#include "causalize/sbg_implementation/algebraic_loops_detection.hpp"

#include <algorithms/scc/scc_data.hpp>

#include <iosfwd>
#include <map>
#include <set>

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// Tearing variable ------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

class TearingVariable {
public:
  TearingVariable(AST::Name name, AST::Bracket subscripts);

  const AST::Name& name() const;
  const AST::Bracket& subscripts() const;

private:
  AST::Name _name;
  AST::Bracket _subscripts;
};

std::ostream& operator<<(std::ostream& out, const TearingVariable& var);

class TearingVariables {
public:
  TearingVariables() = default;

  auto begin() const;
  auto end() const;

  std::set<AST::Bracket>& operator[](AST::Name name);
  const std::set<AST::Bracket>& operator[](AST::Name name) const;
  void insert(TearingVariable variable);

private:
  std::map<AST::Name, std::set<AST::Bracket>> _variables;
};

std::ostream& operator<<(std::ostream& out, const TearingVariables& vars);

////////////////////////////////////////////////////////////////////////////////
// Tearing variables detector --------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

/**
 * @class TearingDetector
 * @brief Identifies tearing variables of the model.
 */
class TearingDetector {
public:
  TearingDetector(AlgebraicLoopsInfo loops_info);

  TearingVariables detect();

private:
  void detectVariable(SetEdge se, CompactSet jth_tear);

  AlgebraicLoopsInfo _loops_info;
  TearingVariables _tearing;
};

} // namespace Causalize

} // namespace Modelica

#endif // MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_TEARING_HPP_
