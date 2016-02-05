#include <parser/modification.h>
#include <parser/modification_def.h>

namespace Modelica
{
  namespace parser {
    typedef std::string::const_iterator iterator_type;
    template struct modification<iterator_type>;
  }
}
