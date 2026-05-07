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

#include "causalize/sbg_implementation/set_vertex.hpp"
#include "causalize/sbg_implementation/equation_info.hpp"

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
  void buildSet(const VarInfo& variable, const Name& name);
  void addVariableNodes();
  void addIndexRange(IndexList range, const std::string& eq_id, int node_id);
  SetVertex indicesDefinition(const IndexList& indices);
  void buildEqualitySet(Equality eq, SetVertex vertex_def);
  void buildForEqSet(ForEq eq, SetVertex vertex_def);
  void addEquationNodes();
  //void addEdges();
  //void addOffset(int edge_id, const std::string& map, int constant, int slope, int dim = -1);
  //void generatePWLMaps(Expression exp, const std::string& eq_id, int edge_id);
  //Integer getMin(const Index& idx) const;
  //Integer getSize(const Index& idx) const;
  //Integer getSize(const IndexList& dom) const;
  //void addEdgeDef(int edge_id, int end, int dim = -1);
  //void generateEdgeMap(const std::string& map_name, const std::string& map_idx, bool fixed_slopes = false);
  void setup();
  void generateSBGInput();

  private:
  Modelica::MMO_Class& _mmo_class;
  std::vector<SetVertex> _set_vertices;
  std::map<int, EquationInfo> _equations_info;
  unsigned int _max_dim;
  int _node_id; ///< Counter for set-vertices
  int _edge_id; ///< Counter for set-edges
  Integer _vertex_offset; ///< Current set-vertex offset
  std::ofstream _sbg_input;
};

} // namespace Causalize

} // namespace Modelica
