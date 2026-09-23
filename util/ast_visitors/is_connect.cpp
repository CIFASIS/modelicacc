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

#include "util/ast_visitors/is_connect.hpp"
#include "util/debug.hpp"

namespace Modelica {

bool IsConnect::operator()(AST::Connect eq)
{
  return true;
}

bool IsConnect::operator()(AST::Equality eq)
{
  return false;
}

bool IsConnect::operator()(AST::CallEq eq)
{
  return false;
}

bool IsConnect::operator()(AST::ForEq eq)
{
  return ApplyThis(eq.elements().front());
}

bool IsConnect::operator()(AST::IfEq eq)
{
  return false;
}

bool IsConnect::operator()(AST::WhenEq eq)
{
  return false;
}

} // namespace Modelica
