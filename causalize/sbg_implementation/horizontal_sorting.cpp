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

#include "causalize/sbg_implementation/horizontal_sorting.hpp"
#include "causalize/sbg_implementation/equation_info.hpp"
#include "causalize/sbg_implementation/set_vertex.hpp"
#include "util/debug.hpp"

#include <algorithms/matching/match_data.hpp>
#include <algorithms/matching/matching.hpp>
#include <boost/variant/get.hpp>
#include <sbg/bipartite_sbg.hpp>
#include <util/time_profiler.hpp>

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// Horizontal sorting return structure -----------------------------------------
////////////////////////////////////////////////////////////////////////////////

// Constructors/Destructors ----------------------------------------------------

HorizontalSortingResult::HorizontalSortingResult(
  ModelicaSBG modelica_bsbg, SBG::LIB::MatchData matching_result
) : _modelica_bsbg(modelica_bsbg), _matching_result(matching_result) {}

// Getters ---------------------------------------------------------------------

const ModelicaSBG& HorizontalSortingResult::modelica_bsbg() const
{
  return _modelica_bsbg;
}

const SBG::LIB::MatchData& HorizontalSortingResult::matching_result() const
{
  return _matching_result;
}

// Extra functions -------------------------------------------------------------

ModelMatch HorizontalSortingResult::equationToModelicaFormat(
  SetEdge se, CompactSet se_match
) const
{
  auto [eq_info, accesses] = getAccess(_modelica_bsbg, se, se_match);

  // Create new equation and save it to current matching.
  ModelMatch result;
  for (const auto& [_, indexes] : accesses) {
    result.pushBack(
      EqVarMatch{eq_info.restrictBounds(indexes), ExpList{se.access()}}
    );
  }
  return result;
}

ModelMatch HorizontalSortingResult::toModelicaFormat() const
{
  ModelMatch result;

  // Traverse set-edges to get equation-variable matching.
  CompactSet sbg_match{_matching_result.M()};
  for (const SetEdge& se : _modelica_bsbg.set_edges()) {
    CompactSet jth_eq_var_match = se.translatedDomain();
    jth_eq_var_match.intersection(sbg_match);
    if (jth_eq_var_match.cardinal() > 0) {
      result.concatenation(equationToModelicaFormat(se, jth_eq_var_match));
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
// Horizontal sorting ----------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

HorizontalSorting::HorizontalSorting(SBGGenerationResult& sbg_generation_result)
  : _sbg_generation_result(sbg_generation_result) {}

namespace {

/**
 * @brief Constructs the new Modelica bipartite SBG with only matched edges,
 * that are the relevant ones for latter stages of the causalization. This
 * takes into account the partition of set-edges induced by the matching, and
 * reduces lookup times in the following phases.
 */
ModelicaSBG constructNewModelicaSBG(
  const ModelicaSBG& old, SBG::LIB::MatchData matching_result
)
{
  ModelicaSBG result;

  // Add all vertices.
  const SetVertices& old_svs = old.set_vertices();
  for (const SetVertex& sv : old_svs) {
    result.addSetVertex(sv);
  }

  // Leave only matched edges in the SBG.
  const SetEdges& old_ses = old.set_edges();
  CompactSet M{matching_result.M()};
  for (const SetEdge& se : old_ses) {
    result.addSetEdge(se.restrict(M));
  }

  return result;
}

}  // namespace

HorizontalSortingResult HorizontalSorting::sort()
{
  // Get SBG matching result
  const SBG::LIB::BipartiteSBG& bipartite_sbg
    = _sbg_generation_result.bipartite_sbg();
  unsigned int num_variables = bipartite_sbg.Y().cardinal();
  unsigned int num_equations = bipartite_sbg.X().cardinal();
  ERROR_UNLESS(num_variables == num_equations
    , "HorizontalSorting::sort: unbalanced system of equations.\n"
    , "Number of variables: ", num_variables, "\n"
    , "Number of equations: ", num_equations);

  SBG::LIB::MatchData matching_result{
    SBG::LIB::BipartiteSBG{}, SBG::LIB::Set{}, false
  };
  {
    SBG::Util::Internal::TimeProfiler profiler{"Horizontal sorting"};
    matching_result = SBG::LIB::Matching{}.calculate(bipartite_sbg);
  }
  ERROR_UNLESS(matching_result.full_match(), "HorizontalSorting::sort: "
    , "higher index system");

  const ModelicaSBG& old = _sbg_generation_result.modelica_bsbg();
  return HorizontalSortingResult{
    constructNewModelicaSBG(old, matching_result), matching_result
  };
}

}  // namespace Causalize

}  // namespace Modelica
