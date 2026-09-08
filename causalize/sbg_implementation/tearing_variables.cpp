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

#include "causalize/sbg_implementation/tearing_variables.hpp"

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// Tearing variable ------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

TearingVariable::TearingVariable(AST::Name name, AST::Bracket subscripts)
  : _name(name), _subscripts(subscripts) {}

const AST::Name& TearingVariable::name() const { return _name; }

const AST::Bracket& TearingVariable::subscripts() const { return _subscripts; }

std::ostream& operator<<(std::ostream& out, const TearingVariable& var)
{
  out << var.name() << var.subscripts();
  return out;
}

////////////////////////////////////////////////////////////////////////////////
// Tearing variables -----------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

bool TearingVariables::empty() const { return _variables.empty(); }

std::set<AST::Bracket>& TearingVariables::operator[](AST::Name name)
{
  return _variables[name];
}

const std::set<AST::Bracket>& TearingVariables::operator[](AST::Name name) const
{
  return _variables.at(name);
}

void TearingVariables::insert(TearingVariable variable)
{
  _variables[variable.name()].insert(variable.subscripts());
}

void TearingVariables::concatenation(TearingVariables other)
{
  for (const auto& [name, access] : other) {
    for (const AST::Bracket& subs : access) {
      insert(TearingVariable{name, subs});
    }
  }
}

std::vector<AST::Name> TearingVariables::variables() const
{
  std::vector<std::string> var_names;
  std::transform(
     _variables.begin(), _variables.end(), std::back_inserter(var_names)
    , [](const auto& pair) { return pair.first; }
  );
  return var_names;
}

std::ostream& operator<<(std::ostream& out, const TearingVariables& vars)
{
  for (auto const& [name, access] : vars) {
    for (const AST::Bracket& subs : access) {
      out << name << subs << "\n";
    }
    out << "\n";
  }
  return out;
}

}  // namespace Causalize

}  // namespace Modelica
