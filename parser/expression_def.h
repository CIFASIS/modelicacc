
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::FunctionExp,
    (Modelica::AST::Name, name_)
    (Modelica::AST::ExpList, args_)
)

BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::Named,
    (Modelica::AST::Name, name_)
    (Modelica::AST::Expression, exp_)
)


BOOST_FUSION_ADAPT_STRUCT(
    Modelica::AST::Call,
    (Modelica::AST::Name, name_)
    (Modelica::AST::ExpList, args_)
)



namespace Modelica
{
  namespace parser {

    struct colon_exp_ : qi::symbols<char, Expression>
    {
      colon_exp_() 
      {
        add
          (":", SubAll())
        ;
      }
    } colon_exp;
 
    struct boolean_ : qi::symbols<char,Boolean>
    {
      boolean_()
      {
        add
            ("true",True)
            ("false",False)
        ;
      }
    } boolean;


    template <typename Iterator>
    expression<Iterator>::expression(Iterator &it) : expression::base_type(expression_), ident_(it), QUOTE("\""), OPAREN("("),CPAREN(")"),
                                                      OBRACKET("["), CBRACKET("]"), COMA(","), it(it), DOT(qi::char_('.')), 
                                                      DER(qi::string("der")),  INITIAL(qi::string("initial")), EQUAL("="), 
                                                      FUNCTION1("function") {
      using qi::char_;
      using qi::_1;
      using qi::_2;
      using qi::_3;
      using qi::_val;
      using phoenix::construct;
      using qi::lexeme;

      expression_ = 
                    *primary 
                  ;

      primary = 
                lexeme[modelica_double]  
              | lexeme[modelica_int] 
              | lexeme[string_] 
              | boolean
              | call_exp 
              | component_reference 
              ;


      name =
             (-DOT) >> ident_ [_val+=_1] >> *(DOT >> ident_) [_val+=_1+_2]
           ;

      component_reference = ((-DOT) >> ident_ >> (-array_subscripts)) [_val=construct<Reference>(_1,_2,_3)]
       >> *(DOT >> ident_ >> (-array_subscripts)) [ _val = construct<Reference>(_val,_2,_3) ]
                          ;

      call_exp =
                 (name  | DER | INITIAL) >> function_call_args 
               ;

      function_call_args = 
                           OPAREN > (-function_arguments) > CPAREN 
                         ;

      named_arguments =
                        named_argument % COMA
                      ;

      named_argument =  
                        ident_ >> EQUAL >> function_argument 
                     ;

      array_subscripts = 
                           OBRACKET > (subscript % COMA) > CBRACKET 
                       ;
 
      function_argument = 
                           function_exp
                        |  expression_ 
                        ;


      function_exp =
                     FUNCTION1 > name > OPAREN >> (-named_arguments) > CPAREN
                   ;
      subscript = 
                  colon_exp 
                | expression_
                ;


      string_ = 
                (QUOTE >> *("\\\"" | (char_ - QUOTE)) > QUOTE) [_val=construct<String>(_1)]
              ;
    

    }
  }
}
