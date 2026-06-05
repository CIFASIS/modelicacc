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

#include <iostream>

namespace Modelica {

namespace Causalize {

// Constructors/Destructors ----------------------------------------------------

SetVertex::SetVertex(int node_id) : _node_id(node_id) {}

SetVertex::SetVertex(int node_id, CompactSet s)
  : _node_id(node_id), _set(s), _translation(s.arity()) {}

// Getters ---------------------------------------------------------------------

int SetVertex::node_id() const { return _node_id; }

std::size_t SetVertex::arity() const { return _set.arity(); }

std::string SetVertex::name() const { return _name; }

const CompactSet& SetVertex::set() const { return _set; }

const Translation& SetVertex::translation() const { return _translation; }

// Setters ---------------------------------------------------------------------

void SetVertex::set_node_id(int node_id) { _node_id = node_id; }

void SetVertex::set_name(std::string name) { _name = name; }

void SetVertex::set_translation(Translation t) { _translation = t; }

// Operators -------------------------------------------------------------------

std::ostream& operator<<(std::ostream& out, const SetVertex& sv)
{
  sv.print(out);

  return out;
}

// Methods ---------------------------------------------------------------------

bool SetVertex::isVariable() const { return _name.substr(0, 3) != "eq_"; }

bool SetVertex::isEquation() const { return _name.substr(0, 3) == "eq_"; }

std::ostream& SetVertex::print(std::ostream& out) const
{
  CompactSet copy = _set;
  copy.translate(_translation);
  out << _name << ": " << copy.toSBGFormat(); 
  return out;
}

std::string SetVertex::toSBGFormat() const
{
  CompactSet copy = _set;
  copy.translate(_translation);
  return copy.toSBGFormat();
}

void SetVertex::cartesianProduct(const CompactSet& s)
{
  _set.cartesianProduct(s);
}

void SetVertex::cartesianProduct(const SetVertex& other)
{
  _set.cartesianProduct(other._set);
}

AST::Integer SetVertex::maxDimPerimetral() const
{
  return _set.maxDimPerimetral();
}

} // namespace Causalize

} // namespace Modelica
