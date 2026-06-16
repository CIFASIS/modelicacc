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

#ifndef MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_ALGEBRAIC_LOOPS_DETECTION_HPP_
#define MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_ALGEBRAIC_LOOPS_DETECTION_HPP_

#include "ast/equation.hpp"
#include "ast/expression.hpp"
#include "causalize/sbg_implementation/horizontal_sorting.hpp"

#include <algorithms/scc/scc_data.hpp>

#include <iosfwd>
#include <vector>

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// Auxiliary definitions -------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

class AlgebraicLoop {
public:
  AlgebraicLoop() = default;
  AlgebraicLoop(EquationList equations, ExpList variables);

  const EquationList& equations() const;
  const ExpList& variables() const;
  bool isEmpty() const;

  void pushBack(Equation eq, Expression variable);

private:
  EquationList _equations;
  ExpList _variables;
};

std::ostream& operator<<(std::ostream& out, const AlgebraicLoop& loop);

class AlgebraicLoops {
public:
  AlgebraicLoops() = default;

  std::size_t size() const;
  AlgebraicLoop operator[](std::size_t k) const;
  auto begin() const { return _loops.begin(); };
  auto end() const { return _loops.end(); };

  void pushBack(AlgebraicLoop loop);
  void reverse();

private:
  std::vector<AlgebraicLoop> _loops;
};

std::ostream& operator<<(std::ostream& out, const AlgebraicLoops& loops);

class AlgebraicLoopsInfo {
public:
  AlgebraicLoopsInfo(const std::vector<SetVertex>& set_vertices
    , const std::vector<SetEdge>& set_edges, SBG::LIB::SCCData scc_result
    , AlgebraicLoops loops);

  const std::vector<SetVertex>& set_vertices() const;
  const std::vector<SetEdge>& set_edges() const;
  const SBG::LIB::SCCData& scc_result() const;
  const AlgebraicLoops& loops() const;

private:
  const std::vector<SetVertex>& _set_vertices;
  const std::vector<SetEdge>& _set_edges;
  SBG::LIB::SCCData _scc_result;
  AlgebraicLoops _loops;
};

////////////////////////////////////////////////////////////////////////////////
// Algebraic Loops Detection ---------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

/**
 * @class AlgebraicLoopsDetector
 * @brief Identifies the algebraic loops of the model. 
 */
class AlgebraicLoopsDetector {
public:
  AlgebraicLoopsDetector(HorizontalSortingInfo& sbg_generator);

  AlgebraicLoopsInfo detect();

private:
  void detectLoop(SetEdge se, AlgebraicLoop& loop);

  HorizontalSortingInfo& _hs_info;
  AlgebraicLoops _loops;
};

} // namespace Causalize

} // namespace Modelica

#endif // MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_ALGEBRAIC_LOOPS_DETECTION_HPP_
