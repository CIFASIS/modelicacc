
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
#ifndef EXTENDS_H
#define EXTENDS_H

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <ast/class.h>
#include <mmo/mmo_class.h>
#include <ast/equation.h>
#include <ast/statement.h>
#include <ast/modification.h>
#include <util/type.h>
#include <ast/ast_types.h>

using namespace Modelica;
using namespace Modelica::AST;

typedef Pair<Type::Type , MMO_Class &> typeContexTuple;
typedef boost::tuple<TypePrefixes,Type::Type , ExpList > typeDefinition;
typedef Option<typeDefinition> OptTypeDefinition;

struct ClassFinder {
public:
	ClassFinder();
	void expand(MMO_Class &up,MMO_Class &down);	
	void ExpandAll(MMO_Class &up);	
	
	Option<typeContexTuple> findTypeByName(MMO_Class &c,Name n) ;
	OptTypeDefinition getFinalClass(MMO_Class &c, Type::Type t) ;
	OptTypeDefinition resolveDependencies(MMO_Class &c,Name n);
	OptTypeDefinition resolveType(MMO_Class &c,Name t);
    
    void applyModification(MMO_Class &,MMO_Class &, Modification );
    void applyArgument(MMO_Class &,MMO_Class &, Argument );
    void applyClassModification(MMO_Class &,MMO_Class &, ClassModification );
    
private:

};


#endif
