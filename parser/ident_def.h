
namespace Modelica
{
  namespace parser {
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
    
    template <typename Iterator>
    ident<Iterator>::ident(Iterator &it) : ident::base_type(ident_) {
      using qi::char_;
      using qi::alpha;
      using qi::alnum;
      using qi::lexeme;

      keyword_ident = keywords >> +(alnum | char_('_')) 
                    ;

      ident_ = lexeme[((char_('_') | alpha) >> *(alnum | char_('_'))) - keywords]
             | lexeme[keyword_ident]
             | lexeme[char_('\'') >> *(alnum | char_('_') | quoted_chars) > char_('\'')] 
             ;


      ident_.name("identifier"); 
      keyword_ident.name("identifier"); 
    }
  }
}
