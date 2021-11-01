/*****************************************************************************
 
    This file is part of the Set-Based Graph library.

******************************************************************************/

#include <util/graph/sbg/sbg.h>

namespace SBG {

// Set-vertex --------------------------------------------------------------------------------------

SET_VERTEX_TEMPLATE
SETV_TEMP_TYPE::SetVertexImp() : name_(""), id_(-1), index_(0), desc_() 
{
  Set emptySet;
  range_ = emptySet;
}

SET_VERTEX_TEMPLATE
SETV_TEMP_TYPE::SetVertexImp(std::string name, Set range) : name_(name), id_(-1), range_(range), index_(0), desc_() {}

SET_VERTEX_TEMPLATE
SETV_TEMP_TYPE::SetVertexImp(std::string name, int id, Set range, int index) : name_(name), id_(id), range_(range), index_(index), desc_() {}

SET_VERTEX_TEMPLATE
SETV_TEMP_TYPE::SetVertexImp(std::string name, int id, Set range, int index, DESC desc) : name_(name), id_(id), range_(range), index_(index), desc_(desc) {}

member_imp_temp(SET_VERTEX_TEMPLATE, SETV_TEMP_TYPE, std::string, name);
member_imp_temp(SET_VERTEX_TEMPLATE, SETV_TEMP_TYPE, int, id);
member_imp_temp(SET_VERTEX_TEMPLATE, SETV_TEMP_TYPE, Set, range);
member_imp_temp(SET_VERTEX_TEMPLATE, SETV_TEMP_TYPE, int, index);
member_imp_temp(SET_VERTEX_TEMPLATE, SETV_TEMP_TYPE, DESC, desc);

SET_VERTEX_TEMPLATE
bool SETV_TEMP_TYPE::operator==(const SETV_TEMP_TYPE &other) const { return id() == other.id(); }

template struct SetVertexImp<SVDesc>;

SET_VERTEX_TEMPLATE
std::ostream &operator<<(std::ostream &out, const SETV_TEMP_TYPE &v)
{
  out << v.name() << ": " << v.range() << "\n";

  return out;
}

template std::ostream &operator<<(std::ostream &out, const SetVertex &v);

// Set-edge ----------------------------------------------------------------------------------------

SET_EDGE_TEMPLATE
SETE_TEMP_TYPE::SetEdgeImp() : name_(""), id_(-1), map_f_(), map_u_(), index_(0), desc_() {}

SET_EDGE_TEMPLATE
SETE_TEMP_TYPE::SetEdgeImp(std::string name, PWLMap map_f, PWLMap map_u) : name_(name), id_(-1), map_f_(map_f), map_u_(map_u), index_(0), desc_() {}

SET_EDGE_TEMPLATE
SETE_TEMP_TYPE::SetEdgeImp(std::string name, PWLMap map_f, PWLMap map_u, DESC desc) : name_(name), id_(-1), map_f_(map_f), map_u_(map_u), index_(0), desc_(desc) {}

SET_EDGE_TEMPLATE
SETE_TEMP_TYPE::SetEdgeImp(std::string name, int id, PWLMap map_f, PWLMap map_u, int index) : name_(name), id_(id), map_f_(map_f), map_u_(map_u), index_(index), desc_() {}

SET_EDGE_TEMPLATE
SETE_TEMP_TYPE::SetEdgeImp(std::string name, int id, PWLMap map_f, PWLMap map_u, int index, DESC desc) : name_(name), id_(id), map_f_(map_f), map_u_(map_u), index_(index), desc_(desc) {}

member_imp_temp(SET_EDGE_TEMPLATE, SETE_TEMP_TYPE, std::string, name);
member_imp_temp(SET_EDGE_TEMPLATE, SETE_TEMP_TYPE, int, id);
member_imp_temp(SET_EDGE_TEMPLATE, SETE_TEMP_TYPE, PWLMap, map_f);
member_imp_temp(SET_EDGE_TEMPLATE, SETE_TEMP_TYPE, PWLMap, map_u);
member_imp_temp(SET_EDGE_TEMPLATE, SETE_TEMP_TYPE, int, index);
member_imp_temp(SET_EDGE_TEMPLATE, SETE_TEMP_TYPE, DESC, desc);

SET_EDGE_TEMPLATE
SETE_TEMP_TYPE SETE_TEMP_TYPE::restrictEdge(Set dom)
{
  PWLMap resf = map_f_ref().restrictMap(dom);
  PWLMap resu = map_u_ref().restrictMap(dom);

  return SetEdgeImp(name(), id(), resf, resu, index(), desc());
}

SET_EDGE_TEMPLATE
bool SETE_TEMP_TYPE::operator==(const SETE_TEMP_TYPE &other) const { return id() == other.id(); }

template struct SetEdgeImp<SEDesc>;

SET_EDGE_TEMPLATE
std::ostream &operator<<(std::ostream &out, const SETE_TEMP_TYPE &E)
{
  SetEdge auxE = E;

  std::string Enm = E.name();
  OrdCT<Set> dom = auxE.map_f_ref().dom();
  OrdCT<Set>::iterator itdom = dom.begin();

  out << Enm << " dom: ";
  out << "[";
  for (; next(itdom, 1) != dom.end(); ++itdom)
    out << *itdom << ", ";
  out << *itdom << "]\n";

  OrdCT<LMap> lmleft = auxE.map_f_ref().lmap();
  OrdCT<LMap>::iterator itleft = lmleft.begin();
  OrdCT<LMap> lmright = auxE.map_u_ref().lmap();
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

template std::ostream &operator<<(std::ostream &out, const SetEdge &e);

}  // namespace SBG
