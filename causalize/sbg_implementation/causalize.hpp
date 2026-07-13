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

#ifndef MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_CAUSALIZE_HPP_
#define MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_CAUSALIZE_HPP_

#include "causalize/sbg_implementation/algebraic_loops_detection.hpp"
#include "causalize/sbg_implementation/generate_sbg_input.hpp"
#include "causalize/sbg_implementation/horizontal_sorting.hpp"
#include "causalize/sbg_implementation/tearing.hpp"
#include "causalize/sbg_implementation/vertical_sorting.hpp"
#include "mmo/mmo_class.hpp"

#include <iosfwd>

namespace Modelica {

namespace Causalize {

class CausalizationResult {
public:
  CausalizationResult() = default;
  CausalizationResult(ModelMatch horizontal_sort, AlgebraicLoops loops
    , TearingVariables tearing, CausalModel vertical_sort);

  const ModelMatch& horizontal_sort() const;
  const AlgebraicLoops& loops() const;
  const TearingVariables& tearing() const;
  const CausalModel& vertical_sort() const;

private:
  ModelMatch _horizontal_sort;
  AlgebraicLoops _loops;
  TearingVariables _tearing;
  CausalModel _vertical_sort;
};

std::ostream& operator<<(std::ostream& out
  , const CausalizationResult& causalized);

class Causalize {
public:
  Causalize() = default;

  CausalizationResult causalize(MMO_Class& mmo_class) const;
};

} // namespace Causalize

} // namespace Modelica

#endif // MODELICACC_CAUSALIZE_SBG_IMPLEMENTATION_CAUSALIZE_HPP_
