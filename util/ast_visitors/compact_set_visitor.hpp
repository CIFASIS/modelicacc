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

#ifndef MODELICACC_UTIL_AST_VISITORS_COMPACT_SET_VISITOR_HPP_
#define MODELICACC_UTIL_AST_VISITORS_COMPACT_SET_VISITOR_HPP_

#include "ast/expression.hpp"
#include "util/hyper_rectangle.hpp"
#include "util/table.hpp"

#include <boost/variant/static_visitor.hpp>

#include <vector>

namespace Modelica {

/**
 * @brief Returns (if possible) an associated compact set in which each element
 * of the arrays accessed by the input AST::Expression is uniquely identified
 * by an element of the compact set. 
 */
class CompactSetVisitor : public boost::static_visitor<CompactSet> {
public:
  /**
   * @brief The second argument is an enviroment of declared counters that
   * are available to use in the expression.
   */
  explicit CompactSetVisitor(const VarSymbolTable& symbols
    , const IndexList& indices);
  CompactSet operator()(Integer v);
  CompactSet operator()(Boolean v);
  CompactSet operator()(AddAll v);
  CompactSet operator()(String v);
  CompactSet operator()(Name v);
  CompactSet operator()(Real v);
  CompactSet operator()(SubEnd v);
  CompactSet operator()(SubAll v);
  CompactSet operator()(BinOp);
  CompactSet operator()(UnaryOp);
  CompactSet operator()(Brace);
  CompactSet operator()(Bracket);
  CompactSet operator()(Call);
  CompactSet operator()(FunctionExp);
  CompactSet operator()(ForExp);
  CompactSet operator()(IfExp);
  CompactSet operator()(Named);
  CompactSet operator()(Output);
  CompactSet operator()(Reference);
  CompactSet operator()(Range);

  const std::vector<Name>& order();

protected:
  /**
   * @brief Checks if an access can be represented as a compact set.
   */
  void checkSupport(std::size_t k, const Expression& expr);

  VarSymbolTable _symbols;
  IndexList _env;

  Integer _dimension_size; ///< Used in the presence of SubAll
  std::vector<Name> _order;
};

CompactSet indicesToCompactSet(const IndexList&indices
  , const VarSymbolTable& symbols);

}  // namespace Modelica

#endif // MODELICACC_UTIL_AST_VISITORS_COMPACT_SET_VISITOR_HPP_
