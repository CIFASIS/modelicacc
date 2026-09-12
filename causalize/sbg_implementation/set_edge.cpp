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
  : _edge_id(edge_id), _map1(arity, 0, 0), _map2(arity, 0, 0) {}

SetEdge::SetEdge(int edge_id, SBG::LIB::Set domain)
  : _edge_id(edge_id), _domain(domain), _map1(domain.arity(), 0, 0)
    , _map2(domain.arity(), 0, 0) {}

// Getters ---------------------------------------------------------------------

int SetEdge::edge_id() const { return _edge_id; }

std::string SetEdge::name() const { return _name; }

int SetEdge::var_id() const { return _var_id; }

int SetEdge::eq_id() const { return _eq_id; }

const SBG::LIB::IntTuple& SetEdge::translation() const { return _translation; }

const SBG::LIB::Set& SetEdge::domain() const { return _domain; }

const SBG::LIB::Expression& SetEdge::map1() const { return _map1; }

const SBG::LIB::Expression& SetEdge::map2() const { return _map2; }

const AST::Expression& SetEdge::access() const { return _access; }

std::size_t SetEdge::arity() const { return _domain.arity(); }

// Setters ---------------------------------------------------------------------

void SetEdge::set_edge_id(int edge_id) { _edge_id = edge_id; }

void SetEdge::set_name(std::string name) { _name = name; }

void SetEdge::set_var_id(int var_id) { _var_id = var_id; }

void SetEdge::set_eq_id(int eq_id) { _eq_id = eq_id; }

void SetEdge::set_translation(SBG::LIB::IntTuple translation)
{
  _translation = translation;
}

void SetEdge::set_domain(SBG::LIB::Set domain) { _domain = domain; }

void SetEdge::set_map1(SBG::LIB::Expression map1) { _map1 = map1; }

void SetEdge::set_map2(SBG::LIB::Expression map2) { _map2 = map2; }

void SetEdge::set_access(AST::Expression access) { _access = access; }

// Operators -------------------------------------------------------------------

std::ostream& operator<<(std::ostream& out, const SetEdge& se)
{
  out << se.name() << ": " << se.toSBGFormat();

  return out;
}

// Methods ---------------------------------------------------------------------

std::string SetEdge::domainToSBGFormat() const
{
  std::ostringstream oss;
  oss << _domain.translate(_translation);
  return oss.str();
}

std::string SetEdge::map1ToSBGFormat() const
{
  std::ostringstream oss;
  oss << _map1;
  return oss.str();
}

std::string SetEdge::map2ToSBGFormat() const
{
  std::ostringstream oss;
  oss << _map2;
  return oss.str();
}

std::string SetEdge::toSBGFormat() const
{
  return map1ToSBGFormat() + "<-" + domainToSBGFormat() + "->"
    + map2ToSBGFormat();
}

SBG::LIB::Set SetEdge::translatedDomain() const
{
  return _domain.translate(_translation);
}

AST::Integer SetEdge::maxDimPerimetral()
{
  SBG::LIB::Int maximum = 0;

  SBG::LIB::IntTuple perimetral_max = _domain.perimeter().max();
  for (std::size_t k = 0; k < arity(); ++k) {
    maximum = std::max(maximum, perimetral_max[k]);
  }

  return maximum;
}

SetEdge SetEdge::restrict(SBG::LIB::Set restriction) const
{
  if (restriction.cardinal() == 0) {
    return SetEdge{_edge_id, 0};
  }
  SBG::LIB::IntTuple neg_translation;
  for (const SBG::LIB::Int x : _translation) {
    neg_translation.pushBack(-x);
  }
  restriction = restriction.translate(neg_translation);
  restriction = restriction.intersection(_domain);

  SetEdge result{_edge_id, restriction};

  result.set_name(_name);
  result.set_var_id(_var_id);
  result.set_eq_id(_eq_id);
  result.set_translation(_translation);
  result.set_map1(_map1);
  result.set_map2(_map2);
  result.set_access(_access);

  return result;
}

}  // namespace Causalize

}  // namespace Modelica
