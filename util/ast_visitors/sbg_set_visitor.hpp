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

#ifndef MODELICACC_UTIL_AST_VISITORS_SBG_SET_VISITOR_HPP_
#define MODELICACC_UTIL_AST_VISITORS_SBG_SET_VISITOR_HPP_

#include "ast/expression.hpp"
#include "util/table.hpp"

#include <boost/variant/static_visitor.hpp>
#include <sbg/set.hpp>

#include <vector>

namespace Modelica {

/**
 * @brief Returns (if possible) an associated SBG set of an index.
 */
class SBGSetVisitor : public boost::static_visitor<SBG::LIB::Set> {
public:
  explicit SBGSetVisitor(const VarSymbolTable& symbols);
  SBG::LIB::Set operator()(Integer v);
  SBG::LIB::Set operator()(Boolean v);
  SBG::LIB::Set operator()(AddAll v);
  SBG::LIB::Set operator()(String v);
  SBG::LIB::Set operator()(Name v);
  SBG::LIB::Set operator()(Real v);
  SBG::LIB::Set operator()(SubEnd v);
  SBG::LIB::Set operator()(SubAll v);
  SBG::LIB::Set operator()(BinOp);
  SBG::LIB::Set operator()(UnaryOp);
  SBG::LIB::Set operator()(Brace);
  SBG::LIB::Set operator()(Bracket);
  SBG::LIB::Set operator()(Call);
  SBG::LIB::Set operator()(FunctionExp);
  SBG::LIB::Set operator()(ForExp);
  SBG::LIB::Set operator()(IfExp);
  SBG::LIB::Set operator()(Named);
  SBG::LIB::Set operator()(Output);
  SBG::LIB::Set operator()(Reference);
  SBG::LIB::Set operator()(Range);

protected:
  VarSymbolTable _symbols;
  IndexList _env;

  Integer _dimension_size; ///< Used in the presence of SubAll
};

}  // namespace Modelica

#endif // MODELICACC_UTIL_AST_VISITORS_SBG_SET_VISITOR_HPP_
