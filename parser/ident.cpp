#include "ident.h"
#include "ident_def.h"



namespace Modelica
{
  namespace parser {
    typedef std::string::const_iterator iterator_type;
    template struct ident<iterator_type>;
  }
}
