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

#include "util/compact_set.hpp"
#include "util/ast_visitors/eval_expression.hpp"
#include "util/debug.hpp"

#include <boost/variant/get.hpp>

#include <iostream>
#include <utility>

namespace Modelica {

// Constructors/destructors ----------------------------------------------------

CompactSet::CompactSet() {}

CompactSet::CompactSet(AST::Integer start, AST::Integer step
  , AST::Integer end)
{
  ERROR_UNLESS(start >= 0 && step >= 0 && end >= 0, "CompactSet: negative "
    , "values not supported");

  _set = SBG::LIB::Set(static_cast<SBG::LIB::NAT>(start)
    , static_cast<SBG::LIB::NAT>(step)
    , static_cast<SBG::LIB::NAT>(end));
}

CompactSet::CompactSet(SBG::LIB::Set s) { _set = s; }

// Getters ---------------------------------------------------------------------

std::size_t CompactSet::arity() const { return _set.arity(); }

// Operators -------------------------------------------------------------------

std::size_t CompactSet::cardinal() const { return _set.cardinal(); }

bool CompactSet::operator==(const CompactSet& other) const
{
  return _set == other._set;
}

void CompactSet::setUnion(const CompactSet& other)
{
  _set = _set.cup(other._set);
}

void CompactSet::intersection(const CompactSet& other)
{
  _set = _set.intersection(other._set);
}

void CompactSet::cartesianProduct(const CompactSet& other)
{
  _set = _set.cartesianProduct(other._set);
}

std::string CompactSet::toSBGFormat() const
{
  std::ostringstream out;
  out << _set;
  return out.str();
}

// Methods ---------------------------------------------------------------------

void CompactSet::reflection()
{
  ERROR("CompactSet::reflection: not supported yet");
}

void CompactSet::translate(const Translation& t)
{
  ERROR_UNLESS(t.arity() == _set.arity(), "CompactSet::translate: dimensions "
    , "of compact set and translation are different");

  SBG::LIB::MD_NAT t_val;
  for (std::size_t k = 0; k < t.arity(); ++k) {
    t_val.pushBack(static_cast<SBG::LIB::NAT>(t[k]));
  } 
  _set = _set.offset(t_val);
}

void CompactSet::scale(AST::Integer factor)
{
  ERROR("CompactSet::scale: not supported yet");
}

//AST::Integer CompactSet::maxDimSize() const
//{
//  AST::Integer maximum = 0;
//
//  //unsigned int arity = _starts.size();
//  //for (unsigned int k = 0; k < arity; ++k) {
//  //  maximum = std::max(maximum, (_ends[k] - _starts[k])/_steps[k]);
//  //}
//
//  return maximum;
//}

AST::Integer CompactSet::maxDimPerimetral() const
{
  SBG::LIB::NAT maximum = 0;

  SBG::LIB::MD_NAT perimetral_max = _set.perimeter().max();
  for (std::size_t k = 0; k < arity(); ++k) {
    maximum = std::max(maximum, perimetral_max[k]);
  }

  return static_cast<AST::Integer>(maximum);
}

} // namespace Modelica
