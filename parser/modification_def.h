
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::ElMod,
    (Modelica::AST::Name, name_)
    (Option<Modelica::AST::Modification>, modification_)
    (Modelica::AST::StringComment, st_comment_)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::Component1, 
    (Modelica::AST::TypePrefixes, prefixes_)
    (Modelica::AST::Name, type_)
    (Modelica::AST::Declaration, declaration_)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::ModClass,
    (Modelica::AST::ClassModification, modification_)
    (Modelica::AST::OptExp, exp_)
)


BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::Constrained,
    (Modelica::AST::Name, name_)
    (Option<Modelica::AST::ClassModification>, modification_)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::Enum, 
    (Modelica::AST::Name, name_)
    (Modelica::AST::Comment, comment_)
)


BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::Comment, 
    (Modelica::AST::StringComment,st_comment_)
    (Option<Modelica::AST::Annotation>, annotation_)
)


namespace Modelica
{
  namespace parser {
  Argument add_each_final_rep(bool each, bool final,boost::variant<ElRepl, ElMod> e) {
    if (e.type()==typeid(ElRepl)) {  
      boost::get<ElRepl>(e).set_each(each);
      boost::get<ElRepl>(e).set_final(final);
    } else {  
      boost::get<ElMod>(e).set_each(each);
      boost::get<ElMod>(e).set_final(final);
    }
    return e;
  }
 
  template <typename T>
  T add_comment(T d, Comment c) {
    d.set_comment(c);
    return d;
  }
 
  struct INPUT_ : qi::symbols<char,TypePrefix>
  {
    INPUT_() {
      add ("input",input) ;
    }
  } INPUT;
  struct OUTPUT_ : qi::symbols<char,TypePrefix>
  {
    OUTPUT_() {
      add ("output",output) ;
    }
  } OUTPUT;
  struct FLOW_ : qi::symbols<char,TypePrefix>
  {
    FLOW_() {
      add ("flow",flow) ;
    }
  } FLOW;
  struct STREAM_ : qi::symbols<char,TypePrefix>
  {
    STREAM_() {
      add ("stream",stream) ;
    }
  } STREAM;
  struct DISCRETE_ : qi::symbols<char,TypePrefix>
  {
    DISCRETE_() {
      add ("discrete",discrete) ;
    }
  } DISCRETE;
  struct PARAMETER_ : qi::symbols<char,TypePrefix>
  {
    PARAMETER_() {
      add ("parameter",parameter) ;
    }
  } PARAMETER;
  struct CONSTANT_ : qi::symbols<char,TypePrefix>
  {
    CONSTANT_() {
      add ("constant",constant) ;
    }
  } CONSTANT;

  struct PARTIAL_: qi::symbols<char,ClassPrefix>
  {
    PARTIAL_(){
      add("partial",partial);
    }
  } PARTIAL;
  struct CLASS_: qi::symbols<char,ClassPrefix>
  {
    CLASS_(){
      add("class",class_prefix);
    }
  } CLASS;
  struct MODEL_: qi::symbols<char,ClassPrefix>
  {
    MODEL_(){
      add("model",model);
    }
  } MODEL;
  struct BLOCK_: qi::symbols<char,ClassPrefix>
  {
    BLOCK_(){
      add("block",block);
    }
  } BLOCK;
  struct RECORD_: qi::symbols<char,ClassPrefix>
  {
    RECORD_(){
      add("record",record);
    }
  } RECORD;
  struct CONNECTOR_: qi::symbols<char,ClassPrefix>
  {
    CONNECTOR_(){
      add("connector",connector);
    }
  } CONNECTOR;
  struct TYPE_: qi::symbols<char,ClassPrefix>
  {
    TYPE_(){
      add("type",Modelica::AST::type);
    }
  } TYPE;
  struct OPERATOR_: qi::symbols<char,ClassPrefix>
  {
    OPERATOR_(){
      add("operator",operator_prefix);
    }
  } OPERATOR;
  struct EXPANDABLE_: qi::symbols<char,ClassPrefix>
  {
    EXPANDABLE_(){
      add("expandable",expandable);
    }
  } EXPANDABLE;
  struct PURE_: qi::symbols<char,ClassPrefix>
  {
    PURE_(){
      add("pure",pure);
    }
  } PURE;
  struct IMPURE_: qi::symbols<char,ClassPrefix>
  {
    IMPURE_(){
      add("impure",impure);
    }
  } IMPURE;
  struct PACKAGE_: qi::symbols<char,ClassPrefix>
  {
    PACKAGE_(){
      add("package",package);
    }
  } PACKAGE;
  struct FUNCTION_: qi::symbols<char,ClassPrefix>
  {
    FUNCTION_(){
      add("function",Modelica::AST::function);
    }
  } FUNCTION;


  std::string at(std::string::const_iterator where, std::string::const_iterator start);
 

    template <typename Iterator>
    modification<Iterator>::modification(Iterator &it) : modification::base_type(modification_), it(it), expression_(it),
                                                         ASSIGN(":="), EQUAL("="), PLUS("+"), ANNOTATION("annotation"),
                                                          COMA(","), COLON(":"), ENUMERATION("enumeration"), OPAREN("("), CPAREN(")"),
                                                         CONSTRAINEDBY("constrainedby"), REPLACEABLE("replaceable"),
                                                         REDECLARE("redeclare"), EACH("each"), FINAL("final") {
      using qi::_1;
      using qi::_val;
      using phoenix::construct;
      using qi::on_error;
      using qi::_2;
      using qi::_3;
      using qi::_4;
      using qi::_5;
      using qi::_6;
      using qi::_7;
      using phoenix::val;
      using qi::fail;
      using qi::matches;
      using phoenix::bind;
      /*using qi::char_;
      using phoenix::val;
      using qi::fail;
      using qi::lexeme;*/

      modification_= 
                     equal_mod
                   | assign_mod 
                   | class_mod
                   ;

 
      equal_mod =
                  EQUAL > expression_
                ;
      
      assign_mod =
                   ASSIGN > expression_
                 ;
    
      annotation =  
                    ANNOTATION > class_modification
                 ;

      comment = 
                string_comment >> (-annotation)
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

      enum_list = 
                  enumeration_literal % COMA 
                ;
      
      enum_spec = 
                  enum_list
                | COLON
                ;

 
      enumeration_literal =
                            expression_.ident_ > comment
                          ;


      class_modification = 
                           OPAREN > (-(argument % COMA)) > CPAREN 
                         ;
    
      argument = 
                 element_redeclaration [_val=_1]
               | (matches[EACH] >> matches[FINAL] >> (element_modification | element_replaceable)) [_val=bind(&add_each_final_rep,_1,_2,_3)]
               ;

 
      element_redeclaration =
                              (REDECLARE >> matches[EACH] >> matches[FINAL] >> ((short_class_definition | component_clause1) | element_replaceable))
                              [_val=construct<ElRedecl>(_1,_2,_3)]
                           ; 

 
      element_modification =
                              expression_.name >> (-modification_) >> string_comment
                           ; 
                                           
 
      element_replaceable =
                            (REPLACEABLE >> (short_class_definition | component_clause1) >> (-constraining_clause)) [_val=construct<ElRepl>(_1,_2)]
                          ; 
                                           
 

      // TODO add the + string
      string_comment =
                       -(expression_.string_ [_val=construct<StringList>(1,_1)] >> *(PLUS > expression_.string_) [_val=StringComment()])
                     ;


      component_clause1 =
                          type_prefix >> expression_.name >> component_declaration1 
                        ;

      component_declaration1 =
                               (declaration >> comment) [_val=bind(&add_comment<Declaration>,_1,_2)]
                             ;

      declaration =
                    expression_.ident_ >> (-expression_.array_subscripts) >> (-modification_)
                  ;

       constraining_clause = 
                            CONSTRAINEDBY > expression_.name > (-class_modification)
                          ;

       class_mod = 
                  class_modification >> -(EQUAL > expression_)
                ;

 

      // TODO: return something meaningful
      short_class_definition =
                               (class_prefixes >> expression_.ident_ >> EQUAL >> type_prefix >> expression_.name >> (-expression_.array_subscripts) >> (-class_modification) >> comment) [_val=construct<ShortClass>(_1,_2,_3,_4,_5,_6,_7)]
                             | (class_prefixes >> expression_.ident_ >> EQUAL >> ENUMERATION >> OPAREN >> enum_spec >> CPAREN >>  comment)  [_val=ShortClass()]
                             ;
      
 
      /* Error and debug */
      on_error<fail>
        (
            modification_
          , std::cerr
                << val("Parser error. Expecting ")
                << _4                               // what failed?
                << bind(&at,_3,it)
                << std::endl
        );
 
      OPAREN.name("(");
      CPAREN.name(")");
      COLON.name(":");
      COMA.name(",");
      EQUAL.name("=");
      PLUS.name("+"); 
      ASSIGN.name(":="); 
      annotation.name("annotation");
      argument.name("argument");
      assign_mod.name("modification");
      class_modification.name("class_modification");
      class_mod.name("class_modification");
      class_prefixes.name("class_prefixes");
      comment.name("comment");
      constraining_clause.name("constraining_clause");
      declaration.name("declaration");
      element_modification.name("element_modification");
      element_redeclaration.name("element_redeclaration");
      equal_mod.name("modification");
      element_replaceable.name("element_replaceable");
      enumeration_literal.name("enumeration literal");
      enum_list.name("enum list");
      modification_.name("modification");
      short_class_definition.name("short_class_definition");
      string_comment.name("string_comment");
      component_clause1.name("component_clause1");
      type_prefix.name("type_prefix");
      component_declaration1.name("component_declaration1");

    }
  }
}
