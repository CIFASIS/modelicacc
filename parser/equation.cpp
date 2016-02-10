#include <parser/equation.h>
#include <parser/equation_def.h>

namespace Modelica
{
  namespace parser {
    typedef std::string::const_iterator iterator_type;
    template struct equation<iterator_type>;
  }
}
