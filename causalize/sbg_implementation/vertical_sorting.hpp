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

#include "ast/equation.hpp"
#include "ast/expression.hpp"
#include "causalize/sbg_implementation/algebraic_loops_detection.hpp"
#include "causalize/sbg_implementation/tearing.hpp"

#include <iosfwd>
#include <vector>

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// ModelicaCC causalized model -------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

/**
 * @class CausalEquations
 * @brief Group of causalized equations that must be solved together.
 */
class CausalEquations {
public:
  CausalEquations() = default;
  CausalEquations(EquationList equations, ExpList variables);

  const EquationList& equations() const;
  const ExpList& variables() const;
  bool isEmpty() const;

  void pushBack(Equation eq, Expression variable);

private:
  EquationList _equations;
  ExpList _variables;
};

std::ostream& operator<<(std::ostream& out, const CausalEquations& causal_eqs);

/**
 * @class CausalModel
 * @brief 
 */
class CausalModel {
public:
  CausalModel() = default;

  std::size_t size() const;
  CausalEquations operator[](std::size_t k) const;

  auto begin() const { return _causal_eqs.begin(); }
  auto end() const { return _causal_eqs.end(); }

  void pushBack(CausalEquations eqs);

private:
  std::vector<CausalEquations> _causal_eqs;
};

std::ostream& operator<<(std::ostream& out, const CausalModel& causal_model);

////////////////////////////////////////////////////////////////////////////////
// Vertical sorting result -----------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

class VerticalSortingResult {
public:
  VerticalSortingResult(ModelicaSBG modelica_bsbg, SBG::LIB::PWMap sort);

  const ModelicaSBG& modelica_bsbg() const;
  const SBG::LIB::PWMap& sort() const;

  /**
   * @brief Converts the SBG obtained result to a ModelicaCC list of equations
   * and expressions that indicate the horizontal sorting, and are grouped
   * accordingly to represent algebraic loops. Additionally, the returned list
   * has algebraic loops ordered (still there is no order inside each
   * algebraic loop).
   */
  CausalModel toModelicaFormat(std::vector<LoopT> loops) const;

private:
  std::vector<LoopT> sortLoops(const std::vector<LoopT>& loops) const;
  CausalEquations causalizeLoop(LoopT loop) const;

  ModelicaSBG _modelica_bsbg;
  SBG::LIB::PWMap _sort;
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
