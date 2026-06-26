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

#include "causalize/sbg_implementation/causalize.hpp"

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// Causalization result --------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

CausalizationResult::CausalizationResult(ModelMatch horizontal_sort, AlgebraicLoops loops, TearingVariables tearing,
                                         CausalModel vertical_sort)
    : _horizontal_sort(horizontal_sort), _loops(loops), _tearing(tearing), _vertical_sort(vertical_sort)
{
}

const ModelMatch& CausalizationResult::horizontal_sort() const { return _horizontal_sort; }

const AlgebraicLoops& CausalizationResult::loops() const { return _loops; }

const TearingVariables& CausalizationResult::tearing() const { return _tearing; }

const CausalModel& CausalizationResult::vertical_sort() const { return _vertical_sort; }

std::ostream& operator<<(std::ostream& out, const CausalizationResult& causalized)
{
  out << "Horizontal sort:\n";
  out << causalized.horizontal_sort() << "\n";
  out << "Algebraic loops:\n";
  out << causalized.loops() << "\n";
  out << "Tearing:\n";
  out << causalized.tearing() << "\n";
  out << "Vertical sort:\n";
  out << causalized.vertical_sort() << "\n";

  return out;
}

////////////////////////////////////////////////////////////////////////////////
// Causalization process -------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

CausalizationResult Causalize::causalize(MMO_Class& mmo_class) const
{
  GenerateSBGInput gen_sbg_input(mmo_class);
  SBGGenerationResult sbg_result = gen_sbg_input.buildFromModel();

  HorizontalSorting horizontal_sorter(sbg_result);
  HorizontalSortingResult hs_result = horizontal_sorter.sort();
  ModelMatch horizontal_sort = hs_result.toModelicaFormat();

  AlgebraicLoopsDetector loops_detector(hs_result);
  AlgebraicLoopsResult loops_result = loops_detector.detect();
  AlgebraicLoops loops = loops_result.toModelicaFormat();

  TearingDetector tearing_detector(loops_result);
  TearingResult tearing_result = tearing_detector.detect();
  TearingVariables tearing = tearing_result.toModelicaFormat();

  VerticalSorting vertical_sorter{loops_result, tearing_result};
  VerticalSortingResult vs_result = vertical_sorter.sort();
  CausalModel vertical_sort = vs_result.toModelicaFormat(loops_result.toSBGFormat());

  return CausalizationResult{horizontal_sort, loops, tearing, vertical_sort};
}

}  // namespace Causalize

}  // namespace Modelica
