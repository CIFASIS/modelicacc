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

#include "test/performance/scalar/mfvs_bm.hpp"
#include "test/performance/scalar/build_graphs.hpp"
#include "test/performance/scalar/mfvs.hpp"
#include "test/performance/scalar/scalar_utils.hpp"

#include <benchmark/benchmark.h>

#include <algorithm>
#include <functional>
#include <iterator>
#include <vector>

namespace Modelica {

namespace test {

namespace scalar {

static void BM_MFVS(benchmark::State& state
  , std::function<Graph(int)> builder)
{
  const int N = static_cast<int>(state.range(0));
  const Graph g = builder(N);

  for (auto _ : state) {
    auto fvs = heuristicFVS(g);
    benchmark::DoNotOptimize(fvs.data());
    benchmark::ClobberMemory();
  }

  const auto V = static_cast<int64_t>(boost::num_vertices(g));
  const auto E = static_cast<int64_t>(boost::num_edges(g));

  state.SetComplexityN(V + E);
  state.SetItemsProcessed(state.iterations() * (V + E));
  state.counters["V"] = static_cast<double>(V);
  state.counters["E"] = static_cast<double>(E);
}

static void BM_MFVSWithBuilder(benchmark::State& state
  , std::function<Graph(int)> builder)
{
  const int N = static_cast<int>(state.range(0));
  const Graph g = builder(N);

  for (auto _ : state) {
    const Graph g = builder(N);
    auto fvs = heuristicFVS(g);
    benchmark::DoNotOptimize(fvs.data());
    benchmark::ClobberMemory();
  }

  const auto V = static_cast<int64_t>(boost::num_vertices(g));
  const auto E = static_cast<int64_t>(boost::num_edges(g));

  state.SetComplexityN(V + E);
  state.SetItemsProcessed(state.iterations() * (V + E));
  state.counters["V"] = static_cast<double>(V);
  state.counters["E"] = static_cast<double>(E);
}

void registerMFVSBenchmark(std::string name
  , std::function<Graph(int)> builder)
{
  benchmark::RegisterBenchmark(name, [builder](benchmark::State& state) {
    BM_MFVS(state, builder);
  })->RangeMultiplier(10)->Range(1e2, 1e4)->Complexity()
    ->Unit(benchmark::kMillisecond);
}

void registerMFVSBenchmarks()
{
  registerMFVSBenchmark("BM_MFVSRL1", buildTestRL1MFVS);
  registerMFVSBenchmark("BM_MFVSRL3", buildTestRL3MFVS);
}

} // namespace scalar

} // namespace test

} // namespace Modelica
