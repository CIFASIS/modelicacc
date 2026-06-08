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

#include "causalize/sbg_implementation/set_edge.hpp"

#include <iostream>

namespace Modelica {

namespace Causalize {

// Constructors/Destructors ----------------------------------------------------

SetEdge::SetEdge(int edge_id, std::size_t arity)
  : _edge_id(edge_id), _map1(arity), _map2(arity) {}

SetEdge::SetEdge(int edge_id, CompactSet domain)
  : _edge_id(edge_id), _domain(domain), _map1(domain.arity())
    , _map2(domain.arity()) {}

// Getters ---------------------------------------------------------------------

int SetEdge::edge_id() const { return _edge_id; }

std::size_t SetEdge::arity() const { return _domain.arity(); }

std::string SetEdge::name() const { return _name; }

int SetEdge::var_id() const { return _var_id; }

int SetEdge::eq_id() const { return _eq_id; }

const Translation& SetEdge::translation() const { return _translation; }

const CompactSet& SetEdge::domain() const { return _domain; }

const Expression& SetEdge::access() const { return _access; }

// Setters ---------------------------------------------------------------------

void SetEdge::set_edge_id(int edge_id) { _edge_id = edge_id; }

void SetEdge::set_name(std::string name) { _name = name; }

void SetEdge::set_var_id(int var_id) { _var_id = var_id; }

void SetEdge::set_eq_id(int eq_id) { _eq_id = eq_id; }

void SetEdge::set_translation(Translation translation)
{
  _translation = translation;
}

void SetEdge::set_domain(CompactSet domain) { _domain = domain; }

void SetEdge::set_map1(CompactTransformation map1) { _map1 = map1; }

void SetEdge::set_map2(CompactTransformation map2) { _map2 = map2; }

void SetEdge::set_access(Expression access) { _access = access; }

// Operators -------------------------------------------------------------------

std::ostream& operator<<(std::ostream& out, const SetEdge& se)
{
  out << se.name() << ": " << se.toSBGFormat();

  return out;
}

// Methods ---------------------------------------------------------------------

std::string SetEdge::domainToSBGFormat() const
{
  return _domain.toSBGFormat();
}

std::string SetEdge::map1ToSBGFormat() const
{
  return _map1.toSBGFormat();
}

std::string SetEdge::map2ToSBGFormat() const
{
  return _map2.toSBGFormat();
}

std::string SetEdge::toSBGFormat() const
{
  return map1ToSBGFormat() + "<-" + _domain.toSBGFormat() + "->"
    + map2ToSBGFormat();
}

AST::Integer SetEdge::maxDimPerimetral()
{
  return _domain.maxDimPerimetral();
}

} // namespace Causalize

} // namespace Modelica
