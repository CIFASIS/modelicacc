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

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <parser/adapters.h>
#include <parser/actions.h>

namespace Modelica {
namespace parser {
  namespace qi = boost::spirit::qi;
  namespace ascii = boost::spirit::ascii;
  using namespace Modelica::AST;

  template <typename Iterator>
  struct parser: qi::grammar<Iterator, skipper<Iterator>, StoredDef() >
  {
   parser(Iterator &it) : parser::base_type(stored_definition), 
      OPAREN("("),CPAREN(")"), COLON(":"), SEMICOLON(";"),  QUOTE("\""), 
      STAR("*"), OBRACKET("["), CBRACKET("]"), COMA(","), OBRACE("{"), 
      CBRACE("}"), EQUAL("="), PLUS("+"), ASSIGN(":="), DOT(qi::char_('.')), 
      DER(qi::string("der")), INITIAL(qi::string("initial")),
      ALGORITHM("algorithm"), AND("and"), ANNOTATION("annotation"), 
      CONNECT("connect"), CONSTRAINEDBY("constrainedby"), DER2("der"), EACH("each"), ELSE("else"),
      ELSEIF("elseif"), ELSEWHEN("elsewhen"), ENCAPSULATED("encapsulated"), 
      END("end"), ENUMERATION("enumeration"), EQUATION("equation"),
      EXTENDS("extends"), EXTERNAL("external"), 
      FALSE("false"), FINAL("final"), FOR("for"), 
      FUNCTION1("function"), IF("if"), IMPORT("import"), 
      IN("in"),  INNER("inner"), LOOP("loop"), NOT("not"), OR("or"), 
      OUTER("outer"), PROTECTED("protected"), PUBLIC("public"), REDECLARE("redeclare"), 
      REPLACEABLE("replaceable"), THEN("then"), TRUE("true"), 
      WHEN("when"), WHILE("while"), WITHIN("within"), it(it)
   {
      using namespace qi::labels;
      using phoenix::bind;
      using phoenix::construct;
      using qi::char_;
      using qi::alpha;
      using qi::lexeme;
      using qi::alnum;
      using qi::fail;
      using qi::on_error;
      using qi::omit;
      using qi::_val;
      using qi::_1;
      using qi::_2;
      using qi::_3;
      using qi::_4;
      using qi::_5;
      using qi::_6;
      using qi::_7;
      using qi::matches;

      stored_definition = 
                          (WITHIN >> (-name) > SEMICOLON > class_list) [_val=construct<StoredDef>(_1,_2) ]
                        | class_list [_val=construct<StoredDef>(_1) ]
                        ;

      class_list =
                   +(class_def > SEMICOLON) 
                 ;
  
      class_def =
                   (matches[FINAL] >> class_definition) [_val=bind(&add_final,_2,_1)]
                ;

      class_definition = 
                         (matches[ENCAPSULATED] >> class_prefixes > class_specifier) [_val=bind(&add_enc_prefixes,_1,_2)]
                       ;
      
      class_prefixes = 
                      (-PARTIAL) >>
                       (  CLASS 
                        | MODEL 
                        | ((-OPERATOR) >> RECORD) 
                        | BLOCK 
                        | ((-EXPANDABLE) >> CONNECTOR) 
                        | TYPE 
                        | PACKAGE 
                        | (-(PURE | IMPURE)) >> (-OPERATOR) >> FUNCTION
                        | OPERATOR
                       )
                   ;

      class_  =
                ident >> string_comment >> composition >> END > omit[ident]
              ;

      def_class = 
                  ident >> EQUAL >> type_prefix >> name >> (-array_subscripts) 
                  >> (-class_modification) >> comment
                ;

      enum_class = 
                   ident >> EQUAL >> ENUMERATION > OPAREN > enum_spec > CPAREN >> comment
                 ;

      deriv_class =
                    ident >> EQUAL >> DER2 > OPAREN > name > COMA > (ident % COMA) > CPAREN > comment
                  ;

      extends_class =
                      EXTENDS > ident >> (-class_modification) >> string_comment >> composition > END > omit[ident]
                    ;

      class_specifier =
                        class_
                      | def_class
                      | enum_class
                      | deriv_class
                      | extends_class
                      ;

      composition = 
                    (element_list >> (*comp_element) 
                        >> (-(EXTERNAL >> (-string_) >> (-external_function_call) >> (-annotation) >> SEMICOLON))
                    ) [_val=construct<Composition>(_1,_2,_3)] >> (-((annotation [_val=bind(&add_annot,_val,_1)]>> SEMICOLON) ))
                  ;

      comp_element =
                     equation_section   [_val=_1]
                   | algorithm_section  [_val=_1]
                   | PUBLIC >> element_list [_val=construct<Public>(_1)]
                   | PROTECTED >> element_list [_val=construct<Protected>(_1)]
                   ;


      external_function_call =
                               (-(component_reference >> EQUAL)) >> ident >> OPAREN >> (-expression_list) >> CPAREN
                             ;

      element_list = 
                      *(element > SEMICOLON)
                   ;

      element = 
                import_clause [_val=_1]
              | extends_clause [_val=_1]
              | (matches[REDECLARE] >> matches[FINAL] >> matches[INNER] >> matches[OUTER] >> element2) [_val=bind(&class_comp,_1,_2,_3,_4, _5)]
              ;

      element2 = 
                 (component_clause [_val=_1] | class_definition [_val=construct<ElemClass>(construct<ClassType_>(_1))])
               | (REPLACEABLE >> (class_definition | component_clause) >> -(constraining_clause >> comment)) [_val=bind(&element2_act,_1,_2)]
               ;

      import_clause = 
                      (IMPORT >> ident >> EQUAL >> name >> comment) [_val=construct<Import>(_1,_2,_3)]
                    | (IMPORT >> name >> (-(omit[DOT] >> (STAR | (OBRACE >> (ident % COMA) >> CBRACE)))) >> comment) [_val=construct<Import>(_1,_2,_3)]
                    ;

      extends_clause =
                      EXTENDS > name >> (-class_modification) >> (-annotation)
                    ;

      component_clause = 
                         type_prefix >> name >> (-array_subscripts) >> component_list
                       ;
    
      type_prefix = 
                    (-(FLOW | STREAM)) >> (-(DISCRETE | PARAMETER | CONSTANT)) >> (-(INPUT | OUTPUT))
                  ;

      component_list =
                       component_declaration % COMA
                     ;
  
      component_declaration = 
                              (declaration >> (-condition_attribute) >> comment)
                               [_val=bind(&add_cond,_1,_2,_3)]
                            ;
  
      condition_attribute =
                            IF > expression
                          ;
      
      declaration =
                    ident >> (-array_subscripts) >> (-modification)
                  ;

      modification = 
                     equal_mod
                   | assign_mod 
                   | class_mod
                   ;

      class_mod = 
                  class_modification >> -(EQUAL > expression)
                ;

      equal_mod =
                  EQUAL > expression
                ;
      
      assign_mod =
                   ASSIGN > expression
                 ;
    
      class_modification = 
                           OPAREN > (-(argument % COMA)) > CPAREN 
                         ;
    
      argument = 
                 element_redeclaration [_val=_1]
               | (matches[EACH] >> matches[FINAL] >> (element_modification | element_replaceable)) [_val=bind(&add_each_final_rep,_1,_2,_3)]
               ;

      element_modification =
                              name >> (-modification) >> string_comment
                           ; 
                                           
      element_replaceable =
                            (REPLACEABLE >> (short_class_definition | component_clause1) >> (-constraining_clause)) [_val=construct<ElRepl>(_1,_2)]
                          ; 
                                           
      element_redeclaration =
                              (REDECLARE >> matches[EACH] >> matches[FINAL] >> ((short_class_definition | component_clause1) | element_replaceable))
                              [_val=construct<ElRedecl>(_1,_2,_3)]
                           ; 

      component_clause1 =
                          type_prefix >> name >> component_declaration1 
                        ;

      component_declaration1 =
                               (declaration >> comment) [_val=bind(&add_comment<Declaration>,_1,_2)]
                             ;

      // TODO: return something meaningful
      short_class_definition =
                               (class_prefixes >> ident >> EQUAL >> type_prefix >> name >> (-array_subscripts) >> (-class_modification) >> comment) [_val=construct<ShortClass>(_1,_2,_3,_4,_5,_6,_7)]
                             | (class_prefixes >> ident >> EQUAL >> ENUMERATION >> OPAREN >> enum_spec >> CPAREN >>  comment)  [_val=ShortClass()]
                             ;
      
      enum_list = 
                  enumeration_literal % COMA 
                ;
      
      enum_spec = 
                  enum_list
                | COLON
                ;

      enumeration_literal =
                            ident > comment
                          ;

      constraining_clause = 
                            CONSTRAINEDBY > name > (-class_modification)
                          ;

      equation_section =
                          matches[INITIAL] >> EQUATION >> equation_list
                       ;

      equality_equation = 
                            (simple_expression >> EQUAL > expression)
                        ;

      call_equation = 
                       name >> function_call_args
                    ;

      connect_equation = 
                         CONNECT > OPAREN > component_reference > COMA > component_reference > CPAREN
                       ;

      equation = (
                    equality_equation
                  | for_equation
                  | if_equation
                  | connect_equation
                  | when_equation
                  | call_equation
                ) > comment 
               ;
  
      equation_list =
                      *(equation > SEMICOLON)
                    ;

      when_equation = 
                      WHEN > expression > THEN > equation_list > *(ELSEWHEN > expression > THEN > equation_list) > END > WHEN
                     ;

      for_equation  = 
                      FOR > for_indices > LOOP > equation_list > END > FOR
                    ;
   
      if_equation = 
                    IF > expression > THEN > equation_list > *(ELSEIF > expression > THEN > equation_list) > -(ELSE > equation_list) > END > IF
                  ;
 
      algorithm_section = 
                           matches[INITIAL] >> ALGORITHM >> statement_list
                        ;

      statement_list =  
                        *(statement > SEMICOLON)
                      ;

      assign_statement =
                         (component_reference >> ASSIGN > expression) [_val=construct<Assign>(_1,_2)]
                       | (component_reference >> function_call_args) [_val=construct<Assign>(_1,_2)]
                       |  component_reference [_val=construct<Assign>(_1)]
                       ;

      call_statement =
                          (OPAREN > output_expression_list > CPAREN > ASSIGN > component_reference > function_call_args)
                      ;

      statement = 
                  (
                      assign_statement
                    | call_statement
                    | ret_break_st
                    | if_statement
                    | for_statement
                    | when_statement
                    | while_statement
                  ) >> comment
                ;

      if_statement = 
                     IF > expression > THEN > statement_list > *(ELSEIF > expression > THEN > statement_list) > -(ELSE > statement_list) > END > IF
                   ;

      for_statement = 
                      FOR > for_indices > LOOP > statement_list > END > FOR
                    ;

      when_statement = 
                       WHEN > expression > THEN > statement_list > *(ELSEWHEN > expression > THEN > statement_list) > END > WHEN;
                     ;

      while_statement = 
                        WHILE > expression > LOOP > statement_list > END > WHILE
                      ;

      for_indices =
                     (for_index % COMA) [_val=construct<Indexes>(_1)]
                  ;

      for_index = 
                  ident >> -(IN > expression) 
                ;
 
      expression = 
                   simple_expression 
                 | if_expression
                 ;

      if_expression = 
                      (IF > expression > THEN > expression > *(ELSEIF > expression > THEN > expression ) > ELSE > expression)
                      [_val=construct<IfExp>(_1, _2, _3, _4)]
                    ;

      simple_expression = 
                          logical_expression [_val=_1] >> (-(COLON > logical_expression [_val=bind(range1,_val,_1)])) 
                          >> (-(COLON > logical_expression [_val=bind(range2,_val,_1)])) 
                        ;

      logical_expression = 
                           logical_term [_val=_1] >> *(or_op > logical_term) [_val=construct<BinOp>(_val, _1, _2)]
                         ;

      logical_term = 
                     logical_factor [_val=_1] >> *(and_op > logical_factor) [_val=construct<BinOp>(_val, _1, _2)]
                   ;

      logical_factor =  
                       (not_op > relation) [_val=construct<UnaryOp>(_2,_1)]
                     | relation [_val=_1]
                     ;

      relation =  
                 arithmetic_expression [_val= _1] >> -(rel_op > arithmetic_expression) [_val=construct<BinOp>(_val, _1, _2)]
               ;

      arithmetic_expression = 
                              term [_val=_1] >> *(add_op > term) [_val=construct<BinOp>(_val, _1, _2)]
                            | (unary_op > term) [_val=construct<UnaryOp>(_2,_1)] >> *(add_op > term) [_val=construct<BinOp>(_val, _1, _2)]
                            ;

      term =
             factor [_val=_1] >> *(mul_op > factor) [_val=construct<BinOp>(_val, _1, _2)]
           ;

      factor =
               primary [_val=_1] >> -(exp_op > primary) [_val=construct<BinOp>(_val, _1, _2)]
             ;
    
      primary =
                lexeme[modelica_double] 
              | lexeme[modelica_int]
              | lexeme[string_]
              | boolean 
              | call_exp 
              | component_reference 
              | output_exp
              | bracket_exp
              | brace_exp
              | end_exp 
              ;
  
      call_exp =
                 (name  | DER | INITIAL) >> function_call_args 
               ;

      bracket_exp =
                    OBRACKET > (expression_list % SEMICOLON)  > CBRACKET
                  ;

      output_exp =
                   OPAREN > output_expression_list > CPAREN
                 ;

      brace_exp =
                  OBRACE > function_arguments > CBRACE
                ;

      expression_list = 
                        expression % COMA
                      ;

      name =
             (-DOT) >> ident [_val+=_1] >> *(DOT >> ident) [_val+=_1+_2]
           ;

      keyword_ident = 
                      keywords [_val=_1] >> +(alnum | char_('_')) [_val+=_1]
                    ;

      ident = 
              lexeme[((char_('_') | alpha) >> *(alnum | char_('_'))) - keywords]
            | lexeme[keyword_ident]
            | lexeme[ char_('\'') >> *(alnum | char_('_') | quoted_chars) > char_('\'') ] 
            ;

      string_ = 
                (QUOTE >> *("\\\"" | (char_ - QUOTE)) > QUOTE) [_val=construct<String>(_1)]
              ;

      component_reference = ((-DOT) >> ident >> (-array_subscripts)) [_val=construct<Reference>(_1,_2,_3)]
       >> *(DOT >> ident >> (-array_subscripts)) [ _val = construct<Reference>(_val,_2,_3) ]
                          ;

      array_subscripts = 
                           OBRACKET > (subscript % COMA) > CBRACKET 
                        ;
      subscript = 
                  colon_exp 
                | expression
                ;

      function_call_args = 
                           OPAREN > (-function_arguments) > CPAREN 
                         ;
      
      // TODO: forexp is missing
      function_arguments = 
                           named_arguments [_val=_1]
                         | (function_argument [_val=bind(&consume_one<Expression>,_1)] >> 
                            (-((COMA > function_arguments [ _val = bind(&append_list<Expression>, _val, _1) ]) 
                              | (FOR > for_indices)  )))
                         ;

      named_arguments =
                        named_argument % COMA
                      ;

      named_argument =  
                        ident >> EQUAL >> function_argument 
                     ;

      function_argument = 
                           function_exp
                        |  expression 
                        ;

      function_exp =
                     FUNCTION1 > name > OPAREN >> (-named_arguments) > CPAREN
                   ;

      output_expression_list = 
                              (-expression) % COMA
                             ;

      comment = 
                string_comment >> (-annotation)
              ;

      // TODO add the + string
      string_comment =
                       -(string_ [_val=construct<StringList>(1,_1)] >> *(PLUS > string_) [_val=StringComment()])
                     ;

      annotation =  
                    ANNOTATION > class_modification
                 ;


      /* Error and debug */
      on_error<fail>
        (
            stored_definition
          , std::cerr
                << phoenix::val("Parser error. Expecting ")
                << _4                               // what failed?
                << bind(&at,_3,it)
                << std::endl
        );
 
      OPAREN.name("(");
      CPAREN.name(")");
      COLON.name(":");
      SEMICOLON.name(";");
      QUOTE.name("\""); 
      STAR.name("*");
      OBRACKET.name("[");
      CBRACKET.name("]");
      COMA.name(",");
      OBRACE.name("{");
      CBRACE.name("}");
      EQUAL.name("=");
      PLUS.name("+"); 
      ASSIGN.name(":="); 
      algorithm_section.name("algorithm_section");
      annotation.name("annotation");
      argument.name("argument");
      arithmetic_expression.name("arithmetic_expression");
      array_subscripts.name("array_subscripts");
      assign_mod.name("modification");
      class_list.name("class_list");
      class_def.name("class_definition");
      class_definition.name("class_definition");
      class_modification.name("class_modification");
      class_mod.name("class_modification");
      class_prefixes.name("class_prefixes");
      class_.name("class");
      class_specifier.name("class_specifier");
      comment.name("comment");
      component_clause1.name("component_clause1");
      component_clause.name("component_clause");
      component_declaration1.name("component_declaration1");
      component_declaration.name("component_declaration");
      component_list.name("component_list");
      component_reference.name("component_reference");
      composition.name("composition");
      condition_attribute.name("condition_attribute");
      constraining_clause.name("constraining_clause");
      declaration.name("declaration");
      element_list.name("element_list");
      element_modification.name("element_modification");
      element.name("element");
      element_redeclaration.name("element_redeclaration");
      element_replaceable.name("element_replaceable");
      enumeration_literal.name("enumeration literal");
      enum_list.name("enum list");
      equation_list.name("equation_list");
      equal_mod.name("modification");
      equation.name("equation");
      equation_section.name("equation_section");
      expression_list.name("expression list");
      expression.name("expression");
      extends_clause.name("extends_clause");
      external_function_call.name("external_function_call");
      factor.name("factor");
      for_equation.name("for equation");
      for_index.name("for index");
      for_indices.name("for indices");
      for_statement.name("for statement");
      function_argument.name("function_argument");
      function_arguments.name("function_arguments");
      function_call_args.name("function_call_args");
      ident.name("identifier");
      if_equation.name("if equation");
      if_statement.name("if statement");
      import_clause.name("import_clause");
      logical_expression.name("logical_expression");
      logical_factor.name("logical_factor");
      logical_term.name("logical_term");
      modification.name("modification");
      named_argument.name("named_argument");
      named_arguments.name("named_arguments");
      name.name("name");
      output_expression_list.name("output_expression_list");
      primary.name("primary");
      relation.name("relation");
      short_class_definition.name("short_class_definition");
      simple_expression.name("expression");
      statement_list.name("statement_list");
      statement.name("statement");
      stored_definition.name("stored_definition");
      string_comment.name("string_comment");
      string_.name("string");
      subscript.name("subscript");
      term.name("term");
      type_prefix.name("type_prefix");
      when_equation.name("when_equation");
      when_statement.name("when_statement");
      while_statement.name("while_statement");

#ifdef MCC_DEBUG
      debug(algorithm_section);
      debug(annotation);
      debug(argument);
      debug(arithmetic_expression);
      debug(array_subscripts);
      debug(class_modification);
      debug(class_prefixes);
      debug(class_specifier);
      debug(comment);
      debug(component_clause);
      debug(component_clause1);
      debug(component_declaration);
      debug(component_declaration1);
      debug(component_list);
      debug(component_reference);
      debug(composition);
      debug(condition_attribute);
      debug(constraining_clause);
      debug(declaration);
      debug(element);
      debug(element_list);
      debug(element_modification);
      debug(element_redeclaration);
      debug(element_replaceable);
      debug(enumeration_literal);
      debug(enum_list);
      debug(equation);
      debug(equation_list);
      debug(equation_section);
      debug(expression);
      debug(expression_list);
      debug(extends_clause);
      debug(external_function_call);
      debug(factor);
      debug(for_equation);
      debug(for_index);
      debug(for_indices);
      debug(for_statement);
      debug(function_argument);
      debug(function_arguments);
      debug(function_call_args);
      debug(ident);
      debug(if_equation);
      debug(if_statement);
      debug(import_clause);
      debug(logical_expression);
      debug(logical_factor);
      debug(logical_term);
      debug(modification);
      debug(name);
      debug(named_argument);
      debug(named_arguments);
      debug(output_expression_list);
      debug(primary);
      debug(relation);
      debug(short_class_definition);
      debug(simple_expression);
      debug(statement);
      debug(statement_list);
      debug(string_);
      debug(string_comment);
      debug(stored_definition);
      debug(subscript);
      debug(term);
      debug(type_prefix);
      debug(when_equation);
      debug(when_statement);
      debug(while_statement);
#endif
      }
      
      // Expressions
      qi::rule<Iterator, skipper<Iterator>, Expression()> 
          primary, factor, term, arithmetic_expression, relation,
          logical_expression,logical_term, logical_factor, simple_expression, 
          expression, subscript,function_argument, condition_attribute;
      qi::rule<Iterator, skipper<Iterator>, Reference()> component_reference;
      qi::rule<Iterator, skipper<Iterator>, FunctionExp()> function_exp;
      qi::rule<Iterator, skipper<Iterator>, Named()> named_argument;
      qi::rule<Iterator, skipper<Iterator>, Brace()> brace_exp;
      qi::rule<Iterator, skipper<Iterator>, Bracket()> bracket_exp;
      qi::rule<Iterator, skipper<Iterator>, Call()> call_exp;
      qi::rule<Iterator, skipper<Iterator>, Output()> output_exp;
      qi::rule<Iterator, skipper<Iterator>, Index()> for_index;
      qi::rule<Iterator, skipper<Iterator>, IfExp()> if_expression;
      qi::rule<Iterator, skipper<Iterator>, Indexes()> for_indices;
      qi::rule<Iterator, skipper<Iterator>, ExpList()> 
          function_call_args, function_arguments, named_arguments, 
          expression_list, array_subscripts;
      qi::rule<Iterator, skipper<Iterator>, OptExpList()> output_expression_list;
      
      // Equations
      qi::rule<Iterator, skipper<Iterator>, Equation()> equation;
      qi::rule<Iterator, skipper<Iterator>, CallEq()> call_equation;
      qi::rule<Iterator, skipper<Iterator>, Connect()> connect_equation;
      qi::rule<Iterator, skipper<Iterator>, Equality()> equality_equation;
      qi::rule<Iterator, skipper<Iterator>, IfEq()> if_equation;
      qi::rule<Iterator, skipper<Iterator>, ForEq()> for_equation;
      qi::rule<Iterator, skipper<Iterator>, WhenEq()> when_equation;
      qi::rule<Iterator, skipper<Iterator>, EquationList() > equation_list;
      qi::rule<Iterator, skipper<Iterator>, EquationSection()> equation_section;

      // Statements
      qi::rule<Iterator, skipper<Iterator>, Statement() > statement;
      qi::rule<Iterator, skipper<Iterator>, Assign() > assign_statement;
      qi::rule<Iterator, skipper<Iterator>, CallSt() > call_statement;
      qi::rule<Iterator, skipper<Iterator>, ForSt() > for_statement;
      qi::rule<Iterator, skipper<Iterator>, WhenSt() > when_statement;
      qi::rule<Iterator, skipper<Iterator>, WhileSt() > while_statement;
      qi::rule<Iterator, skipper<Iterator>, IfSt() > if_statement;
      qi::rule<Iterator, skipper<Iterator>, StatementList() > statement_list;
      qi::rule<Iterator, skipper<Iterator>, StatementSection() > algorithm_section;
      
      // Modifications, Comments and annotations
      qi::rule<Iterator, skipper<Iterator>, Enum()> enumeration_literal;
      qi::rule<Iterator, skipper<Iterator>, EnumList()> enum_list;
      qi::rule<Iterator, skipper<Iterator>, EnumSpec()> enum_spec;
      qi::rule<Iterator, skipper<Iterator>, StringComment()> string_comment;
      qi::rule<Iterator, skipper<Iterator>, Modification()> modification;
      qi::rule<Iterator, skipper<Iterator>, ModEq()> equal_mod;
      qi::rule<Iterator, skipper<Iterator>, ModAssign()> assign_mod;
      qi::rule<Iterator, skipper<Iterator>, ModClass()> class_mod;
      qi::rule<Iterator, skipper<Iterator>, ClassModification()> class_modification;
      qi::rule<Iterator, skipper<Iterator>, Argument()> argument;
      qi::rule<Iterator, skipper<Iterator>, Annotation()> annotation;
      qi::rule<Iterator, skipper<Iterator>, Comment()> comment;

      // Elements and class declarations
      qi::rule<Iterator, skipper<Iterator>, Declaration()> declaration, component_declaration1, component_declaration;
      qi::rule<Iterator, skipper<Iterator>, DeclList()> component_list;
      qi::rule<Iterator, skipper<Iterator>, Component()> component_clause;
      qi::rule<Iterator, skipper<Iterator>, Component1()> component_clause1;
      qi::rule<Iterator, skipper<Iterator>, Extends()> extends_clause;
      qi::rule<Iterator, skipper<Iterator>, Import()> import_clause;
      qi::rule<Iterator, skipper<Iterator>, Element()> element, element2; 
      qi::rule<Iterator, skipper<Iterator>, ElemList()> element_list; 
      qi::rule<Iterator, skipper<Iterator>, ElMod()> element_modification; 
      qi::rule<Iterator, skipper<Iterator>, Constrained()> constraining_clause; 
      qi::rule<Iterator, skipper<Iterator>, ElRepl()> element_replaceable; 
      qi::rule<Iterator, skipper<Iterator>, ElRedecl()> element_redeclaration;

      // Classes 
      qi::rule<Iterator, skipper<Iterator>, CompElement()> comp_element;
      qi::rule<Iterator, skipper<Iterator>, Composition()> composition;
      qi::rule<Iterator, skipper<Iterator>, External()> external_function_call;
      qi::rule<Iterator, skipper<Iterator>, ClassType()> class_specifier, class_definition, class_def;
      qi::rule<Iterator, skipper<Iterator>, Class()> class_;
      qi::rule<Iterator, skipper<Iterator>, DefClass()> def_class;
      qi::rule<Iterator, skipper<Iterator>, EnumClass()> enum_class;
      qi::rule<Iterator, skipper<Iterator>, DerClass()> deriv_class;
      qi::rule<Iterator, skipper<Iterator>, ExtendsClass()> extends_class;
      qi::rule<Iterator, skipper<Iterator>, ClassList()> class_list; 
      qi::rule<Iterator, skipper<Iterator>, StoredDef()> stored_definition; 
      qi::rule<Iterator, skipper<Iterator>, ShortClass()> short_class_definition;
      qi::rule<Iterator, skipper<Iterator>, TypePrefixes()> type_prefix; 
      qi::rule<Iterator, skipper<Iterator>, ClassPrefixes() > class_prefixes; 
      qi::rule<Iterator, skipper<Iterator>, Name()> name;

      /* Rules with no skip */
      qi::rule<Iterator, String()> string_;
      qi::rule<Iterator, Name()> ident, keyword_ident;

      /* Modelica rules for floating point values */
      qi::real_parser<double,modelica_number_real_policies<double> > modelica_double;
      qi::uint_parser<int> modelica_int;
  
      /* Operators tokens */
      qi::rule<Iterator> OPAREN, CPAREN, COLON, SEMICOLON, QUOTE, STAR,
                         OBRACKET, CBRACKET, COMA, OBRACE, CBRACE, EQUAL, PLUS, 
                         ASSIGN;
  
      /* Keywords with value */
      qi::rule<Iterator, Name()> DOT, DER, INITIAL;
  
      /* Keywords tokens */
      qi::rule<Iterator> ALGORITHM, AND, ANNOTATION, BREAK, 
                         CONNECT, CONSTRAINEDBY,  
                         DER2, EACH, ELSE, ELSEIF, ELSEWHEN, ENCAPSULATED, 
                         END, ENUMERATION, EQUATION, EXTENDS, 
                         EXTERNAL, FALSE, FINAL, FOR, FUNCTION1,
                         IF, IMPORT, IN, INNER, 
                         LOOP, NOT, OR, OUTER,  
                         PROTECTED, PUBLIC,  
                         REDECLARE, REPLACEABLE, THEN, TRUE, 
                         WHEN, WHILE, WITHIN;
  
      Iterator &it; 
    };
  }
}
