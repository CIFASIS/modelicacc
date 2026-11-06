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

#include "test/performance/scalar/build_graphs.hpp"
#include "test/performance/scalar/scalar_utils.hpp"

namespace Modelica {

namespace test {

namespace scalar {

Graph buildTestRL1MFVS(int N)
{
  Graph g(3*N);

  add_edge(N+1, 1, g);               // 11 -> 1
  for (int i = 1; i < N - 1; ++i) {
    add_edge(N + i + 1, i + 1, g);   // 12 -> 2, ..., 19 -> 9
  }
  add_edge(3*N, N, g);               // 30 -> 10
  for (int i = 1; i < N; ++i) {
    add_edge(2*N - 1, i + 1, g);     // 20 -> 2, ..., 28 -> 10
    add_edge(2*N - 1 + i, N + i, g); // 20 -> 11, ..., 28 -> 19
    add_edge(N + i, 2*N - 1 + i, g); // 11 -> 20, ..., 19 -> 28
  }
  add_edge(3*N, 3*N - 1, g);         // 30 -> 29
  add_edge(3*N - 1, 3*N, g);         // 29 -> 30

  return g;
}

Graph buildTestRL3MFVS(int N)
{
  Graph g(3*N);

  for (int i = 1; i < N; ++i) {
    add_edge(2*N - 1 + i, i + 1, g);     // 20 -> 2, ..., 28 -> 10
  }
  for (int i = N + 1; i < 2*N - 1; ++i) {
    add_edge(i + 1, i, g);               // 12 -> 11, ..., 19 -> 18
  }
  for (int i = 1; i < N - 1; ++i) {
    add_edge(N + 1 + i, 2*N - 1 + i, g); // 12 -> 20, ..., 19 -> 27
  }
  for (int i = 1; i < N - 1; ++i) {
    add_edge(2*N - 1 + i, 2*N + i, g);   // 20 -> 21, ..., 27 -> 28
  }
  add_edge(3*N, 2*N - 1, g);             // 30 -> 19
  add_edge(3*N, 3*N - 2, g);             // 30 -> 28
  add_edge(3*N - 2, 3*N, g);             // 28 -> 30
  add_edge(N + 1, 3*N - 1, g);           // 11 -> 29
  add_edge(3*N - 1, 1, g);               // 29 -> 1
  add_edge(3*N - 1, 2*N, g);             // 29 -> 20

  return g;
}

Graph buildTestRL1Sort(int N)
{
  Graph g(4*N);

  add_edge(N+1, 1, g);                 // 11 -> 1
  for (int i = 1; i < N - 1; ++i) {
    add_edge(N + i + 1, i + 1, g);     // 12 -> 2, ..., 19 -> 9
  }
  add_edge(3*N - 1, N, g);             // 29 -> 10
  for (int i = 1; i < N; ++i) {
    add_edge(N + i, i + 1, g);         // 11 -> 2, ..., 19 -> 10
    add_edge(2*N - 1 + i, N + i, g);   // 20 -> 11, ..., 28 -> 19
    add_edge(3*N + i, 2*N - 1 + i, g); // 31 -> 20, ..., 39 -> 28
  }
  add_edge(3*N, 3*N - 1, g);           // 30 -> 29
  add_edge(4*N, 3*N, g);               // 40 -> 30

  return g;
}

Graph buildTestRL2Sort(int N)
{
  Graph g(3*N);

  for (int i = 1; i < N; ++i) {
    add_edge(2*N - 1 + i, i + 1, g);     // 20 -> 2, ..., 28 -> 10
  }
  for (int i = N + 1; i < 2*N - 1; ++i) {
    add_edge(i + 1, i, g);               // 12 -> 11, ..., 19 -> 18
  }
  for (int i = 1; i < N - 1; ++i) {
    add_edge(N + 1 + i, 2*N - 1 + i, g); // 12 -> 20, ..., 19 -> 27
  }
  for (int i = 1; i < N - 1; ++i) {
    add_edge(2*N - 1 + i, 2*N + i, g);   // 20 -> 21, ..., 27 -> 28
  }
  add_edge(3*N, 1, g);                   // 30 -> 1
  add_edge(3*N - 1, 2*N - 1, g);         // 29 -> 19
  add_edge(3*N - 1, 3*N - 2, g);         // 29 -> 28
  add_edge(3*N, 2*N, g);                 // 30 -> 20
  add_edge(N + 1, 3*N, g);               // 11 -> 30

  return g;
}

Graph buildTestRL3Sort(int N)
{
  Graph g(3*N+1);

  for (int i = 1; i < N; ++i) {
    add_edge(3*N, i, g);                 // 30 -> 1, ..., 30 -> 10
  }
  for (int i = N + 1; i < 2*N - 1; ++i) {
    add_edge(i + 1, i, g);               // 12 -> 11, ..., 19 -> 18
  }
  for (int i = 1; i < N - 1; ++i) {
    add_edge(N + 1 + i, 2*N - 1 + i, g); // 12 -> 20, ..., 19 -> 27
  }
  for (int i = 1; i < N - 1; ++i) {
    add_edge(2*N - 1 + i, 2*N + i, g);   // 20 -> 21, ..., 27 -> 28
  }
  add_edge(3*N + 1, 2*N - 1, g);         // 31 -> 19
  add_edge(3*N + 1, 3*N - 2, g);         // 31 -> 28
  add_edge(3*N - 2, 3*N, g);             // 28 -> 30
  add_edge(N + 1, 3*N - 1, g);           // 11 -> 29
  add_edge(3*N - 1, 2*N, g);             // 29 -> 20

  return g;
}

} // namespace scalar

} // namespace test

} // namespace Modelica
