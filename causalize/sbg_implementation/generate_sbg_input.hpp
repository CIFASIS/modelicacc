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

#include "causalize/sbg_implementation/equation_info.hpp"
#include "util/hyper_rectangle.hpp"
#include "causalize/sbg_implementation/set_edge.hpp"
#include "causalize/sbg_implementation/set_vertex.hpp"

#include <fstream>
#include <iostream>
#include <string>

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
  void addVariableSet(const VarInfo& variable, const Name& name);

  /**
   * @brief Generates an equivalent list of equations to that of
   * _mmo_class.equations().equations() where each loop has an unique
   * inner-most equation.
   */
  EquationList flatterForEqs() const;

  /**
   * @brief Creates maps from edges to equations nodes.
   */
  CompactTransformation createMap1(const CompactSet& eq_nodes
    , const Translation& eq_nodes_trans) const;

  /**
   * @brief Creates maps from edges to variables nodes.
   */
  CompactTransformation createMap2(const Expression& expr
    , const IndexList& counters, const Translation& var_trans) const;
  void addMaps(std::string name, CompactSet eq_nodes, CompactTransformation map1
    , CompactTransformation map2);

  void setup();
  void addVariableNodes();
  void addEquationNodes();
  void addEdges();

  void generateVSet();
  void generateVMap();
  void generateMap1();
  void generateMap2();
  void generateEMap();
  void generateSBGInput();
  void generatePartition();

private:
  Modelica::MMO_Class& _mmo_class;
  unsigned int _max_dim;
  std::vector<SetVertex> _set_vertices;
  std::map<int, EquationInfo> _equations_info;
  std::vector<SetEdge> _set_edges;
  int _node_id; ///< Counter for set-vertices
  int _edge_id; ///< Counter for set-edges
  Integer _vertex_offset; ///< Current vertex offset
  Integer _edge_offset; ///< Current edge offset
  std::ofstream _sbg_input;
};

} // namespace Causalize

} // namespace Modelica
