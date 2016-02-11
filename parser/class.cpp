#include <parser/class.h>
#include <parser/class_def.h>

namespace Modelica
{
  namespace parser {
    typedef std::string::const_iterator iterator_type;
    template struct class_<iterator_type>;
  }
}
