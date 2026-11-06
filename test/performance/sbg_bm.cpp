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

#include <benchmark/benchmark.h>
#include "causalize/sbg_implementation/algebraic_loops_detection.hpp"
#include "causalize/sbg_implementation/generate_sbg_input.hpp"
#include "causalize/sbg_implementation/horizontal_sorting.hpp"
#include "causalize/sbg_implementation/tearing.hpp"
#include "causalize/sbg_implementation/vertical_sorting.hpp"
#include "causalize/sbg_implementation/builders/causalization_builders.hpp"
#include "parser/parser.hpp"
#include "test/performance/scalar/mfvs_bm.hpp"
#include "test/performance/scalar/sort_bm.hpp"
#include "util/ast_visitors/state_variables_finder.hpp"

#include <boost/variant/get.hpp>
#include <sbgraph/algorithms/mfvs/min_feedback_vertex_set.hpp>
#include <sbgraph/algorithms/sorting/topological/topological_sorting.hpp>
#include <sbgraph/sbg/directed_sbg.hpp>
#include <sbgraph/sbg/pwmap_impl.hpp>
#include <sbgraph/sbg/set_impl.hpp>

#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>

namespace Modelica {

namespace test {

bool setModelSize(const std::string& model_file, long long size)
{
  std::string content;
  {
    std::ifstream input(model_file, std::ios::binary);
    if (!input) {
      throw std::runtime_error("Unable to open: " + model_file);
    }
    std::ostringstream buffer;
    buffer << input.rdbuf();
    content = buffer.str();
  }

  static const std::regex pattern(R"(\bN[ \t]*=[ \t]*(-?\d+))");

  std::smatch m;
  if (!std::regex_search(content, m, pattern)) {
    return false;
  }

  const auto position = static_cast<std::size_t>(m.position(1));
  const auto length = static_cast<std::size_t>(m.length(1));
  content.replace(position, length, std::to_string(size));

  std::ofstream output(model_file, std::ios::binary | std::ios::trunc);
  if (!output) {
    throw std::runtime_error("Unable to write: " + model_file);
  }
  output << content;
  if (!output) {
    throw std::runtime_error("Error writing: " + model_file);
  }
  return true;
}

Causalize::AlgebraicLoopsResult detectLoops(std::string model_file, int N)
{
  setModelSize(model_file, N);
  bool status = false;
  StoredDef stored_def = Parser::ParseFile(model_file, status);

  Class ast_c = boost::get<Class>(stored_def.classes().front());
  MMO_Class mmo_class(ast_c);

  StateVariablesFinder setup_state_var(mmo_class);
  setup_state_var.findStateVariables();

  std::cout.setstate(std::ios_base::failbit);
  Causalize::GenerateSBGInput gen_sbg_input(mmo_class);
  Causalize::SBGGenerationResult sbg_result = gen_sbg_input.buildFromModel();
  std::cout.clear();

  Causalize::HorizontalSorting horizontal_sorter(sbg_result);
  Causalize::HorizontalSortingResult hs_result = horizontal_sorter.sort();

  Causalize::AlgebraicLoopsDetector loops_detector(hs_result);
  Causalize::AlgebraicLoopsResult loops_result = loops_detector.detect();
  return loops_result;
}

SBG::LIB::DirectedSBG buildMFVSBG(std::string model_file, int N)
{
  Causalize::AlgebraicLoopsResult loops_result = detectLoops(model_file, N); 
  SBG::LIB::DirectedSBG dsbg = Causalize::buildTearingSBG(
    loops_result.scc_result()
  );
  return dsbg;
}

SBG::LIB::TSInput buildSortSBG(std::string model_file, int N)
{
  Causalize::AlgebraicLoopsResult loops_result = detectLoops(model_file, N);
  Causalize::TearingDetector tearing_detector(loops_result);
  Causalize::TearingResult tearing_result = tearing_detector.detect();

  Causalize::VerticalSorting vertical_sorter{loops_result, tearing_result};
  Causalize::VerticalSortingBuilder vs_builder{
    loops_result.scc_result(), tearing_result.mfvs_result()
  };
  vs_builder.build();

  SBG::LIB::TSInput input;
  SBG::LIB::DirectedSBG dsbg = vs_builder.dsbg();
  input.set_dsbg(dsbg);
  input.set_rmap(vs_builder.rmap());
  input.set_rmap_ends(vs_builder.reps_to_endpoints());
  input.set_rmap_starts(
    vs_builder.guess_offset().composition(vs_builder.reps_to_endpoints())
  );
  return input;
}

static void SBG_BM_MFVS(benchmark::State& state, std::string filename)
{
  const int N = static_cast<int>(state.range(0));
  const SBG::LIB::DirectedSBG dsbg = buildMFVSBG(filename, N);

  for (auto _ : state) {
    SBG::LIB::Set fvs = SBG::LIB::MinFeedbackVertexSet{}.calculate(dsbg);
    benchmark::DoNotOptimize(fvs);
    benchmark::ClobberMemory();
  }

  state.SetComplexityN(N);
}

static void SBG_BM_Sort(benchmark::State& state, std::string filename)
{
  const int N = static_cast<int>(state.range(0));
  const SBG::LIB::TSInput input = buildSortSBG(filename, N);

  for (auto _ : state) {
    SBG::LIB::PWMap sort = SBG::LIB::TopologicalSorting{}.calculate(input);
    benchmark::DoNotOptimize(sort);
    benchmark::ClobberMemory();
  }

  state.SetComplexityN(N);
}

void registerSBGMFVSBenchmark(std::string filename)
{
  benchmark::RegisterBenchmark(("SBG_BM_MFVS/" + filename).c_str()
    , [filename](benchmark::State& state) {
      SBG_BM_MFVS(state, filename);
    })->RangeMultiplier(10)->Range(1e2, 1e6)->Complexity()
    ->Unit(benchmark::kMillisecond);
}

void registerSBGSortBenchmark(std::string filename)
{
  benchmark::RegisterBenchmark(("SBG_BM_Sort/" + filename).c_str()
    , [filename](benchmark::State& state) {
      SBG_BM_Sort(state, filename);
    })->RangeMultiplier(10)->Range(1e2, 1e6)->Complexity()
    ->Unit(benchmark::kMillisecond);
}

void registerSBGBenchmarks()
{
  SBG::LIB::SET_IMPL.set_set_fact(SBG::LIB::SetKind::kOrdUnidimDense);
  SBG::LIB::PWMAP_IMPL.set_pwmap_fact(SBG::LIB::PWMapKind::kUnordered);

  registerSBGMFVSBenchmark("../test/mccprograms/TestRL1.mo");
  registerSBGMFVSBenchmark("../test/mccprograms/TestRL3.mo");

  registerSBGSortBenchmark("../test/mccprograms/TestRL1.mo");
  registerSBGSortBenchmark("../test/mccprograms/TestRL2.mo");
  registerSBGSortBenchmark("../test/mccprograms/TestRL3.mo");
}

} // namespace test

} // namespace Modelica
