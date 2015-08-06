
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

#include <flatter/classFinder.h>
#include <boost/variant/get.hpp>
#include <iostream>
#include <util/debug.h>

using namespace std;
using namespace Modelica;

ClassFinder::ClassFinder() {}

void ClassFinder::expand(MMO_Class &up,MMO_Class & down)
{
	foreach_(Equation eq,down.equations_ref().equations())	
		up.addEquation(eq);
	foreach_(Equation eq,down.initial_eqs().equations())	
		up.addInitEquation(eq);
	foreach_(Statement st, down.statements().statements())
		up.addStatement(st);
	foreach_(Statement st, down.initial_sts().statements())
		up.addInitStatement(st);

	foreach_(Name n, down.types_ref()) {
		Option<Type::Type> t = down.tyTable_ref()[n];
		if (t) {
			up.types_ref().push_back(n);
			up.tyTable_ref().insert(n,t.get());
		}
	}

	foreach_(Name n, down.variables_ref()) {
		Option<VarInfo> t = down.syms_ref()[n];
		if (t) {
			VarInfo v = t.get();
			OptTypeDefinition otd = resolveType(down, v.type());
			if ( otd ) {
				typeDefinition td = otd.get();
				Type::Type t_final = get<1>(td);
				if ( is<Type::Class>(t_final)) {
					Type::Class tc = boost::get<Type::Class>(t_final);
					up.types_ref().push_back(tc.clase()->name());
					up.tyTable_ref().insert(tc.clase()->name(),t_final);
					v.set_type(tc.clase()->name());
				}
				ExpList indexes = get<2>(td);
				if (v.indices()) indexes += v.indices().get();
				if (indexes.size() > 0 ) v.set_indices(indexes);
				v.set_prefixes( v.prefixes() + get<0>(td) );

				if (is<Type::String>(t_final)) v.set_type("String");
				if (is<Type::Integer>(t_final)) v.set_type("Integer");
				if (is<Type::Real>(t_final)) v.set_type("Real");
				if (is<Type::Boolean>(t_final)) v.set_type("Boolean");
				up.variables_ref().push_back(n);
				up.syms_ref().insert(n,v);
			}

		} else std::cerr << "No encuentro la variable " << n << std::endl;
	}

	foreach_(Import i, down.imports_ref())
		up.imports_ref().push_back(i);

	foreach_(Extends i, down.extends_ref())
		up.extends_ref().push_back(i);
}

void ClassFinder::ExpandAll(MMO_Class &up)
{
	ExtendList exls = up.extends();
	up.extends_ref().clear();
	foreach_(Extends e,exls) {
		OptTypeDefinition oc = resolveType(up,e.name());
		if (oc) {
			typeDefinition td = oc.get();
			Type::Type t_final = get<1>(td);
			if ( is<Type::Class>(t_final)) {
				MMO_Class d = * (boost::get<Type::Class>(t_final).clase()) ;
				ExpandAll(d);
				if (e.modification())
					applyClassModification(up,d,e.modification().get());
				expand(up,d);

			} else {std::cerr << "Error expandiendo. " << e.name() << " no es del tipo Class" << std::endl; exit(-1);}
		} else {
			std::cerr << "Error expandiendo. Clase no encontrada: " << e.name() << std::endl;
			exit(-1);
		}
	}

}


#define newMMO(NEW,OLD)  MMO_Class * (NEW) = new MMO_Class(); *(NEW) = (OLD);

Option<typeContexTuple> ClassFinder::findTypeByName(MMO_Class &c,Name n) 
{
	Option<Type::Type> t = c.tyTable_ref()[n];
	newMMO(contex,c)
	if (t) return typeContexTuple(t.get(),*contex);
	else if (c.father()) {MMO_Class cc = * c.father(); return findTypeByName( cc , n ); }
	return Option<typeContexTuple>();
}

OptTypeDefinition ClassFinder::getFinalClass(MMO_Class &c, Type::Type t) 
{
	if (is<Type::TypeDef>(t)) {
		Type::TypeDef df = boost::get<Type::TypeDef>(t);
		OptTypeDefinition op = resolveType(c,df.definition());
		if (op) {
			typeDefinition td = op.get();
			Type::Type t_final = get<1>(td);
			if ( is<Type::Class>(t_final) && df.modification()) { 
				newMMO(d , * (boost::get<Type::Class>(t_final).clase())) ;
				ExpandAll(*d);
				applyClassModification(c,*d,df.modification().get());
				t_final = Type::Class(d->name(),d);
			}
			ExpList inds = get<2>(td);
			if (df.indices()) inds = df.indices().get() + inds;
			typeDefinition res = typeDefinition( get<0>(td) + df.type_prefixes() , t_final , inds );
			return OptTypeDefinition(res);	
		}
		else return OptTypeDefinition();
	}
	return OptTypeDefinition(typeDefinition( TypePrefixes() , t , ExpList() ));
}

// Tipo simple: A
OptTypeDefinition ClassFinder::resolveDependencies(MMO_Class &c,Name n)
{
	Option<typeContexTuple> op = findTypeByName(c,n);
	if (op) {
		MMO_Class & contex = get<1>(op.get());
		ExpandAll(contex);
		Type::Type t = get<0>(op.get());
		 return getFinalClass(contex,t);
	}
	else return OptTypeDefinition();
}

// Tipo entrada: Tipos compuestos A.B.C.D.E
OptTypeDefinition ClassFinder::resolveType(MMO_Class &c,Name t)
{
	IdentList ts;
	boost::split(ts,t,boost::is_any_of("."));
	int i = 1, size = ts.size();
	ExpList index;
	MMO_Class  contex = c;
	TypePrefixes tpre;
	Type::Type t_final;
	foreach_(Name name , ts){
		OptTypeDefinition otd = resolveDependencies(contex,name);
		if (otd) {
			typeDefinition td = otd.get();
			t_final = get<1>(td);
			index = get<2>(td);
			tpre = get<0>(td);
			if (!is<Type::Class>(t_final) &&  i != size && size > 1) {
				std::cerr << "Error: " << t_final << " no es de tipo clase " << std::endl;
				exit(-1);	
				return OptTypeDefinition();
			} else if (is<Type::Class>(t_final)) {
				Type::Class tc = boost::get<Type::Class>(t_final);
				contex = *tc.clase();
			} 
			i++;
		} else {
			std::cerr << "Error buscando el tipo " << name << " en " << t << std::endl;
			c.tyTable_ref().dump();
			exit(-1);
			return OptTypeDefinition();
		}	
	}
	return OptTypeDefinition(typeDefinition(tpre,t_final,index));
}


//----------------------------------------------------------------------
//--						APPLY MODIFICATION                        --
//----------------------------------------------------------------------


void ClassFinder::applyModification(MMO_Class &contex, MMO_Class &target, Modification m)
{
	if (is<ModClass>(m)) {
		ModClass mc = boost::get<ModClass>(m);
		applyClassModification(contex,target,mc.modification());
	} else return;
}

void ClassFinder::applyClassModification(MMO_Class &contex, MMO_Class &target, ClassModification m)
{
	ExpandAll(target);
	foreach_(Argument a , m)
		applyArgument(contex,target,a);
}

void removeArgument(ClassModification & args, Argument a)
{
	Name n;
	if (is<ElMod>(a)) n = get<ElMod>(a).name();
	else return;
	
	ClassModification  _args = args;
	foreach_(Argument aux, _args) {
		if (is<ElMod>(aux) && get<ElMod>(aux).name() == n) {
			ClassModification::iterator it = std::find(args.begin(),args.end(),aux);
			if (it != args.end()) 
				args.erase(it);	
		}	
	}
}

void ClassFinder::applyArgument(MMO_Class &contex, MMO_Class &target, Argument m)
{
	ReplArg aux;
	if (is<ElMod>(m) ) {
		ElMod mod = boost::get<ElMod>(m);
		Option<VarInfo> opv = target.syms_ref()[mod.name()];
		
		if (!mod.modification()) return;
		Modification mToApply = mod.modification().get();
		
		if (opv) {
			VarInfo v = opv.get();
			if (v.modification()) {
				Modification mm = v.modification().get();
				if (is<ModClass>(mm)) {
					ModClass mClass = boost::get<ModClass>(mm);
					if (is<ModAssign>(mToApply)) std::cout << "Modification incompatibles " << std::endl;
					else if (is<ModEq>(mToApply)) {
						ModEq mEq = boost::get<ModEq>(mToApply);
						OptExp opEx = OptExp(mEq.exp());
						mClass.set_exp(opEx);
					} else {
						ModClass mClass2 = boost::get<ModClass>(mToApply);
						if (mClass2.exp()) mClass.set_exp(mClass2.exp());
						
						foreach_(Argument a, mClass2.modification()) {
							removeArgument(mClass.modification_ref(), a);
							mClass.modification_ref().push_back(a);	
						}
					}	
					v.set_modification(Option<Modification>(mClass));
				} else if (is<ModEq>(mm)){
					if (is<ModEq>(mToApply)) {
						v.set_modification(Option<Modification>(mToApply)); 
					} else {
						ModEq emod = get<ModEq>(mm);
						ModClass mClass = boost::get<ModClass>(mToApply);
						mClass.set_exp(emod.exp());
						v.set_modification(Option<Modification>(mClass));     
					}
						
				}	
			} else v.set_modification(mod.modification()); // Caso: No habia modificaciones    
			
			//target.syms_ref().erase(mod.name());
			target.syms_ref().insert(mod.name(),v);
		} else { 
			std::cout << "Error no encuentro variable: " << mod.name() << " en clase " << target.name() << std::endl;
			exit(-1);
		}	
		return ;
	} else if (is<ElRepl>(m)) {
		ElRepl el = boost::get<ElRepl>(m);
		aux = el.argument();
	} else if (is<ElRedecl>(m)) {
		ElRedecl redecl = boost::get<ElRedecl>(m);
		if (is<ElRepl>(redecl.argument())) {
			ElRepl repl = boost::get<ElRepl>(redecl.argument());
			aux = repl.argument();
		} else 
			aux = boost::get<ReplArg>(redecl.argument());
	}
	if (is<ShortClass>(aux)) { // Redeclare of types
			ShortClass shClass = boost::get<ShortClass>(aux);
			if (shClass.enum_spec()) {  // Cambio de enum
				Type::EnumType tt = Type::EnumType(shClass.prefixes(),shClass.name(),shClass.enum_spec().get());
				//target.tyTable_ref().erase(shClass.name());
				target.tyTable_ref().insert(shClass.name(),tt);
			} else {				
				TypePrefixes preType;
				ExpList index;
				if (shClass.type_prefixes()) preType = shClass.type_prefixes().get();
				if (shClass.indices()) index = shClass.indices().get();
				
				OptTypeDefinition otd = resolveType(contex, shClass.derived().get());
				if ( otd ) {
					typeDefinition td = otd.get();
					Type::Type t_final = get<1>(td);
					if ( is<Type::Class>(t_final)) {				
						target.tyTable_ref().insert(shClass.derived().get(),t_final);
					}	
					preType += get<0>(td);
					index += get<2>(td);
					Option<ExpList> optIndex;
					if (index.size() > 0) optIndex = index;
					Type::TypeDef tt = Type::TypeDef(shClass.prefixes(),shClass.name(),preType,shClass.derived().get(),optIndex,shClass.modification());
					//target.tyTable_ref().erase(shClass.name());
					target.tyTable_ref().insert(shClass.name(),tt);
				} else std::cerr << "No se pudo definir el tipo" << std::endl;
			}
			
	} else if (is<Component1>(aux)) { // Redeclare of Variables
		Component1 comp = boost::get<Component1>(aux);	
		Option<VarInfo> opv = target.syms_ref()[comp.declaration().name()];
		if (!opv) {std::cerr << "Variable no encontrada " << comp.declaration().name() << std::endl; return;}
		VarInfo v = opv.get();
		
		OptTypeDefinition otd = resolveType(contex, comp.type());
		if ( otd ) {
			typeDefinition td = otd.get();
			Type::Type t_final = get<1>(td);
			if ( is<Type::Class>(t_final)) {				
				target.tyTable_ref().insert(comp.type(),t_final);
			}
			TypePrefixes preType = get<0>(td) + comp.prefixes() ;
			ExpList indexes =  comp.declaration().indices() ?  get<2>(td)  + comp.declaration().indices().get()  :  get<2>(td) ;
			Option<ExpList> OptIndices;
			if (indexes.size() > 0) OptIndices = indexes;
			
			v.set_modification(comp.declaration().modification());
			v.set_indices(OptIndices);
			v.set_prefixes(preType);
			v.set_type(comp.type());
			target.syms_ref().insert(comp.declaration().name(),v);
			
		} else std::cerr << "No se pudo definir el tipo" << std::endl;
		
	}
}




