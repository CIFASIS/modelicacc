#include <parser/statement.h>
#include <parser/statement_def.h>

namespace Modelica
{
  namespace parser {
    typedef std::string::const_iterator iterator_type;
    template struct statement<iterator_type>;
  }
}
