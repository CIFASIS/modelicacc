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

#include "test/performance/scalar/sort_bm.hpp"
#include "test/performance/scalar/build_graphs.hpp"
#include "test/performance/scalar/scalar_utils.hpp"

#include <benchmark/benchmark.h>
#include <boost/graph/topological_sort.hpp>

#include <algorithm>
#include <functional>
#include <iterator>
#include <vector>

namespace Modelica {

namespace test {

namespace scalar {

std::vector<Vertex> topologicalSort(const Graph& g)
{
  std::vector<Vertex> sort;
  sort.reserve(boost::num_vertices(g));
  boost::topological_sort(g, std::back_inserter(sort));
  std::reverse(sort.begin(), sort.end());
  return sort;
}

bool isValidOrder(const Graph& g, const std::vector<Vertex>& result)
{
  if (result.size() != boost::num_vertices(g)) {
    return false;
  }
  std::vector<std::size_t> pos(result.size());
  for (std::size_t k = 0; k < result.size(); ++k) {
    pos[result[k]] = k;
  }
  for (auto e : boost::make_iterator_range(boost::edges(g))) {
    if (pos[boost::source(e, g)] >= pos[boost::target(e, g)]) {
      return false;
    }
  }
  return true;
}

static void BM_TopologicalSort(benchmark::State& state
  , std::function<Graph(int)> builder)
{
  const int N = static_cast<int>(state.range(0));
  const Graph g = builder(N);

  if (!isValidOrder(g, topologicalSort(g))) {
    state.SkipWithError("Invalid sorting");
    return;
  }

  for (auto _ : state) {
    auto sort = topologicalSort(g);
    benchmark::DoNotOptimize(sort.data());
    benchmark::ClobberMemory();
  }

  const auto V = static_cast<int64_t>(boost::num_vertices(g));
  const auto E = static_cast<int64_t>(boost::num_edges(g));

  state.SetComplexityN(V + E);
  state.SetItemsProcessed(state.iterations() * (V + E));
  state.counters["V"] = static_cast<double>(V);
  state.counters["E"] = static_cast<double>(E);
}

static void BM_TopologicalSortWithBuilder(benchmark::State& state
  , std::function<Graph(int)> builder)
{
  const int N = static_cast<int>(state.range(0));
  const Graph g = builder(N);

  if (!isValidOrder(g, topologicalSort(g))) {
    state.SkipWithError("Invalid sorting");
    return;
  }

  for (auto _ : state) {
    const Graph g = builder(N);
    auto sort = topologicalSort(g);
    benchmark::DoNotOptimize(sort.data());
    benchmark::ClobberMemory();
  }

  const auto V = static_cast<int64_t>(boost::num_vertices(g));
  const auto E = static_cast<int64_t>(boost::num_edges(g));

  state.SetComplexityN(V + E);
  state.SetItemsProcessed(state.iterations() * (V + E));
  state.counters["V"] = static_cast<double>(V);
  state.counters["E"] = static_cast<double>(E);
}

void registerSortBenchmark(std::string name
  , std::function<Graph(int)> builder)
{
  benchmark::RegisterBenchmark(name, [builder](benchmark::State& state) {
    BM_TopologicalSort(state, builder);
  })->RangeMultiplier(10)->Range(1e2, 1e6)->Complexity()
    ->Unit(benchmark::kMillisecond);
}

void registerSortBenchmarks()
{
  registerSortBenchmark("BM_TopologicalSortRL1", buildTestRL1Sort);
  registerSortBenchmark("BM_TopologicalSortRL2", buildTestRL2Sort);
  registerSortBenchmark("BM_TopologicalSortRL3", buildTestRL3Sort);
}

} // namespace scalar

} // namespace test

} // namespace Modelica
