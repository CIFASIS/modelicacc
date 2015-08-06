
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
#ifndef FLATTER_H
#define FLATTER_H

#include <flatter/remove_composition.h>
#include <flatter/classFinder.h>
#include <mmo/mmo_class.h>
#include <ast/equation.h>
#include <ast/statement.h>
#include <ast/element.h>
#include <util/type.h>
#include <util/ast_visitors/eq_visitor.h>
#include <util/ast_visitors/st_visitor.h>
#include <util/ast_visitors/dotExpression.h>
#include <util/ast_visitors/markConnector.h>
#include <boost/variant/get.hpp>

class Flatter {
public:
	typedef EqVisitor<dotExpression> EqdotExpression;
	typedef StVisitor<dotExpression> StdotExpression;

	Flatter();
	void 			Flat(MMO_Class &c,bool flatConnector,bool initial);
	void			removeConnectorVar(MMO_Class &c);
};


#endif
