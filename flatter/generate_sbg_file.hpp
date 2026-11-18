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

/**
 * @file
 * @brief Constructs the corresponding SBG to calculate connected components.
 */

#ifndef MODELICACC_FLATTER_GENERATE_SBG_FILE_HPP_
#define MODELICACC_FLATTER_GENERATE_SBG_FILE_HPP_

#include "ast/expression.hpp"
#include "mmo/mmo_class.hpp"
#include "util/table.hpp"
#include "util/sbg/equation_info.hpp"
#include "util/sbg/modelica_sbg.hpp"
#include "util/sbg/set_edge.hpp"
#include "util/sbg/set_vertex.hpp"

#include <sbgraph/sbg/expression.hpp>
#include <sbgraph/sbg/sbg.hpp>

#include <fstream>
#include <map>
#include <set>
#include <string>
#include <utility>

namespace Modelica {

namespace Flatter {

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
    , SBG::LIB::SBG sbg);

  const Modelica::MMO_Class& mmo_class() const;
  const ModelicaSBG& modelica_sbg() const;
  const SBG::LIB::SBG& sbg() const;

private:
  const Modelica::MMO_Class& _mmo_class;
  const ModelicaSBG _modelica_sbg;
  const SBG::LIB::SBG _sbg;
};

////////////////////////////////////////////////////////////////////////////////
// Generate SBG Input ----------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

/**
 * @class GenerateSBGInput
 * @brief Given a Modelica MMO_Class it generates a SBG program that defines
 * a bipartite SBG as follows: it adds one vertex for each state variable, and
 * adds an edge between two vertices if there is a connect equation that
 * involves both of them.
 */
class GenerateSBGInput {
public:
  explicit GenerateSBGInput(Modelica::MMO_Class& mmo_class);
  virtual ~GenerateSBGInput() = default;
  virtual SBGGenerationResult buildFromModel();
  virtual std::string fileName();

protected:
  /**
   * @brief Adds variable arrays to _modelica_bsbg.
   *
   * Vertices that belong to singleton CCs are deleted once the SBG is
   * constructed.
   */
  void addSetVertex(const AST::Name& var_name, const VarInfo& var_info);

  /**
   * @brief Creates maps from edges to variables nodes.
   */
  SBG::LIB::Expression createEdgeVarMap(
    const AST::Equation& eq, const SetEdge& eq_se, const SetVertex& var_sv
    , const AST::Expression& access
   ) const;

  std::pair<SetVertex, SetVertex> endpoints(const AST::Equation& eq);

  /**
   * @brief Adds all edges between an equation and all of the occurences of a
   * variable in that equation. It adds a new set-edge for each occurence.
   */
  void addSetEdge(const AST::Equation& eq);

  void setup();
  void addVariableNodes();
  void addSetEdges();

  void generateVSet();
  void generateVMap();
  void generateMap1();
  void generateMap2();
  void generateEMap();
  void generateSBGInput();

private:
  Modelica::MMO_Class& _mmo_class;
  ModelicaSBG _modelica_sbg;
  unsigned int _max_dim;
  std::ofstream _sbg_file;
  AST::EquationList _not_connect; ///< Equations that must remain untouched.
  std::set<AST::Expression> _matched_exprs;
  std::map<int, AST::Expression> _edges_to_left;
  std::map<int, AST::Expression> _edges_to_right;
};

} // namespace Flatter

} // namespace Modelica

#endif // MODELICACC_FLATTER_GENERATE_SBG_FILE_HPP_
