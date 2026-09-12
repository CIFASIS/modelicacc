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

#include "causalize/sbg_implementation/set_vertex.hpp"
#include "ast/modification.hpp"

#include <iostream>
#include <sstream>

namespace Modelica {

namespace Causalize {

// Constructors/Destructors ----------------------------------------------------

SetVertex::SetVertex(int node_id) : _node_id(node_id) {}

SetVertex::SetVertex(int node_id, SBG::LIB::Set s)
  : _node_id(node_id), _set(s), _translation(s.arity()) {}

// Getters ---------------------------------------------------------------------

int SetVertex::node_id() const { return _node_id; }

std::size_t SetVertex::arity() const { return _set.arity(); }

std::string SetVertex::name() const { return _name; }

const SBG::LIB::Set& SetVertex::set() const { return _set; }

const SBG::LIB::IntTuple& SetVertex::translation() const
{
  return _translation;
}

const VertexInfo& SetVertex::info() const { return _info; }

// Setters ---------------------------------------------------------------------

void SetVertex::set_node_id(int node_id) { _node_id = node_id; }

void SetVertex::set_name(std::string name) { _name = name; }

void SetVertex::set_translation(SBG::LIB::IntTuple translation)
{
  _translation = translation;
}

void SetVertex::set_info(VertexInfo info) { _info = info; }

// Operators -------------------------------------------------------------------

std::ostream& operator<<(std::ostream& out, const SetVertex& sv)
{
  sv.print(out);

  return out;
}

// Member functions ------------------------------------------------------------

bool SetVertex::isVariable() const { return _name.substr(0, 3) != "eq_"; }

bool SetVertex::isEquation() const { return _name.substr(0, 3) == "eq_"; }

std::ostream& SetVertex::print(std::ostream& out) const
{
  out << _name << ": " << _set.translate(_translation);
  return out;
}

std::string SetVertex::toSBGFormat() const
{
  std::ostringstream oss;
  oss << _set.translate(_translation);
  return oss.str();
}

void SetVertex::cartesianProduct(const SBG::LIB::Set& s)
{
  _set = _set.cartesianProduct(s);
}

SBG::LIB::Int SetVertex::maxDimPerimetral() const
{
  return Modelica::Causalize::maxDimPerimetral(_set);
}

SBG::LIB::Int maxDimPerimetral(SBG::LIB::Set s)
{
  SBG::LIB::Int maximum = 0;

  SBG::LIB::IntTuple perimetral_max = s.perimeter().max();
  for (std::size_t k = 0; k < s.arity(); ++k) {
    maximum = std::max(maximum, perimetral_max[k]);
  }

  return maximum;
}

}  // namespace Causalize

}  // namespace Modelica
