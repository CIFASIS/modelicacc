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
#include <util/graph/graph_definition.h>

namespace Modelica {

using namespace Modelica::AST;

// Evaluates only linear expresions
class EvalExpFlatter{
  public:
  EvalExpFlatter(const VarSymbolTable &);
  EvalExpFlatter(const VarSymbolTable &, Name, Real);
  Interval operator()(Integer v) const;
  Interval operator()(Boolean v) const;
  Interval operator()(AddAll v) const;
  Interval operator()(String v) const;
  Interval operator()(Name v) const;
  Interval operator()(Real v) const;
  Interval operator()(SubEnd v) const;
  Interval operator()(SubAll v) const;
  Interval operator()(BinOp) const;
  Interval operator()(UnaryOp) const;
  Interval operator()(Brace) const;
  Interval operator()(Bracket) const;
  Interval operator()(Call) const;
  Interval operator()(FunctionExp) const;
  Interval operator()(ForExp) const;
  Interval operator()(IfExp) const;
  Interval operator()(Named) const;
  Interval operator()(Output) const;
  Interval operator()(Reference) const;
  Interval operator()(Range) const;
  const VarSymbolTable &vtable;
  Option<Name> name;
  Option<Real> val;
};

}  // namespace Modelica
#endif
