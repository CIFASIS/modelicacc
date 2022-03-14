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
#include <sbg/sbg.hpp>

namespace Causalize {

class MatchingGraphBuilder {
  public:
  MatchingGraphBuilder(Modelica::MMO_Class& mmo_class);
  ~MatchingGraphBuilder() = default;
  virtual SBG::SBGraph makeGraph();

  protected:
  typedef std::pair<SBG::PWLMap, SBG::PWLMap> MatchingMaps;

  SBG::Set buildSet(VarInfo variable, int offset, size_t max_dim);
  SBG::Set buildSet(Equation eq, std::string eq_id, int offset, size_t max_dim);
  SBG::Set buildSet(SBG::MultiInterval variable);
  SBG::SetVertexDesc addVertex(std::string vertex_name, SBG::Set set, SBG::SBGraph& graph);
  void addEquation(Equation eq, std::string id, SBG::Set set, SBG::SBGraph& graph);
  Real getValue(Expression exp);
  SBG::PWLMap buildPWLMap(SBG::ORD_REALS constants, SBG::ORD_REALS slopes, SBG::Set dom);
  MatchingMaps generatePWLMaps(Expression exp, SBG::Set dom, SBG::Set unk_dom, int offset, std::string eq_id, size_t max_dim);
  SBG::Set generateMapDom(SBG::Set dom, SBG::Set unk_dom, int offset, size_t max_dim);
  void addDims(size_t max_dim, size_t exp_dim, SBG::MultiInterval& intervals, int offset);
  void addDims(size_t max_dim, size_t exp_dim, SBG::ORD_REALS& constant, SBG::ORD_REALS& slope);
  
  private:
  typedef std::pair<SBG::SetVertexDesc, Equality> EquationDesc; 
  typedef std::map<std::string, int> Usage;
  typedef std::map<std::string, Usage> EqUsage;

  Modelica::MMO_Class& _mmo_class;
  std::list<SBG::SetVertexDesc> _U;
  std::list<EquationDesc> _F;
  EqUsage _eq_usage;
};

}  // namespace Causalize
