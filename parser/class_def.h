#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/boost_tuple.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>
#include <ast/element.h>

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::Class, 
    (Modelica::AST::Name, name_)
    (Modelica::AST::StringComment, st_comment_)
    (Modelica::AST::Composition, composition_)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::EnumClass, 
    (Modelica::AST::Name, name_)
    (Modelica::AST::EnumSpec, enum_spec_)
    (Modelica::AST::Comment, comment_)
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
    Modelica::AST::External, 
    (Modelica::AST::OptExp, comp_ref_)
    (Modelica::AST::Name, fun_)
    (Modelica::AST::OptExpList, args_)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::Extends, 
    (Modelica::AST::Name, name_)
    (Option<Modelica::AST::ClassModification>, modification_)
    (Option<Modelica::AST::Annotation>, annotation_)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::Component, 
    (Modelica::AST::TypePrefixes, prefixes_)
    (Modelica::AST::Name, type_)
    (Option<Modelica::AST::ExpList>, indices_)
    (Modelica::AST::DeclList, declarations_)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::ExtendsClass, 
    (Modelica::AST::Name, name_)
    (Option<Modelica::AST::ClassModification>, modification_)
    (Modelica::AST::StringComment, st_comment_)
    (Modelica::AST::Composition, composition_)
)




namespace Modelica
{
  namespace parser {

  Declaration add_cond(Declaration d,OptExp cond, Comment c) {
    Declaration ret=d;
    ret.set_comment(c);
    ret.set_conditional(cond);
    return ret;
  }
   Composition add_annot(Composition c, Annotation a) {
    c.set_annotation(a);
    return c;
  }
   ClassType add_enc_prefixes(fusion::vector2<bool, ClassPrefixes> a, ClassType c) {
    const bool &encap = get<0>(a);
    const ClassPrefixes &pre = get<1>(a);
    if (is<Class>(c)) {
      get<Class>(c).set_encapsulated(encap);
      get<Class>(c).set_prefixes(pre);
    }
    if (is<DerClass>(c)) {
      get<DerClass>(c).set_encapsulated(encap);
      get<DerClass>(c).set_prefixes(pre);
    }
    if (is<DefClass>(c)) {
      get<DefClass>(c).set_encapsulated(encap);
      get<DefClass>(c).set_prefixes(pre);
    }
    if (is<EnumClass>(c)) {
      get<EnumClass>(c).set_encapsulated(encap);
      get<EnumClass>(c).set_prefixes(pre);
    }
    if (is<ExtendsClass>(c)) {
      get<ExtendsClass>(c).set_encapsulated(encap);
      get<ExtendsClass>(c).set_prefixes(pre);
    }
    return c;
  }
 
   ClassType add_final(ClassType c, bool final) {
    if (is<Class>(c)) {
      get<Class>(c).set_final(final);
      return get<Class>(c);
    } 
    if (is<DerClass>(c)) {
      get<DerClass>(c).set_final(final);
      return get<DerClass>(c);
    }
    if (is<DefClass>(c)) {
      get<DefClass>(c).set_final(final);
      return get<DefClass>(c);
    }
    if (is<EnumClass>(c)) {
      get<EnumClass>(c).set_final(final);
      return get<EnumClass>(c);
    }
    if (is<ExtendsClass>(c)) {
      get<ExtendsClass>(c).set_final(final);
      return get<ExtendsClass>(c);
    }
    return c;
  }
  Element class_comp(bool red, bool final, bool inner, bool outer, Element c) {
    if (c.type()==typeid(Component)) {
      boost::get<Component>(c).set_redeclare(red);
      boost::get<Component>(c).set_final(final);
      boost::get<Component>(c).set_inner(inner);
      boost::get<Component>(c).set_outer(outer);
      return boost::get<Component>(c);
    } else {
      boost::get<ElemClass>(c).set_final(final);
      boost::get<ElemClass>(c).set_inner(inner);
      boost::get<ElemClass>(c).set_outer(outer);
      boost::get<ElemClass>(c).set_redeclare(red);
      return boost::get<ElemClass>(c);
    }
  } 


  typedef boost::variant<Component, ClassType> CompClass;
  typedef boost::optional<boost::fusion::vector2<Constrained,Comment> > Constr;
  Element element2_act(CompClass cl, Constr con) {
    using boost::fusion::at_c;
    if (cl.type()==typeid(Component)) {
      boost::get<Component>(cl).set_replaceable(true);
      if (con) {
        boost::get<Component>(cl).set_constrained(at_c<0>(con.get()));
        boost::get<Component>(cl).set_constrained_comment(at_c<1>(con.get()));
      }
      return boost::get<Component>(cl);
    }
    else {
      ElemClass r(ClassType_(boost::get<ClassType>(cl)));
      if (con) {
        r.set_replaceable(true);
        r.set_constrained(at_c<0>(con.get()));
        r.set_constrained_comment(at_c<1>(con.get()));
      }
      return r;
    }
  }

 
    std::string at(std::string::const_iterator where, std::string::const_iterator start) ;
    template <typename Iterator>
    class_<Iterator>::class_(Iterator &it) : class_::base_type(stored_definition), it(it), modification_(it), expression_(it),
                                                        equation_(it), statement_(it), IMPORT("import"), EXTENDS("extends"),
                                                        OUTER("outer"), INNER("inner"), REDECLARE("redeclare"), 
                                                        EXTERNAL("external"), WITHIN("within"),
                                                        EQUAL("="), DOT("."), STAR("*"), OBRACE("{"), CBRACE("}"), COMA(","),
                                                        ENCAPSULATED("encapsulated"), FINAL("final"), REPLACEABLE("replaceable"),
                                                        IF("if"), SEMICOLON(";"), PROTECTED("protected"), PUBLIC("public"),
                                                        OPAREN("("), CPAREN(")"), DER("der"), ENUMERATION("enumeration"), END("end") {
      using qi::_1;
      using qi::_2;
      using qi::_3;
      using qi::_4;
      using qi::_5;
      using qi::_val;
      using phoenix::bind;
      using phoenix::construct;
      using phoenix::val;
      using qi::fail;
      using qi::on_error;
      using qi::matches;
      using qi::omit;

      stored_definition = 
                          (WITHIN >> (-expression_.name) > SEMICOLON > class_list) [_val=construct<StoredDef>(_1,_2) ]
                        | class_list [_val=construct<StoredDef>(_1) ]
                        ;

 
      class_list =
                   +(class_def > SEMICOLON) 
                 ;
 
      class_def =
                   (matches[FINAL] >> class_definition) [_val=bind(&add_final,_2,_1)]
                ;

 
      class_definition = 
                         (matches[ENCAPSULATED] >> modification_.class_prefixes > class_specifier) [_val=bind(&add_enc_prefixes,_1,_2)]
                       ;
      
 
      class_specifier =
                        class_t
                      | def_class
                      | enum_class
                      | deriv_class
                      | extends_class
                      ;

 
      class_t  =
                expression_.ident_ >> modification_.string_comment >> composition >> END > omit[expression_.ident_]
              ;

 
      extends_class =
                      EXTENDS > expression_.ident_ >> (-modification_.class_modification) >> modification_.string_comment >> composition > END > omit[expression_.ident_]
                    ;

      deriv_class =
                    expression_.ident_ >> EQUAL >> DER > OPAREN > expression_.name > COMA > (expression_.ident_ % COMA) > CPAREN > modification_.comment
                  ;

 
      external_function_call =
                               (-(expression_.component_reference >> EQUAL)) >> expression_.ident_ >> OPAREN >> (-expression_.expression_list) >> CPAREN
                             ;

 
      composition = 
                    (element_list >> (*comp_element) 
                        >> (-(EXTERNAL >> (-expression_.string_) >> (-external_function_call) >> (-modification_.annotation) >> SEMICOLON))
                    ) [_val=construct<Composition>(_1,_2,_3)] >> (-((modification_.annotation [_val=bind(&add_annot,_val,_1)]>> SEMICOLON) ))
                  ;

 
      comp_element =
                     equation_.equation_section   [_val=_1]
                   | statement_.algorithm_section  [_val=_1]
                   | PUBLIC >> element_list [_val=construct<Public>(_1)]
                   | PROTECTED >> element_list [_val=construct<Protected>(_1)]
                   ;


      enum_class = 
                   expression_.ident_ >> EQUAL >> ENUMERATION > OPAREN > modification_.enum_spec > CPAREN >> modification_.comment
                 ;

 
      def_class = 
                  expression_.ident_ >> EQUAL >> modification_.type_prefix >> expression_.name >> (-expression_.array_subscripts) 
                  >> (-modification_.class_modification) >> modification_.comment
                ;

 
      element2 = 
                 (component_clause [_val=_1] | class_definition [_val=construct<ElemClass>(construct<ClassType_>(_1))])
               | (REPLACEABLE >> (class_definition | component_clause) >> -(modification_.constraining_clause >> modification_.comment)) [_val=bind(&element2_act,_1,_2)]
               ;

 
      component_clause = 
                         modification_.type_prefix >> expression_.name >> (-expression_.array_subscripts) >> component_list
                       ;
    
 
      component_list =
                       component_declaration % COMA
                     ;
  
 
      component_declaration = 
                              (modification_.declaration >> (-condition_attribute) >> modification_.comment)
                               [_val=bind(&add_cond,_1,_2,_3)]
                            ;
 
      condition_attribute =
                            IF > expression_
                          ;
      
 

      import_clause = 
                      (IMPORT >> expression_.ident_ >> EQUAL >> expression_.name >> modification_.comment) [_val=construct<Import>(_1,_2,_3)]
                    | (IMPORT >> expression_.name >> (-(omit[DOT] >> (STAR | (OBRACE >> (expression_.ident_ % COMA) >> CBRACE)))) >> modification_.comment) [_val=construct<Import>(_1,_2,_3)]
                    ;

      extends_clause =
                      EXTENDS > expression_.name >> (-modification_.class_modification) >> (-modification_.annotation)
                    ;

      element_list = 
                      *(element > SEMICOLON)
                   ;

 
      element = 
                import_clause [_val=_1]
              | extends_clause [_val=_1]
              | (matches[REDECLARE] >> matches[FINAL] >> matches[INNER] >> matches[OUTER] >> element2) [_val=bind(&class_comp,_1,_2,_3,_4, _5)]
              ;

 
      stored_definition.name("stored_definition");
      class_def.name("class_definition");
      class_specifier.name("class_specifier");
      class_definition.name("class_definition");
      class_list.name("class_list");
      component_declaration.name("component_declaration");
      component_list.name("component_list");
      component_clause.name("component_clause");
      class_t.name("class");
      def_class.name("Definition class");
      enum_class.name("Enum class");
      deriv_class.name("Derivate class");
      extends_class.name("Extends class");
 
      on_error<fail>
        (
            stored_definition
          , std::cerr
                << val("Parser error. Expecting ")
                << _4                               // what failed?
                << bind(&at,_3,it)
                << std::endl
        );
 
    }
  }
}
