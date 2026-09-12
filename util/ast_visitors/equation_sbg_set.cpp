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

#include "util/ast_visitors/equation_sbg_set.hpp"
#include "util/debug.hpp"
#include "util/ast_visitors/sbg_set_visitor.hpp"

namespace Modelica {

EquationToSBGSet::EquationToSBGSet(VarSymbolTable vtable
  , unsigned int max_dim) : _vtable(vtable), _max_dim(max_dim), _counters() {}

SBG::LIB::Set EquationToSBGSet::operator()(Connect eq)
{
  ERROR("EquationToSBGSet: Connect not yet supported");
  return SBG::LIB::Set{};
}

SBG::LIB::Set EquationToSBGSet::operator()(Equality eq)
{
  SBG::LIB::Set result;

  // Scalar equations
  if (_counters.empty()) {
    for (std::size_t k = 0; k < _max_dim; ++k) {
      if (k == 0) {
        result = SBG::LIB::Set{1, 1, 1};
      } else {
        result.cartesianProduct(SBG::LIB::Set{1, 1, 1});
      }
    }
    return result;
  }

  // Equation arrays; start inserting counters to environment
  for (const Index& counter : _counters) {
    OptExp counter_exp = counter.exp();
    ERROR_UNLESS(counter_exp.has_value(), "EquationToSBGSet: for index "
      , "without definition in ", eq);

    ExpList counter_indices{1, counter.exp().value()};
    VarInfo counter_info{TypePrefixes{}, counter.name(), Option<Comment>{}
      , Option<Modification>{}, Option<ExpList>{counter_indices}
      , false};
    _vtable[counter.name()] = counter_info;
  }

  // Calculate set for counters with new environment
  SBGSetVisitor set_visitor{_vtable};
  std::size_t k = 0;
  for (const Index& counter : _counters) {
    if (k == 0) {
      result = Apply(set_visitor, counter.exp().value());
    } else {
      result = result.cartesianProduct(Apply(
        set_visitor, counter.exp().value()
      ));
    }
    ++k;
  }

  // Fill remaining dimensions
  for (std::size_t k = _counters.size(); k < _max_dim; ++k) {
    result.cartesianProduct(SBG::LIB::Set{1, 1, 1});
  }

  // TODO: check that expressions in equations are compatible with the result
  // and rotate if necessary. Only 1D supported.
  //for (/*each state variable*/) {
  //  MatchingExps matching_exprs(var_name, isState(var_name, _vtable));
  //  Apply(matching_exprs, eq.left());
  //  Apply(matching_exprs, eq.right());
  //  std::set<Expression> matched_exprs = matching_exprs.matchedExps();
  //  for (const Expression& expr : matched_exprs) {
  //  }
  //}

  return result;
}

SBG::LIB::Set EquationToSBGSet::operator()(CallEq eq)
{
  ERROR("EquationToSBGSet: trying to convert a CallEq");
  return SBG::LIB::Set{};
}

SBG::LIB::Set EquationToSBGSet::operator()(ForEq eq)
{
  IndexList for_indices = eq.range().indexes();
  _counters.insert(_counters.end(), for_indices.begin(), for_indices.end());
  EquationList eq_elems = eq.elements();
  ERROR_UNLESS(eq_elems.size() == 1, "EquationToSBGSet: ForEq should be "
    , "composed by a singleton list");

  return ApplyThis(eq_elems.front());
}

SBG::LIB::Set EquationToSBGSet::operator()(IfEq eq)
{
  ERROR("EquationToSBGSet: trying to convert an IfEq");
  return SBG::LIB::Set{};
}

SBG::LIB::Set EquationToSBGSet::operator()(WhenEq eq)
{
  ERROR("EquationToSBGSet: trying to convert a WhenEq");
  return SBG::LIB::Set{};
}

} // namespace Modelica
