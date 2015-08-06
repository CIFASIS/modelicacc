/*
 * collect_unknowns.cpp
 *
 *  Created on: 29/04/2013
 *      Author: fede
 */

#include <ast/queries.h>
#include <causalize/unknowns_collector.h>
#include <util/ast_visitors/evalexp.h>
#include <util/table.h>
#include <util/type.h>
#include <util/debug.h>
#include <iostream>

UnknownsCollector::UnknownsCollector(MMO_Class &c): _c(c), _finder(c) {

}


ExpList UnknownsCollector::collectUnknowns() {
  ExpList _unknowns;
  _finder.findStateVariables();
  foreach_(VarSymbolTable::table_type::value_type val, _c.syms_ref()) {
    VarInfo varInfo = val.second;
    Name name = val.first;
		if (!varInfo.builtin() && !isConstant(name,_c.syms_ref()) && !isDiscrete(name, _c.syms_ref()) && !isParameter(name,_c.syms_ref())) {
      Option<Type::Type> opt_type =  _c.tyTable_ref()[varInfo.type()];
      ERROR_UNLESS(opt_type, "No %s type found", varInfo.type().c_str());
      Type::Type type = opt_type.get();
      if (is<Type::Real>(type)) {
        if (varInfo.state()) {
            if (!varInfo.indices())
              _unknowns.push_back(Call("der",ExpList(1,Reference(Ref(1,RefTuple(name,ExpList(0)))))));
            else if (varInfo.indices().get().size()==1) {
              EvalExp ev(_c.syms_ref());
              Expression lim=varInfo.indices().get().front();
              const int limit=boost::apply_visitor(ev,lim);
              for (int i=1;i<=limit;i++)
                _unknowns.push_back(Call("der",ExpList(1,Reference(Ref(1,RefTuple(name,ExpList(1,Integer(i))))))));
            } else {
              ERROR("Multidimensional variables not supported yet");
            } 
        } else {
            if (!varInfo.indices())
              _unknowns.push_back(Reference(Ref(1,RefTuple(name,ExpList(0)))));
            else if (varInfo.indices().get().size()==1) {
              EvalExp ev(_c.syms_ref());
              Expression lim=varInfo.indices().get().front();
              const int limit=boost::apply_visitor(ev,lim);
              for (int i=1;i<=limit;i++) 
                _unknowns.push_back(Reference(Ref(1,RefTuple(name,ExpList(1,Integer(i))))));
            } else {
              ERROR("Multidimensional variables not supported yet");
            }
        }

      } else if (is<Type::Array>(type)) {
        ERROR("No vectorial support yet!");
      }
    } 
  }
	return _unknowns;
}


