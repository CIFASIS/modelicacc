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

#ifndef TEST_PERFORMANCE_SCALAR_MFVS_HPP_
#define TEST_PERFORMANCE_SCALAR_MFVS_HPP_

#include "test/performance/scalar/mfvs.hpp"
#include "test/performance/scalar/scalar_utils.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/strong_components.hpp>
#include <boost/range/iterator_range.hpp>

namespace Modelica {

namespace test {

namespace scalar {

namespace detail {

class MFVSAlgorithm {
public:
  MFVSAlgorithm(const Graph& g, std::vector<Vertex> vertices);

  std::vector<int> heuristic(); 

  std::vector<Vertex> toGlobals(const std::vector<int>& locales) const;

private:
  int chooseCandidate();
  void erase(int v);
  void restore(int v);
  void undoUntil(std::size_t position);
  bool compress(int& k, std::vector<int>& result);
  bool acyclic() const;

  int _n; // Number of vertices.
  std::vector<Vertex> _globals; // índice local -> vértice de g
  std::vector<std::vector<int>> _out;
  std::vector<std::vector<int>> _in;
  std::vector<char> _loop;
  std::vector<char> _alive;
  std::vector<int> _input_degree; // Input degree for alive vertices.
  std::vector<int> _output_degree; // Output degree for alive vertices.
  std::vector<int> _deleted; // Deleted vertices.
};

} // namespace detail

std::vector<Vertex> heuristicFVS(const Graph& g);

} // namespace scalar

} // namespace test

} // namespace Modelica

#endif // TEST_PERFORMANCE_SCALAR_MFVS_HPP_
