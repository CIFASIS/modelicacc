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

#include "causalize/sbg_implementation/vertex_definition.hpp"

#include <fstream>
#include <iostream>

#include <mmo/mmo_class.hpp>

namespace Modelica {

namespace Causalize {

class GenerateSBGInput {
  public:
  explicit GenerateSBGInput(Modelica::MMO_Class& mmo_class);
  virtual ~GenerateSBGInput() = default;
  virtual void buildFromModel();
  virtual std::string fileName();

  protected:
  Integer getValue(Expression exp) const;
  void buildSet(const VarInfo& variable);
  void addVariableNodes();
  void addIndexRange(IndexList range, const std::string& eq_id, int node_id);
  VertexDefinition indicesDefinition(const IndexList& indices);
  void buildEqualitySet(Equality eq);
  void buildForEqSet(ForEq eq, VertexDefinition vertex_def);
  //void addEquationInfo(const std::string& eq_name, Equality eq, IndexList indexes, int node_id);
  void addEquationNodes();
  void addEdges();
  void addOffset(int edge_id, const std::string& map, int constant, int slope, int dim = -1);
  void generatePWLMaps(Expression exp, const std::string& eq_id, int edge_id);
  Integer getMin(const Index& idx) const;
  Integer getSize(const Index& idx) const;
  Integer getSize(const IndexList& dom) const;
  void addEdgeDef(int edge_id, int end, int dim = -1);
  void generateEdgeMap(const std::string& map_name, const std::string& map_idx, bool fixed_slopes = false);
  void setup();
  void generateSBGInput();

  private:
  using Usage = std::map<std::string, int, std::less<>>;
  using EqUsage = std::map<std::string, Usage, std::less<>>;

  Modelica::MMO_Class& _mmo_class;
  std::list<std::string> _V;
  std::list<std::string> _E;
  std::map<std::string, Equality, std::less<>> _eqs;
  std::map<std::string, IndexList, std::less<>> _eq_range;
  std::map<std::string, int, std::less<>> _var_nodes;
  std::map<std::string, int, std::less<>> _eq_nodes;
  EqUsage _eq_usage;
  std::list<std::string> _V_map;
  std::list<std::string> _offsets;
  std::ofstream _sbg_input;
  std::vector<int> _m1_slopes;
  unsigned long _max_dim;
  int _node_id;
  int _edge_id;
  Integer _vertex_offset;
};

} // namespace Causalize

} // namespace Modelica
