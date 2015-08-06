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


#ifndef AST_ELEMENT
#define AST_ELEMENT 
#include <ast/modification.h>

namespace Modelica {
  namespace AST {
  
  struct Extends {
    member(Name,name);
    member(Option<ClassModification>,modification);
    member(Option<Annotation>,annotation);
    printable(Extends);
  };

  struct Import {
    Import(){};
    Import(Name, Name, Comment) {}
    Import(Name, boost::optional<boost::optional<IdentList> >, Comment) {}
    printable(Import);
    
  };

  struct ClassType_;  // For recursion
  struct ElemClass { 
    ElemClass(){}
    ElemClass(ClassType_);
    member(boost::recursive_wrapper<ClassType_>, class_element);
    member(bool, replaceable);
    member(bool, redeclare);
    member(bool, final);
    member(bool, inner);
    member(bool, outer);
    member(Option<Constrained>, constrained);
    member(Option<Comment>, constrained_comment);
    printable(ElemClass);
  };
  typedef boost::variant<
    Import,
    Component,
    Extends,
    ElemClass
  > Element;
  
  template<typename T> 
  bool is(const Element & e) {
    return e.type()==typeid(T);
  }
  typedef std::vector<Element> ElemList;
  typedef std::vector<Extends> ExtendList;
  typedef std::vector<Import> ImportList;
  }
}
#endif
