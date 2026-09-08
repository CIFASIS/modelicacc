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

#ifndef MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_VERTICAL_SORTING_HPP_
#define MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_VERTICAL_SORTING_HPP_

#include "ast/expression.hpp"
#include "causalize/sbg_implementation/algebraic_loops_detection.hpp"
#include "causalize/sbg_implementation/causal_model.hpp"
#include "causalize/sbg_implementation/equation_info.hpp"
#include "causalize/sbg_implementation/modelica_sbg.hpp"
#include "causalize/sbg_implementation/tearing.hpp"
#include "causalize/sbg_implementation/builders/causalization_builders.hpp"
#include "util/table.hpp"

#include <sbg/set.hpp>

#include <iosfwd>
#include <tuple>
#include <utility>
#include <vector>

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// ModelicaSBG modifier --------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Handles the addition of guess and residual vertices in Modelica
 * format. 
 */
class ModelicaSBGModifier {
public:
  ModelicaSBGModifier() = default;

  const ModelicaSBG& modelica_bsbg() const;
  const std::vector<std::pair<AST::Name, VarInfo>>& added_variables() const;

  ModelicaSBG modify(
    ModelicaSBG modelica_bsbg, const VerticalSortingBuilder& builder
  );

private:
  void partition(const SBG::LIB::Set& not_residual);

  void addGuess(
    const SetEdge& se, const CompactSet& se_res
    , const SBG::LIB::MD_NAT& max_elem
  );
  void addGuessMatchs(const SBG::LIB::MD_NAT& max_elem);

  void modifyResidual(SetEdge& se, const CompactSet& se_res);
  void modifyResidualMatchs();

  void addVarsDeclarations();

  ModelicaSBG _input_modelica_bsbg;
  ModelicaSBG _output_modelica_bsbg;
  std::vector<std::pair<AST::Name, VarInfo>> _added_variables;
  SBG::LIB::Set _residual_vertices;
};

////////////////////////////////////////////////////////////////////////////////
// Converter to Modelica code of an equation/variable pairing ------------------
////////////////////////////////////////////////////////////////////////////////

namespace detail {

using SortedMatch = std::pair<EquationInfo, AST::Expression>;
using SortedMatchs = std::vector<SortedMatch>;

} // namespace detail

class MatchToModelicaFormat {
public:
  MatchToModelicaFormat(
    const ModelicaSBG& modelica_bsbg, const SBG::LIB::PWMap& sort
    , const VerticalSortingBuilder& builder, const SBG::LIB::Map& jth_scc_smap
  );

  detail::SortedMatchs format(
    const SBG::LIB::Map& match, const SBG::LIB::Expression& expr
  );

private:
  detail::SortedMatchs equationToModelicaFormat(
    const SetEdge& se, const CompactSet& se_match
    , const SBG::LIB::Expression& expr
  );

  ModelicaSBG _modelica_bsbg;
  SBG::LIB::PWMap _sort;
  VerticalSortingBuilder _builder;
  SBG::LIB::Map _jth_scc_smap;
  detail::SortedMatchs _residual_zero;
};

////////////////////////////////////////////////////////////////////////////////
// Vertical sorting result -----------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

class VerticalSortingResult {
public:
  VerticalSortingResult(
    const ModelicaSBG& modelica_bsbg, const SBG::LIB::PWMap& sort
    , const VerticalSortingBuilder& builder
  );

  const ModelicaSBG& modelica_bsbg() const;
  const SBG::LIB::PWMap& sort() const;
  const std::vector<std::pair<AST::Name, VarInfo>> added_variables() const;

  /**
   * @brief Converts the SBG obtained result to a ModelicaCC list of equations
   * and expressions that indicate the horizontal sorting, and are grouped
   * accordingly to represent vertically sorted algebraic loops, after applying
   * tearing.
   */
  CausalModel toModelicaFormat();

private:
  /**
   * @brief Partitions _sort so that the domain of each map only contains
   * elements of the same set-edge, i.e., same array of equations.
   */
  void partitionSort();

  SortedAlgebraicLoop outerBounds(
    const detail::SortedMatchs& matchs, bool is_scalar
  ) const;

  /**
   * @brief Sorts inside a single algebraic loop, or array of algebraic loops.
   */
  SortedAlgebraicLoop sortLoop(const SBG::LIB::Map& jth_scc_smap) const;

  /**
   * @brief Sorts between different algebraic loops or different arrays of
   * algebraic loops. It is equivalent to decide the order of different SCCs
   * in the directed SBG.
   */
  CausalModel sortLoops() const;

  ModelicaSBG _modelica_bsbg;
  SBG::LIB::PWMap _sort;
  VerticalSortingBuilder _builder;
  std::vector<std::pair<AST::Name, VarInfo>> _added_variables;
};

////////////////////////////////////////////////////////////////////////////////
// Vertical sorting ------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

/**
 * @class VerticalSorting
 * @brief Class that orders vertically the algebraic loops of the model.
 */
class VerticalSorting {
public:
  VerticalSorting(AlgebraicLoopsResult& loops_result
    , TearingResult& tearing_result);

  VerticalSortingResult sort();

private:
  AlgebraicLoopsResult& _loops_result;
  TearingResult& _tearing_result;
};

} // namespace Causalize

} // namespace Modelica

#endif // MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_VERTICAL_SORTING_HPP_
