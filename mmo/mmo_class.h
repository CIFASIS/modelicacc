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

#ifndef MMO_CLASS
#define MMO_CLASS


#include <ast/class.h>
#include <util/table.h>
#include <util/type.h>


namespace Modelica {
using namespace Modelica::AST;
struct MMO_Class;


struct MMO_Class {
  MMO_Class();
  MMO_Class(Class &c);
  member(Name,name);
  member(ClassPrefixes,prefixes);
  member(EquationSection, initial_eqs);
  member(EquationSection, equations);
  member(StatementSection, initial_sts);
  member(StatementSection, statements);
  
  member(Option<Annotation>,annotation);
  member(Option<Annotation>,external_annot);
  member(Option<String>,language);
  member(Option<External>,external);
  
  member(VarSymbolTable,syms);
  member(TypeSymbolTable,tyTable);
  member(std::vector<Name>, variables);
  member(std::vector<Name>, types);
  
  printable(MMO_Class);
  bool isConnector();	
  void insertElement(Element);
  
  void addEquation(Equation);
  void addInitEquation(Equation);
  void addStatement(Statement);
  void addInitStatement(Statement);
  
  member(ExtendList,extends);
  member(ImportList,imports);
  member(MMO_Class *,father);
  void addVar(Name n , VarInfo var);
  Option<VarInfo> getVar(Name n);
  bool isLocal(Name n);

  
};
}
#endif

