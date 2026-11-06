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

#include "test/performance/scalar/mfvs.hpp"
#include "test/performance/scalar/scalar_utils.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/strong_components.hpp>
#include <boost/range/iterator_range.hpp>

#include <algorithm>
#include <cstddef>
#include <limits>
#include <numeric>
#include <vector>

namespace Modelica {

namespace test {

namespace scalar {

namespace detail {

MFVSAlgorithm::MFVSAlgorithm(const Graph& g, std::vector<Vertex> vertices)
  : _globals(std::move(vertices)), _n(static_cast<int>(vertices.size()))
    , _out(_n), _in(_n), _loop(_n, 0), _alive(_n, 1), _input_degree(_n, 0)
    , _output_degree(_n, 0)
{
  std::vector<int> local(boost::num_vertices(g), -1);
  for (int i = 0; i < _n; ++i) {
    local[_globals[i]] = i;
  }

  for (int i = 0; i < _n; ++i) {
    auto glob = _globals[i];
    for (auto e : boost::make_iterator_range(boost::out_edges(glob, g))) {
      const int j = local[boost::target(e, g)];
      if (j < 0) { // Out of subgraph.
        continue;
      }
      if (j == i) { // Self loops are treated separetely.
        _loop[i] = 1;
        continue;
       }
      _out[i].push_back(j);
      _in[j].push_back(i);
    }
  }
  for (int i = 0; i < _n; ++i) {
    _input_degree[i]  = static_cast<int>(_in[i].size());
    _output_degree[i] = static_cast<int>(_out[i].size());
  }
}

std::vector<int> MFVSAlgorithm::heuristic()
{
  std::vector<int> solution;
  auto max_limit = std::numeric_limits<int>::max();

  for (;;) {
    compress(max_limit, solution);
    int choosen = chooseCandidate();
    if (choosen < 0) { // No remaining vertices: acyclic graph.
      break;
    }
    erase(choosen);
    solution.push_back(choosen);
  }
  undoUntil(0);

  // Discard vertices that don't add cycles with their reinsertion, to reduce
  // the number of returned nodes in the solution.
  for (int v : solution) {
    _alive[v] = 0;
  }
  std::vector<int> result;
  for (auto it = solution.rbegin(); it != solution.rend(); ++it) {
    _alive[*it] = 1;
    if (!acyclic()) {
      _alive[*it] = 0;
     result.push_back(*it);
    }
  }
  std::fill(_alive.begin(), _alive.end(), 1);

  return result;
}

int MFVSAlgorithm::chooseCandidate()
{
  int choosen = -1;
  long long max_points = -1;
  for (int v = 0; v < _n; ++v) {
    if (!_alive[v]) {
      continue;
    }
    const long long p = static_cast<long long>(_input_degree[v])
      *_output_degree[v];
    if (p > max_points) {
      max_points = p;
      choosen = v;
    }
  }

  return choosen;
}

std::vector<Vertex> MFVSAlgorithm::toGlobals(const std::vector<int>& locales)
  const
{
  std::vector<Vertex> r;
  r.reserve(locales.size());
  for (int v : locales) {
    r.push_back(_globals[v]);
  }
  std::sort(r.begin(), r.end());
  return r;
}

void MFVSAlgorithm::erase(int v)
{
  _alive[v] = 0;
  for (int w : _out[v]) {
    if (_alive[w]) {
      --_input_degree[w];
    }
  }
  for (int u : _in[v]) {
    if (_alive[u]) {
      --_output_degree[u];
    }
  }
  _deleted.push_back(v);
}

void MFVSAlgorithm::restore(int v)
{
  _alive[v] = 1;
  for (int w : _out[v]) {
    if (_alive[w]) {
      ++_input_degree[w];
     }
  }
  for (int u : _in[v]) {
    if (_alive[u]) {
      ++_output_degree[u];
    }
  }
}

void MFVSAlgorithm::undoUntil(std::size_t position)
{
  while (_deleted.size() > position) {
    const int v = _deleted.back();
    _deleted.pop_back();
    restore(v);
  }
}

bool MFVSAlgorithm::compress(int& k, std::vector<int>& result)
{
  std::vector<int> pending;
  for (int v = 0; v < _n; ++v) {
    if (_alive[v]) {
      pending.push_back(v);
    }
  }

  while (!pending.empty()) {
    const int v = pending.back();
    pending.pop_back();
    if (!_alive[v]) {
      continue;
    }

    if (_loop[v]) { // Self loops must go in the result.
      if (k == 0) {
        return false;
      }
      --k;
      result.push_back(v);
    } else if (_input_degree[v] > 0 && _output_degree[v] > 0) {
      // Irreducible vertex.
      continue;
    }
    erase(v);
    for (int w : _out[v]) {
      if (_alive[w]) {
        pending.push_back(w);
      }
    }
    for (int u : _in[v]) {
      if (_alive[u]) {
        pending.push_back(u);
      }
    }
  }

  return true;
}

bool MFVSAlgorithm::acyclic() const
{
  std::vector<int> grado(_n, 0);
  std::vector<int> sequence;
  int alive_quantity = 0;
  for (int v = 0; v < _n; ++v) {
    if (!_alive[v]) {
      continue;
    }
    ++alive_quantity;
    if (_loop[v]) {
      return false;
    }
    for (int w : _out[v]) {
      if (_alive[w]) {
        ++grado[w];
      }
    }
  }

  for (int v = 0; v < _n; ++v) {
    if (_alive[v] && grado[v] == 0) {
      sequence.push_back(v);
    }
  }

  for (std::size_t h = 0; h < sequence.size(); ++h) {
    for (int w : _out[sequence[h]]) {
      if (_alive[w] && --grado[w] == 0) {
        sequence.push_back(w);
      }
    }
  }

  return static_cast<int>(sequence.size()) == alive_quantity;
}

} // namespace detail

std::vector<Vertex> heuristicFVS(const Graph& g) {
  std::vector<Vertex> V(boost::num_vertices(g));
  std::iota(V.begin(), V.end(), Vertex{0});
  detail::MFVSAlgorithm algorithm(g, std::move(V));
  return algorithm.toGlobals(algorithm.heuristic());
}

} // namespace scalar

} // namespace test

} // namespace Modelica
