#include <parser/expression.h>
#include <parser/expression_def.h>

namespace Modelica
{
  namespace parser {
    typedef std::string::const_iterator iterator_type;
    template struct expression<iterator_type>;
  }
}
