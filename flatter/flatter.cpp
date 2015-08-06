
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

#include <flatter/flatter.h>
#include <util/ast_visitors/part_evalexp.h>
#include <boost/algorithm/string.hpp>
#include <util/debug.h>

#include <iostream>
using namespace std;
#define Visit(X,Y) boost::apply_visitor(X,Y)

Flatter::Flatter(){}


void Flatter::removeConnectorVar(MMO_Class &c)
{
	ClassFinder re =  ClassFinder();
	VarSymbolTable & vsd = c.syms_ref();
	IdentList vars = c.variables();
	c.set_variables(IdentList());
	foreach_(Name n,vars) {
		if (!vsd[n]) continue;
			VarInfo v = vsd[n].get();
			OptTypeDefinition m = re.resolveType(c,v.type());
			if (m) {
				typeDefinition td = m.get();
				Type::Type t_final = get<1>(td);
				if (!is<Type::Class>(t_final))  c.variables_ref().push_back(n);
			}
	}
}

void Flatter::Flat(MMO_Class &c, bool flatConnector,bool initial)
{
	ClassFinder re = ClassFinder();
	Remove_Composition rc = Remove_Composition();
	re.ExpandAll(c);

	VarSymbolTable & vsd = c.syms_ref();
	IdentList vars = c.variables();
	c.set_variables(IdentList());
	foreach_(Name n , vars) {
		if (!vsd[n]) continue;
		VarInfo v = vsd[n].get();
		OptTypeDefinition m = re.resolveType(c,v.type());
		if ( m ) {
			typeDefinition td = m.get();
			Type::Type t_final = get<1>(td);
			ExpList indexes = get<2>(td);
			if (v.indices()) indexes += v.indices().get();
			if (indexes.size() > 0 ) v.set_indices(indexes);
			if ( is<Type::Class>(t_final)) {
				MMO_Class down =  *(boost::get<Type::Class>(t_final).clase());
				//if (flatConnector || !down.isConnector()) {
					if (v.modification()) re.applyModification(c,down,v.modification().get());
					Flat(down,flatConnector,false);
					rc.LevelUp(c,down,n,v);
					if (down.isConnector()) {
						c.variables_ref().push_back(n);
						Type::Class tc = boost::get<Type::Class>(t_final);
						Name newType = replace_all_copy(v.type(),".","_");
	                                        c.types_ref().push_back(newType);
        	                                c.tyTable_ref().insert(newType,t_final);
						v.set_type(newType);
						c.syms_ref().insert(n,v);
					}
				//} else c.variables_ref().push_back(n);
			} else {
				v.set_prefixes( v.prefixes() + get<0>(td) );
				if (is<Type::String>(t_final)) v.set_type("String");
				if (is<Type::Integer>(t_final)) v.set_type("Integer");
				if (is<Type::Real>(t_final)) v.set_type("Real");
				if (is<Type::Boolean>(t_final)) v.set_type("Boolean");
				c.syms_ref().insert(n,v);
				c.variables_ref().push_back(n);
			}
		} else { 
			std::cerr << "No se pudo definir el tipo de la variable " << n << " en " << c.name() << std::endl;	
			exit(-1);
		}	
	}
	
	MarkConnector mc;
	foreach_(Equation &eq,c.equations_ref().equations_ref())	
		eq = Visit(mc,eq);
	foreach_(Equation &eq,c.initial_eqs_ref().equations_ref())	
		eq = Visit(mc,eq);
	
	//Eliminar Variables
	if (initial) {
		dotExpression dot = dotExpression(Option<MMO_Class &> (),"",ExpList());
		EqdotExpression eqChange = EqdotExpression(dot);	
		StdotExpression stChange = StdotExpression(dot);
		
		foreach_(Equation &eq,c.equations_ref().equations_ref())	
			eq = Visit(eqChange,eq);
		foreach_(Equation &eq,c.initial_eqs_ref().equations_ref())	
			eq = Visit(eqChange,eq);
		
		foreach_(Statement &st,c.statements_ref().statements_ref())	
			st = Visit(stChange,st);
		foreach_(Statement &st,c.initial_sts_ref().statements_ref())	
			st = Visit(stChange,st);
		c.types_ref().clear();	
	
		// Remove if equation when evauate to false and dont have else 
		EquationList to_delete;
		EquationList to_add;
		foreach_(Equation &eq,c.equations_ref().equations_ref())	{ // Evaluate if equations
			if (is<IfEq>(eq)) {
				IfEq &ieq = get<IfEq>(eq);
				if (ieq.ifnot().size()==0 && ieq.elseif().size()==0) {
				  PartEvalExp pe(c.syms_ref());  
				  Expression res = boost::apply_visitor(pe,ieq.cond_ref());
				  ERROR_UNLESS(is<Boolean>(res), "Error evaluating const if equation condition");
				  if (get<Boolean>(res).val()) {
					to_add.insert(to_add.end(),ieq.elements_ref().begin(), ieq.elements_ref().end());
				  } 
				  to_delete.push_back(ieq);
				}
			}
		}
		EquationList &eqs=c.equations_ref().equations_ref();
		eqs.insert(eqs.end(),to_add.begin(),to_add.end());
		  foreach_(Equation &eq,to_delete) 
		  eqs.erase(std::find(eqs.begin(),eqs.end(),eq));
   }
}
