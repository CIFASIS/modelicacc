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

#ifndef MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_TEARING_VARIABLES_HPP_
#define MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_TEARING_VARIABLES_HPP_

#include "ast/expression.hpp"

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

  bool empty() const { return _variables.empty(); }

  auto begin() const { return _variables.begin(); }
  auto end() const { return _variables.end(); }

  std::set<AST::Bracket>& operator[](AST::Name name);
  const std::set<AST::Bracket>& operator[](AST::Name name) const;
  void insert(TearingVariable variable);
  std::vector<AST::Name> variables() const;

private:
  std::map<AST::Name, std::set<AST::Bracket>> _variables;
};

std::ostream& operator<<(std::ostream& out, const TearingVariables& vars);


} // namespace Causalize

} // namespace Modelica

#endif // MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_TEARING_VARIABLES_HPP_
