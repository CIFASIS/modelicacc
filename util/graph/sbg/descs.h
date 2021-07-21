/*****************************************************************************
 
    This file is part of the Set-Based Graph library.

******************************************************************************/

#pragma once

#include <util/graph/sbg/defs.h>

namespace SBG {

// Set-vertex --------------------------------------------------------------------------------------

struct SVDesc {
  member_class(std::string, text);

  SVDesc();
};

// Set-edge ----------------------------------------------------------------------------------------

struct SEDesc {
  member_class(std::string, text);

  SEDesc();
};

} // namespace SBG
