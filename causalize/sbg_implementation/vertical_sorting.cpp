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

#include "causalize/sbg_implementation/vertical_sorting.hpp"
#include "causalize/sbg_implementation/equation_info.hpp"
#include "causalize/sbg_implementation/set_edge.hpp"
#include "util/compact_set.hpp"
#include "util/debug.hpp"

#include <algorithms/misc/causalization_builders.hpp>
#include <algorithms/sorting/topological/topological_sorting.hpp>
#include <boost/variant/get.hpp>
#include <sbg/directed_sbg.hpp>
#include <sbg/expression.hpp>
#include <sbg/map.hpp>
#include <sbg/pw_map.hpp>
#include <sbg/set.hpp>

#include <algorithm>

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// Auxiliary structures --------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

// CausalEquations ---------------------------------------------------------------

CausalEquations::CausalEquations(EquationList equations, ExpList variables)
  : _equations(equations), _variables(variables) {}

const EquationList& CausalEquations::equations() const { return _equations; }

const ExpList& CausalEquations::variables() const { return _variables; }

bool CausalEquations::isEmpty() const { return _equations.empty(); }

void CausalEquations::pushBack(Equation equation, Expression variable)
{
  _equations.push_back(equation);
  _variables.push_back(variable);
}

std::ostream& operator<<(std::ostream& out, const CausalEquations& eqs)
{
  for (const Expression& var : eqs.variables()) {
    out << var << "\n";
  }
  for (const Equation& eq : eqs.equations()) {
    out << eq << "\n";
  }

  return out;
}

// CausalizationResult --------------------------------------------------------------

std::size_t CausalizationResult::size() const { return _causal_eqs.size(); }

CausalEquations CausalizationResult::operator[](std::size_t k) const
{
  ERROR_UNLESS(k < _causal_eqs.size(), "CausalizationResult::operator[]: index ", k
    , " out of range");
  return _causal_eqs[k];
}

auto CausalizationResult::begin() const { return _causal_eqs.begin(); }

auto CausalizationResult::end() const { return _causal_eqs.end(); }

void CausalizationResult::pushBack(CausalEquations eqs)
{
  if (!eqs.isEmpty()) {
    _causal_eqs.push_back(eqs);
  }
}

std::ostream& operator<<(std::ostream& out
  , const CausalizationResult& causal_eqs)
{
  for (const CausalEquations& eqs : causal_eqs) {
    out << eqs << "\n";
  }

  return out;
}

////////////////////////////////////////////////////////////////////////////////
// Auxiliary functions ---------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

// getSCCDSBG ------------------------------------------------------------------

/**
 * @brief Constructs a directed SBG where the vertices of \p dsbg are grouped
 * according to \p scc_data.rmap(), and only edges in \p scc_data.Ediff()
 * are preserved. This directed SBG is constructed to get the order between
 * algebraic loops.
 */
SBG::LIB::DirectedSBG getSCCDSBG(SBG::LIB::DirectedSBG dsbg
  , SBG::LIB::SCCData scc_data)
{
  SBG::LIB::PWMap rmap = scc_data.rmap();
  SBG::LIB::Set V = rmap.image();
  SBG::LIB::PWMap Vmap = dsbg.Vmap().restrict(V); 

  SBG::LIB::Set E = scc_data.Ediff();
  SBG::LIB::PWMap mapB = rmap.composition(dsbg.mapB().restrict(E));
  SBG::LIB::PWMap mapD = rmap.composition(dsbg.mapD().restrict(E));
  SBG::LIB::PWMap Emap = dsbg.Emap().restrict(E);

  return SBG::LIB::DirectedSBG{V, Vmap, mapB, mapD, Emap};
}

// getExpr ---------------------------------------------------------------------

/**
 * @brief Give a sorting \p sort of a acyclic directed SBG, and a sub-map \p m,
 * it calculates the expression of the composed \p sort that revisits the
 * elements of the domain of \p m for the first time.
 */
SBG::LIB::Expression getExpr(const SBG::LIB::Set& m_domain
  , const SBG::LIB::PWMap& sort)
{
  SBG::LIB::PWMap m_sort = sort.restrict(m_domain);
  while (m_sort.image().intersection(m_domain).isEmpty()) {
    m_sort = sort.composition(m_sort);
  }
  m_sort = m_sort.restrict(m_domain);
  m_sort = m_sort.restrict(m_sort.preImage(m_domain));
  SBG::LIB::Set self_reps = m_sort.fixedPoints();
  m_sort = m_sort.restrict(m_sort.domain().difference(self_reps));
  unsigned int j = 0;
  for (const auto& _ : m_sort) {
    ++j;
  }
  ERROR_UNLESS(1 >= j, "getExpr: case not yet supported");
  return (*(m_sort.begin())).law();
}

// detectLoops -----------------------------------------------------------------

std::vector<std::vector<SBG::LIB::Set>> detectLoops(const AlgebraicLoopsInfo&
  loops_info)
{
  std::vector<std::vector<SBG::LIB::Set>> result;

  SBG::LIB::PWMap rmap = loops_info.scc_result().rmap();
  SBG::LIB::Set representatives = rmap.image();
  while (!representatives.isEmpty()) {
    SBG::LIB::Set min_elem_set{representatives.minElem()};
    SBG::LIB::Set represented = rmap.preImage(min_elem_set);
    // Get all equations and variables that belong to this loop
    std::vector<SBG::LIB::Set> loop;
    for (const SetEdge& se : loops_info.set_edges()) {
      SBG::LIB::Set se_domain = se.domain().set();
      SBG::LIB::Set loop_in_se = se_domain.intersection(represented);
      if (loop_in_se.cardinal() > 0) {
        loop.push_back(se.domain().set());
        representatives = representatives.difference(se_domain);
      }
    }
    result.push_back(loop);
  }

  return result;
}

// sortLoops -------------------------------------------------------------------

SBG::LIB::Set flatten(const std::vector<SBG::LIB::Set>& s_vector)
{
  SBG::LIB::Set flat_set;
  for (const SBG::LIB::Set& s : s_vector) {
    flat_set = std::move(flat_set).disjointCup(s);
  }
  return flat_set;
}

std::vector<std::vector<SBG::LIB::Set>> sortLoops(
  const SBG::LIB::PWMap& sort
  , const std::vector<std::vector<SBG::LIB::Set>>& loops)
{
  std::vector<std::vector<SBG::LIB::Set>> result;

  SBG::LIB::Set jth_flat_loop;
  SBG::LIB::Set start = sort.fixedPoints();
  for (const std::vector<SBG::LIB::Set>& loop : loops) {
    SBG::LIB::Set flat_loop = flatten(loop);
    if (!flat_loop.intersection(start).isEmpty()) {
      jth_flat_loop = flat_loop;
      result.push_back(loop);
      break;
    }
  }

  SBG::LIB::Set sorted = jth_flat_loop;
  SBG::LIB::PWMap aux = sort.restrict(sort.domain().difference(sorted));
  SBG::LIB::Set sort_domain = sort.domain();
  while (sorted != sort_domain) {
    for (const std::vector<SBG::LIB::Set>& loop : loops) {
      SBG::LIB::Set new_loop = flatten(loop);
      SBG::LIB::Set ingoing = aux.image(new_loop).intersection(jth_flat_loop);
      SBG::LIB::Set new_sorted = new_loop.intersection(sorted);
      if (!ingoing.isEmpty() && new_sorted.isEmpty()) {
        sorted = std::move(sorted).disjointCup(new_loop);
        jth_flat_loop = new_loop;
        result.push_back(loop);
        break;
      }
    }
  }

  std::reverse(result.begin(), result.end());
  return result;
}

////////////////////////////////////////////////////////////////////////////////
// Vertical Sorting ------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

VerticalSorting::VerticalSorting(AlgebraicLoopsInfo& loops_info
  , TearingResult& tearing_result)
  : _loops_info(loops_info), _tearing_result(tearing_result) {}

CausalizationResult VerticalSorting::toCausalEquations(
  const SBG::LIB::PWMap& sort, std::map<int, EquationInfo> equations_info) const
{
  CausalizationResult result;

  // Detect algebraic loops first
  std::vector<std::vector<SBG::LIB::Set>> loops = detectLoops(_loops_info);

  // Sort between different algebraic loops (different SCCs of the SBG)
  std::vector<std::vector<SBG::LIB::Set>> sorted_loops = sortLoops(sort, loops);

  // Order each ModelicaCC array equation (inside each SCC of the SBG)
  for (const std::vector<SBG::LIB::Set>& loop : sorted_loops) {
    CausalEquations causal_eqs;
    for (const SBG::LIB::Set& s : loop) {
      SBG::LIB::Expression expr{s.arity(), 1, 0};
      if (s.cardinal() > 1) {
        expr = getExpr(s, sort);
      }

      for (const SetEdge& se : _loops_info.set_edges()) {
        CompactSet se_and_m_domain = se.domain();
        se_and_m_domain.intersection(CompactSet{s});
        if (se_and_m_domain.cardinal() > 0) {
          EquationInfo eq_info = equations_info.at(se.eq_id());
          std::vector<Name> counters;
          for (const Index& index : eq_info.indices()) {
            counters.push_back(index.name());
          }
          std::vector<Indexes> indices = toModelicaIndices(se_and_m_domain
            , se.translation(), counters, expr);
          for (const Indexes& indexes : indices) {
            causal_eqs.pushBack(eq_info.restrictEquation(indexes), se.access());
          }
        }
      }
    }
    result.pushBack(causal_eqs);
  }
 
  return result;
}

CausalizationResult VerticalSorting::sort(
  std::map<int, EquationInfo> equations_info)
{
  SBG::LIB::DirectedSBG vertical_dsbg = misc::buildVerticalSortingSBG(
    _loops_info.scc_result(), _tearing_result.mfvs_result());
  SBG::LIB::PWMap vertical_sort = SBG::LIB::TopologicalSorting{}.calculate(
    vertical_dsbg, SBG::LIB::PWMap{});

  return toCausalEquations(vertical_sort, equations_info);
}

} // namespace Causalize

} // namespace Modelica
