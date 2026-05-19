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

#include "ast/queries.hpp"
#include "util/ast_visitors/compact_set_visitor.hpp"
#include "util/ast_visitors/contains_expression.hpp"
#include "util/ast_visitors/eval_expression.hpp"
#include "util/debug.hpp"

namespace Modelica {

CompactSetVisitor::CompactSetVisitor(const VarSymbolTable& symbols
  , const IndexList& indices) : _symbols(symbols), _env(indices)
{
  for (std::size_t k = 0; k < indices.size(); ++k) {
    _order.push_back("dummy_" + k);
    _symbols.insert(indices[k].name(), VarInfo{TypePrefixes{}, "Real"});
  }
}

CompactSet CompactSetVisitor::operator()(Integer v)
{
  return CompactSet{v, 1, v};
}

CompactSet CompactSetVisitor::operator()(Boolean v)
{
  ERROR("CompactSetVisitor: Boolean is not an compact set");
  return CompactSet{};
}

CompactSet CompactSetVisitor::operator()(AddAll v) 
{
  ERROR("CompactSetVisitor: AddAll is not an compact set");
  return CompactSet{};
}

CompactSet CompactSetVisitor::operator()(String v) 
{
  ERROR("CompactSetVisitor: String is not an compact set");
  return CompactSet{};
}

CompactSet CompactSetVisitor::operator()(Name v) 
{
  ERROR("CompactSetVisitor: Name is not an compact set");
  return CompactSet{};
}

CompactSet CompactSetVisitor::operator()(Real v) 
{
  ERROR("CompactSetVisitor: Real is not an compact set");
  return CompactSet{};
}

CompactSet CompactSetVisitor::operator()(SubEnd v)
{
  ERROR("CompactSetVisitor: SubEnd is not an compact set");
  return CompactSet{};
}

CompactSet CompactSetVisitor::operator()(SubAll v)
{
  return CompactSet{1, 1, _dimension_size};
}

CompactSet CompactSetVisitor::operator()(IfExp v)
{
  EvalExpression eval_expr{_symbols};
  Real cond = Apply(eval_expr, v.cond());
  if (cond == 1.0) {
    return ApplyThis(v.then());
  }

  for (const ExpPair& elseif : v.elseif()) {
    cond = Apply(eval_expr, get<0>(elseif));
    if (cond == 1.0) {
      return ApplyThis(get<1>(elseif));
    }
  }

  return ApplyThis(v.elseexp());
}

CompactSet CompactSetVisitor::operator()(Range v)
{
  EvalExpression eval_expr(_symbols);
  Integer start = Integer(Apply(eval_expr, v.start()));
  Integer step = 1;
  Integer end = Integer(Apply(eval_expr, v.end()));
  if (v.step()) {
    step = Integer(Apply(eval_expr, v.step().get()));
  }

  if (start > end) { // Decreasing range
    ERROR("CompactSetVisitor: only increasing Range expressions supported");
  }

  return CompactSet{start, step, end};
}

CompactSet CompactSetVisitor::operator()(Brace v)
{
  ERROR("CompactSetVisitor: Brace not supported");
  return CompactSet{};
}

CompactSet CompactSetVisitor::operator()(Bracket v)
{
  ERROR("CompactSetVisitor: Bracket not supported");
  return CompactSet{};
}

CompactSet CompactSetVisitor::operator()(Call v)
{
  if (v.name() == "der") {
    ERROR_UNLESS(v.args().size() == 1
      , "CompactSetVisitor: Call to der() ", v, " with 0 or >1 arguments");
    Expression expr = v.args().front();
    ERROR_UNLESS(is<Reference>(expr)
      , "CompactSetVisitor: argument to der() ", v, " is not a reference");
    return ApplyThis(expr);
  }

  return CompactSet{};
}

CompactSet CompactSetVisitor::operator()(FunctionExp v)
{
  ERROR("CompactSetVisitor: FunctionExp not supported");
  return CompactSet{};
}

CompactSet CompactSetVisitor::operator()(ForExp v)
{
  ERROR("CompactSetVisitor: ForExp not supported");
  return CompactSet{};
}

CompactSet CompactSetVisitor::operator()(Named v)
{
  ERROR("CompactSetVisitor: Named not supported");
  return CompactSet{};
}

CompactSet CompactSetVisitor::operator()(Output v)
{
  ERROR_UNLESS(v.args().size() == 1
    , "CompactSetVisitor: Output expressions with more than one element are not "
    , "supported");

  if (v.args().front()) {
    return ApplyThis(v.args().front().value());
  }

  ERROR("CompactSetVisitor: Output with no expression");
  return CompactSet{};
}

void CompactSetVisitor::checkSupport(std::size_t k, const Expression& expr)
{
  for (const Index& index : _env) {
    ContainsExpression contains_expr{Reference{index.name()}};
    bool appears = Apply(contains_expr, expr);
    if (appears && _order[k] == "dummy_" + k) {
      _order[k] = index.name(); 
    } else if (appears && _order[k] != index.name()) {
      ERROR("CompactSetVisitor: subscript ", expr, " not supported");
    }
  }
}

CompactSet CompactSetVisitor::operator()(Reference v)
{
  CompactSet result;

  // Get variable information
  Name v_name = refName(v);
  Option<VarInfo> v_info = _symbols[v_name]; 
  ERROR_UNLESS(v_info.has_value(), "CompactSetVisitor: undefined variable "
    , v_name);
  Option<ExpList> dimensions_sizes = v_info->indices();
  auto v_dimensions = dimensions_sizes.has_value() ? dimensions_sizes->size()
    : 0;
  ExpList subscripts = get<1>(v.ref().front());
  ERROR_UNLESS(subscripts.size() <= v_dimensions
    , "CompactSetVisitor: wrong number of subscripts in ", v);

  if (dimensions_sizes) { // Array variable
    EvalExpression eval_expr{_symbols};
    for (std::size_t k = 0; k < subscripts.size(); ++k) {
      checkSupport(k, subscripts[k]);

      _dimension_size = static_cast<Integer>(Apply(eval_expr
        , dimensions_sizes.value()[k]));
      CompactSet kth_set = ApplyThis(subscripts[k]);
      result.cartesianProduct(kth_set);
    }
  } else { // Scalar variable
    result = CompactSet{1, 1, 1};
  }

  return result;
}

CompactSet CompactSetVisitor::operator()(BinOp v)
{
  CompactSet left_set = ApplyThis(v.left());
  CompactSet right_set = ApplyThis(v.right());

  // TODO: check if one set can be obtained from the other applying a compact
  // transformation

  return left_set;
}

CompactSet CompactSetVisitor::operator()(UnaryOp v)
{
  return CompactSet{ApplyThis(v.exp())};
}

const std::vector<Name>& CompactSetVisitor::order() { return _order; }

CompactSet indexToCompactSet(const Index& index, const VarSymbolTable& symbols)
{
  OptExp expr = index.exp();
  if (!expr) {
    ERROR("indexToCompactSet: empty index");
  } else if (!is<Range>(expr.get())) {
    ERROR("indexToCompactSet: only Range expressions supported");
  }

  CompactSetVisitor set_visitor{symbols, IndexList{}};
  return Apply(set_visitor, expr.value());
}

CompactSet indicesToCompactSet(const IndexList& indices
  , const VarSymbolTable& symbols)
{
  CompactSet result;

  for (const Index& index : indices) {
    result.cartesianProduct(indexToCompactSet(index, symbols));
  }

  return result;
}

}  // namespace Modelica
