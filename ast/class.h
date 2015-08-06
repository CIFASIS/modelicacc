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


#ifndef AST_CLASS
#define AST_CLASS
#include <ast/equation.h>
#include <ast/statement.h>
#include <ast/element.h>
#include <ast/expression.h>

namespace Modelica {
  namespace AST {
  
  struct External {
    member(OptExp,comp_ref);
    member(Name,fun);
    member(OptExpList,args);
  };
  
  struct Public: public ElemList {
    Public() {};
    Public(ElemList l): ElemList(l) {};
    printable(Public);
  };

  struct Protected: public ElemList {
    Protected() {};
    Protected(ElemList l): ElemList(l) {};
    printable(Protected);
  };
  typedef boost::variant<
    EquationSection,
    StatementSection,
    Public,
    Protected
  > CompElement;

  typedef std::vector<CompElement> CompElemList;

  template<typename T> 
  inline bool is(CompElement c) {
    return c.type()==typeid(T);
  }

  struct Composition {
    Composition(){};
    Composition(ElemList, CompElemList ,boost::optional<boost::fusion::vector3<boost::optional<String>, boost::optional<External>, boost::optional<Annotation> > >);
    member(ElemList,elements);
    member(CompElemList,comp_elem);
    member(bool,external);
    member(Option<String>,language);
    member(Option<External>,call);
    member(Option<Annotation>,ext_annot);
    member(Option<Annotation>,annotation);
  } ;

  struct ExtendsClass {
    member(Name, name);
    member(Option<ClassModification>, modification);
    member(StringComment, st_comment);
    member(Composition, composition);
    member(ClassPrefixes,prefixes);
    member(bool,encapsulated);
    member(bool,final);
    printable(ExtendsClass);
  };

  struct DerClass {
    member(Name, name);
    member(Name, deriv);
    member(IdentList, ident_list);
    member(Comment, comment);
    member(ClassPrefixes,prefixes);
    member(bool,encapsulated);
    member(bool,final);
    printable(DerClass);
  };

  struct EnumClass {
    member(Name, name);
    member(EnumSpec, enum_spec);
    member(Comment, comment);
    member(ClassPrefixes,prefixes);
    member(bool,encapsulated);
    member(bool,final);
    printable(EnumClass);
  };

  struct DefClass {
    member(Name,name);
    member(TypePrefixes,type_prefixes);
    member(Name,definition);
    member(Option<ExpList>,indices);
    member(Option<ClassModification>,modification);
    member(Comment,comment);
    printable(DefClass);
    member(ClassPrefixes,prefixes);
    member(bool,encapsulated);
    member(bool,final);
  };
  struct Class {
    Class(): final_(false), encapsulated_(false) {}
    member(Name,name);
    member(StringComment,st_comment);
    member(Composition, composition);
    member(Name,end_name);
    member(ClassPrefixes,prefixes);
    member(bool,encapsulated);
    member(bool,final);
    printable(Class);
 };
  typedef boost::variant<
    Class,
    DefClass,
    EnumClass,
    DerClass,
    ExtendsClass
  > ClassType;
  
  struct ClassType_ {
    ClassType_(){};
    ClassType_(ClassType);
    member(ClassType, cl);
  };

  template<typename T> 
  bool is(ClassType c) {
    return c.type()==typeid(T);
  }

  typedef std::vector<ClassType> ClassList;

  struct StoredDef {
    StoredDef(){};
    StoredDef(boost::optional<Name> name, ClassList cl);
    StoredDef(ClassList cl);
    member(bool,within);
    member(Option<Name>,name);
    member(ClassList,classes);
    printable(StoredDef);
  };

  Name className(ClassType c);

  }
}
#endif
