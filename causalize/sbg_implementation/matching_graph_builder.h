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

#include <mmo/mmo_class.h>
#include <util/graph/graph_definition.h>

namespace Causalize {

class MatchingGraphBuilder {
  public:
  MatchingGraphBuilder(Modelica::MMO_Class& mmo_class);
  ~MatchingGraphBuilder() = default;
  virtual SBG::SBGraph makeGraph();

  protected:
  typedef std::pair<SBG::PWLMap, SBG::PWLMap> MatchingMaps;

  SBG::Set buildSet(VarInfo variable);
  SBG::Set buildSet(Equation eq);
  SBG::Set buildSet(SBG::MultiInterval variable);
  SBG::SetVertexDesc addVertex(std::string vertex_name, SBG::Set set, SBG::SBGraph& graph);
  void addEquation(Equation eq, int id, SBG::Set set, SBG::SBGraph& graph);
  Real getValue(Expression exp);
  SBG::PWLMap buildPWLMap(SBG::OrdCT<SBG::NI2> constants, SBG::OrdCT<SBG::NI2> slopes, SBG::Set dom);
  MatchingMaps generatePWLMaps(Expression exp, SBG::Set dom, int offset);
  SBG::Set generateMapDom(SBG::Set dom, int offset);

  private:
  typedef std::pair<SBG::SetVertexDesc, Equality> EquationDesc; 

  Modelica::MMO_Class& _mmo_class;
  std::list<SBG::SetVertexDesc> _var_nodes;
  std::list<EquationDesc> _equation_nodes;
};

}  // namespace Causalize
