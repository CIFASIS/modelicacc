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

#ifndef MODELICACC_TEST_SCALARORMANCE_SCALAR_SCALAR_UTILS_HPP_
#define MODELICACC_TEST_SCALARORMANCE_SCALAR_SCALAR_UTILS_HPP_

#include <boost/graph/adjacency_list.hpp>

namespace Modelica {

namespace test {

namespace scalar {

using Graph  = boost::adjacency_list<boost::vecS, boost::vecS
  , boost::directedS>;
using Vertex = boost::graph_traits<Graph>::vertex_descriptor;

void add_edge(int u, int v, Graph& g);

} // namespace scalar

} // namespace test

} // namespace Modelica

#endif // MODELICACC_TEST_SCALARORMANCE_SCALAR_SCALAR_UTILS_HPP_
