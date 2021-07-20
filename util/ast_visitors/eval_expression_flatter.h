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

#ifndef AST_VISITOR_EVALEXP_FLATTER
#define AST_VISITOR_EVALEXP_FLATTER
#include <boost/variant/static_visitor.hpp>
#include <ast/expression.h>
#include <util/table.h>
#include <util/graph/sbg/graph_definition.h>

namespace Modelica {

using namespace Modelica::AST;

// Evaluates only linear expresions
class EvalExpFlatter{
  public:
  EvalExpFlatter(const VarSymbolTable &);
  EvalExpFlatter(const VarSymbolTable &, Name, Real);
  SBG::Interval operator()(Integer v) const;
  SBG::Interval operator()(Boolean v) const;
  SBG::Interval operator()(AddAll v) const;
  SBG::Interval operator()(String v) const;
  SBG::Interval operator()(Name v) const;
  SBG::Interval operator()(Real v) const;
  SBG::Interval operator()(SubEnd v) const;
  SBG::Interval operator()(SubAll v) const;
  SBG::Interval operator()(BinOp) const;
  SBG::Interval operator()(UnaryOp) const;
  SBG::Interval operator()(Brace) const;
  SBG::Interval operator()(Bracket) const;
  SBG::Interval operator()(Call) const;
  SBG::Interval operator()(FunctionExp) const;
  SBG::Interval operator()(ForExp) const;
  SBG::Interval operator()(IfExp) const;
  SBG::Interval operator()(Named) const;
  SBG::Interval operator()(Output) const;
  SBG::Interval operator()(Reference) const;
  SBG::Interval operator()(Range) const;
  const VarSymbolTable &vtable;
  Option<Name> name;
  Option<Real> val;
};

}  // namespace Modelica
#endif
