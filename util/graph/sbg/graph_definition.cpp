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

#include <util/graph/sbg/graph_definition.h>

namespace SBG {

SET_VERTEX_TEMPLATE
SETV_TEMP_TYPE::SetVertexImp() : name_(""), id_(-1), index_(0), desc_() 
{
  Set emptySet;
  range_ = emptySet;
}

SET_VERTEX_TEMPLATE
SETV_TEMP_TYPE::SetVertexImp(std::string name, Set range) : name_(name), id_(-1), range_(range), index_(0), desc_() {}

SET_VERTEX_TEMPLATE
SETV_TEMP_TYPE::SetVertexImp(std::string name, int id, Set range, int index, DESCRIPTION desc) : name_(name), id_(id), range_(range), index_(index), desc_(desc) {}

member_imp_temp(SET_VERTEX_TEMPLATE, SETV_TEMP_TYPE, std::string, name);
member_imp_temp(SET_VERTEX_TEMPLATE, SETV_TEMP_TYPE, int, id);
member_imp_temp(SET_VERTEX_TEMPLATE, SETV_TEMP_TYPE, Set, range);
member_imp_temp(SET_VERTEX_TEMPLATE, SETV_TEMP_TYPE, int, index);
member_imp_temp(SET_VERTEX_TEMPLATE, SETV_TEMP_TYPE, DESCRIPTION, desc);

SET_VERTEX_TEMPLATE
bool SET_VERTEX_TEMPLATE::operator==(const SET_VERTEX_TEMPLATE &other) const { return id() == other.id(); }

template struct SetVertexImp<std::string>;

SET_VERTEX_TEMPLATE
std::ostream &operator<<(std::ostream &out, const SETV_TEMP_TYPE &v)
{
  out << v.name() << ": " << v.range() << "\n";

  return out;
}

/*-----------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
// Printing instances
/*-----------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/

std::ostream &operator<<(std::ostream &out, SetEdge &E)
{
  std::string Enm = E.name_();
  OrdCT<Set> dom = E.es1_().dom();
  OrdCT<Set>::iterator itdom = dom.begin();

  out << Enm << " dom: ";
  out << "[";
  for (; next(itdom, 1) != dom.end(); ++itdom)
    out << *itdom << ", ";
  out << *itdom << "]\n";

  OrdCT<LMap> lmleft = E.es1_().lmap();
  OrdCT<LMap>::iterator itleft = lmleft.begin();
  OrdCT<LMap> lmright = E.es2_().lmap();
  OrdCT<LMap>::iterator itright = lmright.begin();

  out << Enm << " left | right: ";
  out << "[";
  for (; next(itleft, 1) != lmleft.end(); ++itleft)
    out << *itleft << ", ";
  out << *itleft << "] | ";

  out << "[";
  for (; next(itright, 1) != lmright.end(); ++itright)
    out << *itright << ", ";
  out << *itright << "]\n";

  return out;
}

/*-----------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
// Map operations
/*-----------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/

SetEdge restrictEdge(SetEdge e, Set dom)
{
  PWLMap es1 = e.es1_();
  PWLMap es2 = e.es2_();

  PWLMap res1 = es1.restrictMap(dom);
  PWLMap res2 = es2.restrictMap(dom);

  SetEdge res(e.name, e.id_(), res1, res2, 0);
  return res;
}

}  // namespace SBG
