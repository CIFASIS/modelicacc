
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


//#include <flatten/change_compref.h>
#include <mmo/mmo_class.h>
#include <ast/equation.h>
#include <ast/statement.h>
#include <util/type.h>
#include <util/ast_visitors/eq_visitor.h>
#include <util/ast_visitors/st_visitor.h>
#include <util/ast_visitors/dot_expression.h>

#include <iostream>
#include <sstream> 

#ifndef RMCOMP_H
#define RMCOMP_H

using namespace std;
using namespace Modelica;
using namespace Modelica::AST;

struct Remove_Composition {
public:
	typedef EqVisitor<DotExpression> EqDotExpression;
	typedef StVisitor<DotExpression> StDotExpression;
	Remove_Composition();
	void 		LevelUp(MMO_Class &up, MMO_Class &down, Name nUp , VarInfo viUp);
private:
	Equation 	createForEquation	(IdentList index,ExpList indexVar, EquationList el);
	Statement 	createForStatement	(IdentList index,ExpList indexVar, StatementList el);
};


#endif
