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

#include "causalize/sbg_implementation/tearing.hpp"
#include "causalize/sbg_implementation/set_edge.hpp"
#include "causalize/sbg_implementation/set_vertex.hpp"
#include "causalize/sbg_implementation/builders/causalization_builders.hpp"
#include "util/debug.hpp"

#include <algorithms/mfvs/min_feedback_vertex_set.hpp>
#include <sbg/directed_sbg.hpp>
#include <sbg/map.hpp>
#include <sbg/set.hpp>
#include <util/time_profiler.hpp>

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// Tearing return structure ----------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

TearingResult::TearingResult(
  ModelicaSBG modelica_bsbg, SBG::LIB::Set mfvs_result
) : _modelica_bsbg(modelica_bsbg), _mfvs_result(mfvs_result) {}

const ModelicaSBG& TearingResult::modelica_bsbg() const
{
  return _modelica_bsbg;
}

const SBG::LIB::Set& TearingResult::mfvs_result() const
{
  return _mfvs_result;
}

TearingVariables TearingResult::variableToModelicaFormat(
  const SetEdge& se, SBG::LIB::Set se_tear
) const
{
  TearingVariables result;

  // Get variable information that is accessed by the set-edge.
  SetVertex var_sv = _modelica_bsbg.setVertex(se.var_id());
  AST::Name name = var_sv.name();
  SBG::LIB::IntTuple var_translation = var_sv.translation();

  // Convert se_tear to bracket expression and save it to result.
  SBG::LIB::Set img{SBG::LIB::Map{se_tear, se.map2()}.image()};
  Accesses accesses = toModelicaIndices(
    img, var_translation, std::vector<AST::Name>{var_sv.arity(), ""}
  );
  for (const auto& [_, indexes] : accesses) {
    AST::ExpList expr_list;
    for (const AST::Index& index : indexes.indexes()) {
      ERROR_UNLESS(index.exp().has_value()
        , "TearingDetector::variableToModelica: empty index");
      expr_list.push_back(index.exp().value());
    }
    AST::Bracket subscripts{ExpListList{1, expr_list}};
    result.insert(TearingVariable{name, subscripts});
  }

  return result;
}

TearingVariables TearingResult::toModelicaFormat() const
{
  TearingVariables result;

  for (const SetEdge& se : _modelica_bsbg.set_edges()) {
    SBG::LIB::Set se_tear = se.translatedDomain().intersection(_mfvs_result);
    if (se_tear.cardinal() > 0) {
      result.concatenation(variableToModelicaFormat(se, se_tear));
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
// Tearing variables detector --------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

TearingDetector::TearingDetector(AlgebraicLoopsResult loops_result)
  : _loops_result(loops_result) {}

TearingResult TearingDetector::detect()
{
  SBG::LIB::DirectedSBG dsbg;
  {
    SBG::Util::Internal::TimeProfiler profiler{"Tearing SBG builder"};
    dsbg = buildTearingSBG(_loops_result.scc_result());
  }
  SBG::LIB::Set mfvs;
  {
    SBG::Util::Internal::TimeProfiler profiler{"Tearing"};
    mfvs = SBG::LIB::MinFeedbackVertexSet{}.calculate(dsbg);
  }
  return TearingResult{_loops_result.modelica_bsbg(), mfvs};
}

}  // namespace Causalize

}  // namespace Modelica
