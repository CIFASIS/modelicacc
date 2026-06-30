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
#include <string>
#include <utility>

namespace Modelica {

// Constructors/destructors ----------------------------------------------------

CompactSet::CompactSet() {}

CompactSet::CompactSet(AST::Integer start, AST::Integer step, AST::Integer end)
{
  ERROR_UNLESS(start >= 0 && step >= 0 && end >= 0, "CompactSet: negative ", "values not supported");

  _set = SBG::LIB::Set(static_cast<SBG::LIB::NAT>(start), static_cast<SBG::LIB::NAT>(step), static_cast<SBG::LIB::NAT>(end));
}

CompactSet::CompactSet(SBG::LIB::Set s) { _set = s; }

// Getters ---------------------------------------------------------------------

const SBG::LIB::Set& CompactSet::set() const { return _set; }

std::size_t CompactSet::arity() const { return _set.arity(); }

// Set operations --------------------------------------------------------------

std::size_t CompactSet::cardinal() const { return _set.cardinal(); }

bool CompactSet::operator==(const CompactSet& other) const { return _set == other._set; }

void CompactSet::setUnion(const CompactSet& other) { _set = _set.cup(other._set); }

void CompactSet::intersection(const CompactSet& other) { _set = _set.intersection(other._set); }

void CompactSet::cartesianProduct(const CompactSet& other) { _set = _set.cartesianProduct(other._set); }

// Additional methods ----------------------------------------------------------

void CompactSet::reflection() { ERROR("CompactSet::reflection: not supported yet"); }

void CompactSet::translate(const Translation& t)
{
  ERROR_UNLESS(t.arity() == _set.arity(), "CompactSet::translate: dimensions ", "of compact set ", "and translation are different");

  SBG::LIB::MD_NAT t_val;
  for (std::size_t k = 0; k < t.arity(); ++k) {
    t_val.pushBack(static_cast<SBG::LIB::NAT>(t[k]));
  }
  _set = _set.offset(t_val);
}

void CompactSet::scale(AST::Integer factor) { ERROR("CompactSet::scale: not supported yet"); }

AST::Integer CompactSet::maxDimPerimetral() const
{
  SBG::LIB::NAT maximum = 0;

  SBG::LIB::MD_NAT perimetral_max = _set.perimeter().max();
  for (std::size_t k = 0; k < arity(); ++k) {
    maximum = std::max(maximum, perimetral_max[k]);
  }

  return static_cast<AST::Integer>(maximum);
}

std::string CompactSet::toSBGFormat() const
{
  std::ostringstream out;
  out << _set;
  return out.str();
}

// Non-member functions -------------------------------------------------------

// TODO: generalize this to return an expression.
int toNumber(const rapidjson::Value& value)
{
  std::string str_expr(value.GetString(), value.GetStringLength());
  try {
    std::stoi(str_expr);
  } catch (const std::invalid_argument& e) {
    std::cerr << "Invalid argument: Could not parse string to int.\n";
  } catch (const std::out_of_range& e) {
    std::cerr << "Out of range: The number is too large for an int.\n";
  }

  return std::stoi(str_expr);
}

// TODO: generalize to analyze step, slope and offset.
Index dimensionToModelicaIndices(const rapidjson::Value& kth_bounds, const rapidjson::Value& kth_expr, Integer offset, Name counter)
{
  ERROR_UNLESS(kth_bounds.Size() == 3, "dimensionToModelicaIndices: interval ", " not defined with three values");

  int m = toNumber(kth_expr[0]);
  int h = toNumber(kth_expr[1]);

  int begin = kth_bounds[0].GetInt() - offset;
  int step = kth_bounds[1].GetInt();
  int end = kth_bounds[2].GetInt() - offset;
  if (m * h < 0) {
    step = -step;
    std::swap(begin, end);
  }

  return Index{counter, OptExp{Range{begin, step, end}}};
}

Indexes pieceToModelicaIndices(const rapidjson::Value& piece, const rapidjson::Value& expr_json, const Translation& t,
                               const std::vector<Name>& counters)
{
  IndexList result;

  std::size_t k = 0;
  const rapidjson::Value& bounds = piece["bounds"];
  for (const rapidjson::Value& kth_bounds : bounds.GetArray()) {
    result.push_back(dimensionToModelicaIndices(kth_bounds, expr_json[k], t[k], counters[k]));
    ++k;
  }

  return Indexes{result};
}

std::vector<Indexes> toModelicaIndices(const CompactSet& s, const Translation& t, const std::vector<Name>& counters,
                                       const SBG::LIB::Expression& expr)
{
  rapidjson::Document set_doc;
  rapidjson::Value set_json = s.set().toJSON(set_doc.GetAllocator());
  ERROR_UNLESS(set_json.IsObject(),
               "toModelicaIndices: value "
               "is not an object");
  ERROR_UNLESS(set_json.HasMember("pieces"), "toModelicaIndices: ", "incorrect SBG::LIB::Set format");

  rapidjson::Document expr_doc;
  rapidjson::Value expr_json = expr.toJSON(expr_doc.GetAllocator());
  ERROR_UNLESS(expr_json.IsArray(),
               "toModelicaIndices: value "
               "is not an expression");

  std::vector<Indexes> result;
  const rapidjson::Value& pieces = set_json["pieces"];
  for (const auto& piece : pieces.GetArray()) {
    result.push_back(pieceToModelicaIndices(piece, expr_json, t, counters));
  }

  return result;
}

std::vector<Indexes> toModelicaIndices(const CompactSet& s, const Translation& t, const std::vector<Name>& counters)
{
  return toModelicaIndices(s, t, counters, SBG::LIB::Expression{counters.size()});
}

}  // namespace Modelica
