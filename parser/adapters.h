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

#include <ast/statement.h>
#include <ast/equation.h>
#include <ast/modification.h>
#include <ast/element.h>
#include <ast/class.h>

// Equations
BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::Equality,
    (Modelica::AST::Expression, left_)
    (Modelica::AST::Expression, right_)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::Connect,
    (Modelica::AST::Expression, left_)
    (Modelica::AST::Expression, right_)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::CallEq,
    (Modelica::AST::Name, name_)
    (Modelica::AST::ExpList , args_)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::IfEq,
    (Modelica::AST::Expression, c)
    (Modelica::AST::EquationList, els)
    (Modelica::AST::IfEq::ElseList, elsesif)
    (Modelica::AST::EquationList, elses)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::ForEq,
    (Modelica::AST::Indexes, r)
    (Modelica::AST::EquationList, els)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::WhenEq,
    (Modelica::AST::Expression, c)
    (Modelica::AST::EquationList, els)
    (Modelica::AST::WhenEq::ElseList , elsew)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::EquationSection,
    (bool, initial_)
    (Modelica::AST::EquationList, equations_)
)

// Statements
BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::CallSt,
    (Modelica::AST::OptExpList, out_)
    (Modelica::AST::Expression, n_)
    (Modelica::AST::ExpList, arg_)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::IfSt,
    (Modelica::AST::Expression, c)
    (Modelica::AST::StatementList, els)
    (Modelica::AST::IfSt::ElseList, elsesif)
    (Modelica::AST::StatementList, elses)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::ForSt,
    (Modelica::AST::Indexes, r)
    (Modelica::AST::StatementList, els)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::WhenSt,
    (Modelica::AST::Expression, c)
    (Modelica::AST::StatementList, els)
    (Modelica::AST::WhenSt::ElseList , elsew)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::WhileSt,
    (Modelica::AST::Expression, cond_)
    (Modelica::AST::StatementList, elements_)
)

/*BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::Assign,
    (Modelica::AST::Expression, left_)
    (Modelica::AST::OptExp, right_)
    (Modelica::AST::ExpList, rl_)
)*/

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::StatementSection,
    (bool, initial_)
    (Modelica::AST::StatementList, statements_)
)

// Expressions
BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::Index,
    (Modelica::AST::Name, name_)
    (Modelica::AST::OptExp, exp_)
)


BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::Bracket,
    (Modelica::AST::ExpListList, args_)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::Named,
    (Modelica::AST::Name, name_)
    (Modelica::AST::Expression, exp_)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::FunctionExp,
    (Modelica::AST::Name, name_)
    (Modelica::AST::ExpList, args_)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::Call,
    (Modelica::AST::Name, name_)
    (Modelica::AST::ExpList, args_)
)

// Modifications, comments and annotations
BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::ModClass,
    (Modelica::AST::ClassModification, modification_)
    (Modelica::AST::OptExp, exp_)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::Annotation, 
    (Modelica::AST::ClassModification, modification_)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::Comment, 
    (Modelica::AST::StringComment,st_comment_)
    (Option<Modelica::AST::Annotation>, annotation_)
)

// Elements and class declarations
BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::Declaration, 
    (Modelica::AST::Name, name_)
    (Option<Modelica::AST::ExpList>, indices_)
    (Option<Modelica::AST::Modification>, modification_)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::Enum, 
    (Modelica::AST::Name, name_)
    (Modelica::AST::Comment, comment_)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::Component, 
    (Modelica::AST::TypePrefixes, prefixes_)
    (Modelica::AST::Name, type_)
    (Option<Modelica::AST::ExpList>, indices_)
    (Modelica::AST::DeclList, declarations_)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::Component1, 
    (Modelica::AST::TypePrefixes, prefixes_)
    (Modelica::AST::Name, type_)
    (Modelica::AST::Declaration, declaration_)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::Extends, 
    (Modelica::AST::Name, name_)
    (Option<Modelica::AST::ClassModification>, modification_)
    (Option<Modelica::AST::Annotation>, annotation_)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::ElMod,
    (Modelica::AST::Name, name_)
    (Option<Modelica::AST::Modification>, modification_)
    (Modelica::AST::StringComment, st_comment_)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::Constrained,
    (Modelica::AST::Name, name_)
    (Option<Modelica::AST::ClassModification>, modification_)
)

// Clases
/*
BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::Composition, 
    (Modelica::AST::ElemList, elements_)
    (Modelica::AST::CompElemList, comp_elem_)
    (Option<Modelica::AST::String>, language_)
    (Option<Modelica::AST::External>, call)
    (Option<Modelica::AST::Annotation>, ext_annot)
    (Option<Modelica::AST::Annotation>, annotation_)
)
*/

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::External, 
    (Modelica::AST::OptExp, comp_ref_)
    (Modelica::AST::Name, fun_)
    (Modelica::AST::OptExpList, args_)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::Class, 
    (Modelica::AST::Name, name_)
    (Modelica::AST::StringComment, st_comment_)
    (Modelica::AST::Composition, composition_)
    //(Modelica::AST::Name, end_name_)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::DefClass, 
    (Modelica::AST::Name, name_)
    (Modelica::AST::TypePrefixes,type_prefixes_)
    (Modelica::AST::Name, definition_)
    (Option<Modelica::AST::ExpList>, indices_)
    (Option<Modelica::AST::ClassModification>, modification_)
    (Modelica::AST::Comment, comment_)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::DerClass, 
    (Modelica::AST::Name, name_)
    (Modelica::AST::Name, deriv_)
    (Modelica::AST::IdentList, ident_list_)
    (Modelica::AST::Comment, comment_)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::EnumClass, 
    (Modelica::AST::Name, name_)
    (Modelica::AST::EnumSpec, enum_spec_)
    (Modelica::AST::Comment, comment_)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::ExtendsClass, 
    (Modelica::AST::Name, name_)
    (Option<Modelica::AST::ClassModification>, modification_)
    (Modelica::AST::StringComment, st_comment_)
    (Modelica::AST::Composition, composition_)
)


/*
BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::StoredDef, 
    (Option<bool>, within_)
    (Option<Option<Modelica::AST::Name> >, name_)
    (Modelica::AST::ClassList, classes_)
)
*/
