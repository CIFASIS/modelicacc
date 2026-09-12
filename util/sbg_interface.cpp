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

#include "util/sbg_interface.hpp"
#include "util/ast_visitors/eval_expression.hpp"
#include "util/debug.hpp"

#include <boost/variant/get.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <utility>

namespace Modelica {

namespace {

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
std::pair<SBG::LIB::Set, Index> dimensionToModelicaIndices(
  const rapidjson::Value& kth_bounds, const rapidjson::Value& kth_expr
  , Integer offset, Name counter
)
{
  ERROR_UNLESS(kth_bounds.Size() == 3, "dimensionToModelicaIndices: interval "
    , " not defined with three values");

  int m = toNumber(kth_expr[0]);
  int h = toNumber(kth_expr[1]);

  int begin = kth_bounds[0].GetInt() - offset;
  int step = kth_bounds[1].GetInt();
  int end = kth_bounds[2].GetInt() - offset;
  if (m*begin + h > begin) {
    step = -step;
    std::swap(begin, end);
  }
  SBG::LIB::Int set_begin = std::min(begin, end);
  SBG::LIB::Int set_step = std::abs(step);
  SBG::LIB::Int set_end = std::max(begin, end);

  return {SBG::LIB::Set{set_begin, set_step, set_end}
    , Index{counter, OptExp{Range{begin, step, end}}}};
}

Access pieceToModelicaIndices(
  const rapidjson::Value& piece, const rapidjson::Value& expr_json
  , const SBG::LIB::IntTuple& t, const std::vector<Name>& counters
)
{
  IndexList result;

  SBG::LIB::Set s;
  std::size_t k = 0;
  const rapidjson::Value& bounds = piece["bounds"];
  for (const rapidjson::Value& kth_bounds : bounds.GetArray()) {
    auto [kth_set, idx] = dimensionToModelicaIndices(
       kth_bounds, expr_json[k], t[k], counters[k]
    );
    result.push_back(idx);
    if (k == 0) {
      s = kth_set;
    } else {
      s.cartesianProduct(kth_set);
    }
    ++k;
  }
  s = s.translate(t);

  return {s, Indexes{result}};
}

} // namespace

Accesses toModelicaIndices(
  const SBG::LIB::Set& s, const SBG::LIB::IntTuple& t
  , const std::vector<Name>& counters
  , const SBG::LIB::Expression& expr
)
{
  rapidjson::Document set_doc;
  rapidjson::Value set_json = s.toJSON(set_doc.GetAllocator());
  ERROR_UNLESS(set_json.IsObject(),
               "toModelicaIndices: value "
               "is not an object");
  ERROR_UNLESS(set_json.HasMember("pieces"), "toModelicaIndices: "
    , "incorrect SBG::LIB::Set format");

  rapidjson::Document expr_doc;
  rapidjson::Value expr_json = expr.toJSON(expr_doc.GetAllocator());
  ERROR_UNLESS(expr_json.IsArray(), "toModelicaIndices: value is not an "
    , "expression");

  Accesses result;
  const rapidjson::Value& pieces = set_json["pieces"];
  for (const auto& piece : pieces.GetArray()) {
    result.push_back(pieceToModelicaIndices(piece, expr_json, t, counters));
  }

  return result;
}

Accesses toModelicaIndices(
  const SBG::LIB::Set& s
  , const SBG::LIB::IntTuple& t
  , const std::vector<Name>& counters
)
{
  return toModelicaIndices(
    s, t, counters, SBG::LIB::Expression{counters.size()}
  );
}

}  // namespace Modelica
