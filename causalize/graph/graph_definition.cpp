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

/*
 *  Created on: 21 Jul 2016
 *      Author: Diego Hollmann
 */

#include <causalize/graph/graph_definition.h>

namespace Causalize {

Unknown::Unknown() {}

Unknown::Unknown(Modelica::AST::Expression exp): expression(exp) {}

Unknown::Unknown(VarInfo varInfo, Modelica::AST::Reference var) {
  if (false && varInfo.state()) {
    expression = Modelica::AST::Call("der",Modelica::AST::Reference(var));
  } else {
    expression = Modelica::AST::Reference(var);
  }
}

Expression Unknown::operator() () const {
  return expression;
}

}
