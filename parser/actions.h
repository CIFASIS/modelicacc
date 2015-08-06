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

namespace Modelica
{
  namespace parser {
    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;
    using namespace Modelica::AST;

    /* Modelica rules for floating point values */
    template <typename T>
    struct modelica_number_real_policies : qi::strict_ureal_policies<T>
    {
      static bool const expect_dot = true;
      static bool const allow_leading_dot = false;
      static bool const allow_trailing_dot = true;

      template <typename Iter, typename Attribute>
      static bool
      parse_nan(Iter& first, Iter const& last, Attribute& attr) {
        return false;
      }

      template <typename Iter, typename Attribute>
      static bool
      parse_inf(Iter& first, Iter const& last, Attribute& attr) {
        return false;
      }
    };
 
    struct end_exp_ : qi::symbols<char, Expression>
    {
      end_exp_() 
      {
        add
          ("end", SubEnd())
        ;
      }
    } end_exp;

    struct colon_exp_ : qi::symbols<char, Expression>
    {
      colon_exp_() 
      {
        add
          (":", SubAll())
        ;
      }
    } colon_exp;
 
    struct ret_break_st_ : qi::symbols<char, Statement>
    {
      ret_break_st_() 
      {
        add
          ("return", Return())
          ("break", Break())
        ;
      }
    } ret_break_st;
 
    struct quoted_chars_ : qi::symbols<char,char>
    {
      quoted_chars_()
      {
        add
          (" ",' ')
          ("!",'!')
          ("#",'#')
          ("$",'$')
          ("%",'%')
          ("&",'&')
          ("(",'(')
          (")",')')
          ("*",'*')
          ("+",'+')
          (",",',')
          ("-",'-')
          (".",'.')
          ("/",'/')
          (":",':')
          (";",';')
          ("<",'<')
          (">",'>')
          ("=",'=')
          ("?",'?')
          ("@",'@')
          ("[",'[')
          ("]",']')
          ("^",'^')
          ("{",'{')
          ("}",'}')
          ("~",'~')
          ("|",'|')
        ;
      }
    } quoted_chars;
    struct keywords_ : qi::symbols<char, std::string>
    {
      keywords_()
      {
        add
           ("algorithm","algorithm")
           ("and","and")
           ("annotation","annotation")
           ("block","block")
           ("break","break")
           ("class","class")
           ("connect","connect")
           ("connector","connector")
           ("constant","constant")
           ("constrainedby","constrainedby")
           ("der","der")
           ("discrete","discrete")
           ("each","each")
           ("else","else")
           ("elseif","elseif")
           ("elsewhen","elsewhen")
           ("encapsulated","encapsulated")
           ("end","end")
           ("enumeration","enumeration")
           ("equation","equation")
           ("expandable","expandable")
           ("extends","extends")
           ("external","external")
           ("false","false")
           ("final","final")
           ("flow","flow")
           ("for","for")
           ("function","function")
           ("if","if")
           ("import","import")
           ("impure","impure")
           ("in","in")
           ("initial","initial")
           ("inner","inner")
           ("input","input")
           ("loop","loop")
           ("model","model")
           ("not","not")
           ("operator","operator")
           ("or","or")
           ("outer","outer")
           ("output","output")
           ("package","package")
           ("parameter","parameter")
           ("partial","partial")
           ("protected","protected")
           ("public","public")
           ("pure","pure")
           ("record","record")
           ("redeclare","redeclare")
           ("replaceable","replaceable")
           ("return","return")
           ("stream","stream")
           ("then","then")
           ("true","true")
           ("type","type")
           ("when","when")
           ("while","while")
           ("within","within")
        ;
      }
    } keywords;

    struct exp_op_ : qi::symbols<char, BinOpType>
    {
      exp_op_()
      {
        add
          (".^", ElExp)
          ("^", Exp)
        ;
      }
    } exp_op;
  
    struct mul_op_ : qi::symbols<char, BinOpType>
    {
      mul_op_()
      {
        add
          ("*", Mult)
          ("/", Div)
          (".*", ElMult)
          ("./", ElDiv)
        ;
      }
    } mul_op;

    struct add_op_ : qi::symbols<char, BinOpType>
    {
      add_op_()
      {
        add
          ("+",Add)
          ("-",Sub)
          (".+",ElAdd)
          (".-",ElSub)
        ;
      }
    } add_op;

    struct rel_op_ : qi::symbols<char, BinOpType>
    {
      rel_op_()
      {
        add
          ("<=", LowerEq)
          ("<", Lower) 
          (">=", GreaterEq)
          (">", Greater)
          ("==", CompEq)
          ("<>", CompNotEq)
        ;
      }
    } rel_op;

    struct and_op_ : qi::symbols<char, BinOpType>
    {
      and_op_()
      {
        add
          ("and", And)
        ;
      }
    } and_op;

    struct or_op_ : qi::symbols<char, BinOpType>
    {
      or_op_()
      {
        add
          ("or", Or)
        ;
      }
    } or_op;

    struct unary_op_ : qi::symbols<char, UnaryOpType>
    {
      unary_op_()
      {
        add
          ("+", Plus)
          ("-", Minus)
        ;
      }
    } unary_op;
   
    struct not_op_ : qi::symbols<char, UnaryOpType>
    {
      not_op_()
      {
        add
          ("not", Not)
        ;
      }
    } not_op;

    struct boolean_ : qi::symbols<char,Expression>
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
  struct skipper : qi::grammar<Iterator>
  {
      skipper() : skipper::base_type(start)
      {
          qi::char_type char_;
          ascii::space_type space;

          start =
                  space                               // tab/space/cr/lf
              |   "//" >> *(char_ - "\n") >> "\n"     // C++-style comments
              |   "/*" >> *(char_ - "*/") >> "*/"     // C-style comments
              ;
      }

      qi::rule<Iterator> start;
  };
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
      add("type",type);
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
      add("function",function);
    }
  } FUNCTION;


  // Helper functions


  template <typename T>
  T add_comment(T d, Comment c) {
    d.set_comment(c);
    return d;
  }
  Declaration add_cond(Declaration d,OptExp cond, Comment c) {
    Declaration ret=d;
    ret.set_comment(c);
    ret.set_conditional(cond);
    return ret;
  }
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
 
  template <typename T>
  std::vector<T> consume_one(T t) {
    std::vector<T> l(1,t);
    return l;
  }
 

  template <typename T>
  std::vector<T> append_list(std::vector<T> lt, std::vector<T> lt2) {
    lt.insert(lt.end(),lt2.begin(), lt2.end());
    return lt;
  }

  Expression range1(Expression a, Expression b) {
    return Range(a,b);
  }
  Expression range2(Expression a, Expression b) {
    Range r=boost::get<Range>(a);
    return Range(r.start(),r.end(),b);
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
  
  Composition add_annot(Composition c, Annotation a) {
    c.set_annotation(a);
    return c;
  }
  
  std::string at(std::string::const_iterator where, std::string::const_iterator start) {
    std::stringstream ss;
    int line=1, col=1;
    while (where!=start) {
      if (*start=='\n') {
        line++; 
        col=1;
      } else {
        col++; 
      }
      start++;
    }
    ss << " at Line " << line << " Col: " << col;
    return ss.str();
  }
 
 
  }
}
