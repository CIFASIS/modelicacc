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
  Integer getValue(Expression expr) const;
  void buildSet(const VarInfo& variable, const Name& name);
  void buildEqualitySet(Equality eq, SetVertex vertex_def);
  void buildForEqSet(ForEq eq, SetVertex vertex_def);

  void generateExpression(const SetVertex& sv, const Expression& expr
    , EquationInfo& eq_info, CompactSet domain);

  void setup();
  void addVariableNodes();
  void addEquationNodes();
  void addEdges();
  void generateSBGInput();

private:
  Modelica::MMO_Class& _mmo_class;
  unsigned int _max_dim;
  std::vector<SetVertex> _set_vertices;
  std::map<int, EquationInfo> _equations_info;
  int _node_id; ///< Counter for set-vertices
  int _edge_id; ///< Counter for set-edges
  Integer _vertex_offset; ///< Current vertex offset
  Integer _edge_offset; ///< Current edge offset
  std::ofstream _sbg_input;
};

} // namespace Causalize

} // namespace Modelica
