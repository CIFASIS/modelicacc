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

#ifndef MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_GENERATE_SBG_INPUT_HPP_
#define MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_GENERATE_SBG_INPUT_HPP_

#include "causalize/sbg_implementation/modelica_sbg.hpp"
#include "causalize/sbg_implementation/set_edge.hpp"
#include "causalize/sbg_implementation/set_vertex.hpp"
#include "mmo/mmo_class.hpp"
#include "util/compact_set.hpp"

#include "sbg/bipartite_sbg.hpp"

#include <fstream>
#include <iostream>
#include <string>

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// SBG generation return structure ---------------------------------------------
////////////////////////////////////////////////////////////////////////////////

/*
 * @class SBGGenerationResult
 * @brief Structure to keep and return relevant data after the SBG associated
 * to a model has been generated.
 */
class SBGGenerationResult {
public:
  explicit SBGGenerationResult(Modelica::MMO_Class& mmo_class
    , ModelicaSBG modelica_bsbg
    , SBG::LIB::BipartiteSBG bipartite_sbg);

  const Modelica::MMO_Class& mmo_class() const;
  const ModelicaSBG& modelica_bsbg() const;
  const SBG::LIB::BipartiteSBG& bipartite_sbg() const;

private:
  const Modelica::MMO_Class& _mmo_class;
  const ModelicaSBG _modelica_bsbg;
  const SBG::LIB::BipartiteSBG _bipartite_sbg;
};

////////////////////////////////////////////////////////////////////////////////
// Generate SBG Input ----------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

/**
 * @class GenerateSBGInput
 * @brief Given a Modelica MMO_Class it generates a SBG program that defines
 * a bipartite SBG as follows: it adds one vertex for each state variable
 * (right) and equation (left), and then adds an edge between a variable and an
 * equation if it is one of its unknowns.
 */
class GenerateSBGInput {
public:
  explicit GenerateSBGInput(Modelica::MMO_Class& mmo_class);
  virtual ~GenerateSBGInput() = default;
  virtual SBGGenerationResult buildFromModel();
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
  CompactTransformation createMap2(const Reference& reference
    , const IndexList& counters, const Translation& var_trans) const;

  /**
   * @brief Adds maps definitions to a set-edge, and saves the element to
   * _set_edges.
   */
  void addMaps(SetEdge se, const SetVertex& eq_sv, const SetVertex& var_sv
    , const Reference& reference);

  /**
   * @brief Adds all edges between an equation and all of the occurences of a
   * variable in that equations. It adds a set-edge for each occurence.
   */
  void addEdge(const SetVertex& eq_sv, const SetVertex& sv
    , const EquationInfo& eq_info);

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
  SetVertices _set_vertices;
  SetEdges _set_edges;
  int _node_id; ///< Counter for set-vertices
  int _edge_id; ///< Counter for set-edges
  Integer _vertex_offset; ///< Current vertex offset
  Integer _edge_offset; ///< Current edge offset
  std::ofstream _sbg_input;
};

} // namespace Causalize

} // namespace Modelica

#endif // MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_GENERATE_SBG_INPUT_HPP_
