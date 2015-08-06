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

#ifndef REMOVE_ALIAS_H
#define REMOVE_ALIAS_H
#include <mmo/mmo_class.h>

namespace Modelica {
class RemoveAlias {
  MMO_Class &_c;
  void alias(Reference a, Expression b);
public:
  RemoveAlias(MMO_Class &c);
  void removeAliasEquations();
};
}

#endif
