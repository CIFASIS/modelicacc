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
#include <sbg/pw_map.hpp>

#include <utility>

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// ModelicaCC causalized model -------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

// CausalEquations -------------------------------------------------------------

CausalEquations::CausalEquations(EquationList equations, ExpList variables) : _equations(equations), _variables(variables) {}

const EquationList& CausalEquations::equations() const { return _equations; }

const ExpList& CausalEquations::variables() const { return _variables; }

bool CausalEquations::isEmpty() const { return _equations.empty(); }

void CausalEquations::pushBack(Equation equation, Expression variable)
{
  _equations.push_back(equation);
  _variables.push_back(variable);
}

std::ostream& operator<<(std::ostream& out, const CausalEquations& causal_eqs)
{
  for (const Expression& var : causal_eqs.variables()) {
    out << var << "\n";
  }
  for (const Equation& eq : causal_eqs.equations()) {
    out << eq << "\n";
  }

  return out;
}

// CausalModel -----------------------------------------------------------------

std::size_t CausalModel::size() const { return _causal_eqs.size(); }

CausalEquations CausalModel::operator[](std::size_t k) const
{
  ERROR_UNLESS(k < _causal_eqs.size(), "CausalModel::operator[]: index ", k, " out of range");
  return _causal_eqs[k];
}

void CausalModel::pushBack(CausalEquations eqs)
{
  if (!eqs.isEmpty()) {
    _causal_eqs.push_back(eqs);
  }
}

std::ostream& operator<<(std::ostream& out, const CausalModel& causal_model)
{
  for (const CausalEquations& eqs : causal_model) {
    out << eqs << "\n";
  }

  return out;
}

////////////////////////////////////////////////////////////////////////////////
// Auxiliary functions for VerticalSortingResult -------------------------------
////////////////////////////////////////////////////////////////////////////////

namespace {

// getExpr ---------------------------------------------------------------------

/**
 * @brief Give a sorting \p sort of a acyclic directed SBG, and a sub-map \p m,
 * it calculates the expression of the composed \p sort that revisits the
 * elements of the domain of \p m for the first time.
 */
SBG::LIB::Expression getExpr(const SBG::LIB::Set& m_domain, const SBG::LIB::PWMap& sort)
{
  SBG::LIB::PWMap m_sort = sort.restrict(m_domain);
  while (m_sort.image().intersection(m_domain).isEmpty()) {
    m_sort = sort.composition(m_sort);
  }
  SBG::LIB::Set aux = m_domain.intersection(m_sort.preImage(m_domain));
  aux = aux.difference(m_sort.fixedPoints());
  m_sort = m_sort.restrict(aux);
  unsigned int j = 0;
  for (const auto& _ : m_sort) {
    ++j;
  }
  ERROR_UNLESS(1 >= j, "getExpr: case not yet supported");

  SBG::LIB::Expression result = (*(m_sort.begin())).law();
  result = result.composition(SBG::LIB::Expression{m_domain.arity(), -1, 0});
  result = SBG::LIB::Expression{m_domain.arity(), 0, 0} - result;
  return result;
}

// flatten ---------------------------------------------------------------------

SBG::LIB::Set flatten(const std::vector<SBG::LIB::Set>& s_vector)
{
  SBG::LIB::Set flat_set;
  for (const SBG::LIB::Set& s : s_vector) {
    flat_set = std::move(flat_set).disjointCup(s);
  }
  return flat_set;
}

// sortSets --------------------------------------------------------------------

std::vector<SBG::LIB::Set> sortSets(const SBG::LIB::PWMap& sort, std::vector<SBG::LIB::Set> sets, SBG::LIB::Set start)
{
  std::vector<SBG::LIB::Set> result;

  result.push_back(start);

  SBG::LIB::PWMap jth_sort = sort.restrict(start);
  SBG::LIB::Set sorted = start;
  SBG::LIB::PWMap tail_sort = sort.restrict(sort.domain().difference(sorted));
  SBG::LIB::Set sort_domain = sort.domain();
  while (sorted != sort_domain) {
    for (const SBG::LIB::Set& new_elems : sets) {
      SBG::LIB::Set not_sorted = new_elems.difference(sorted);
      SBG::LIB::Set ingoing = tail_sort.image(not_sorted).intersection(sorted);
      if (!ingoing.isEmpty()) {
        SBG::LIB::PWMap new_sort = sort.restrict(not_sorted);
        SBG::LIB::PWMap jth_sort_copy = jth_sort.concatenation(new_sort);
        SBG::LIB::Set reps = jth_sort_copy.mapInf().image();
        if (!reps.isEmpty()) {
          result.push_back(not_sorted);
          sorted = std::move(sorted).disjointCup(not_sorted);
          jth_sort = std::move(jth_sort).concatenation(std::move(new_sort));
          break;
        }
      }
    }
  }

  return result;
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////
// Vertical sorting return structure -------------------------------------------
////////////////////////////////////////////////////////////////////////////////

VerticalSortingResult::VerticalSortingResult(ModelicaSBG modelica_bsbg, SBG::LIB::PWMap sort) : _modelica_bsbg(modelica_bsbg), _sort(sort)
{
}

const ModelicaSBG& VerticalSortingResult::modelica_bsbg() const { return _modelica_bsbg; }

const SBG::LIB::PWMap& VerticalSortingResult::sort() const { return _sort; }

std::vector<SBG::LIB::Set> VerticalSortingResult::toIntermediateFormat() const
{
  std::vector<SBG::LIB::Set> modelica_sort;
  for (const SBG::LIB::Map& m : const_cast<SBG::LIB::PWMap&>(_sort)) {
    SBG::LIB::Set domain = m.domain();
    for (const SetEdge& se : _modelica_bsbg.set_edges()) {
      SBG::LIB::Set se_in_domain = se.domain().set().intersection(domain);
      if (!se_in_domain.isEmpty()) {
        modelica_sort.push_back(se_in_domain);
      }
    }
  }

  return sortSets(_sort, modelica_sort, _sort.fixedPoints());
}

std::vector<LoopT> VerticalSortingResult::groupLoops(std::vector<SBG::LIB::Set> modelica_sort, std::vector<LoopT> loops) const
{
  std::vector<LoopT> loops_copy = loops;
  std::vector<LoopT> sorted_loops;
  for (unsigned int j = 0; j < modelica_sort.size(); ++j) {
    SBG::LIB::Set flattened_loop;
    for (const LoopT& loop : loops_copy) {
      flattened_loop = flatten(loop);
      if (!flattened_loop.intersection(modelica_sort[j]).isEmpty()) {
        break;
      }
    }

    LoopT sorted_loop;
    for (unsigned int h = j; h < modelica_sort.size(); ++h) {
      if (flattened_loop.intersection(modelica_sort[h]).isEmpty()) {
        j = h - 1;
        sorted_loops.push_back(sorted_loop);
        break;
      } else if (h + 1 == modelica_sort.size()) {
        j = h;
        sorted_loop.push_back(modelica_sort[h]);
        sorted_loops.push_back(sorted_loop);
        break;
      } else {
        sorted_loop.push_back(modelica_sort[h]);
      }
    }
  }

  return sorted_loops;
}

CausalEquations VerticalSortingResult::causalizeLoop(LoopT loop) const
{
  CausalEquations result;

  // Convert to ModelicaCC representation
  for (const SBG::LIB::Set& s : loop) {
    SBG::LIB::Expression expr{s.arity(), 1, 0};
    if (s.cardinal() > 1) {
      expr = getExpr(s, _sort);
    }

    for (const SetEdge& se : _modelica_bsbg.set_edges()) {
      CompactSet se_and_m_domain = se.domain();
      se_and_m_domain.intersection(CompactSet{s});
      if (se_and_m_domain.cardinal() > 0) {
        EquationInfo eq_info = _modelica_bsbg.setVertex(se.eq_id()).info().value();
        std::vector<Name> counters;
        for (const Index& index : eq_info.indices()) {
          counters.push_back(index.name());
        }
        std::vector<Indexes> indices = toModelicaIndices(se_and_m_domain, se.translation(), counters, expr);
        for (const Indexes& indexes : indices) {
          result.pushBack(eq_info.restrictEquation(indexes), se.access());
        }
      }
    }
  }

  return result;
}

CausalModel VerticalSortingResult::toModelicaFormat(std::vector<LoopT> loops) const
{
  CausalModel result;

  // Convert _sort to equivalent SBG auxiliary representation
  std::vector<SBG::LIB::Set> modelica_sort = toIntermediateFormat();

  // Group algebraic loops
  std::vector<LoopT> sorted_loops = groupLoops(modelica_sort, loops);

  // Convert to final ModelicaCC format
  for (const LoopT& loop : sorted_loops) {
    result.pushBack(causalizeLoop(loop));
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
// Vertical sorting ------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

VerticalSorting::VerticalSorting(AlgebraicLoopsResult& loops_result, TearingResult& tearing_result)
    : _loops_result(loops_result), _tearing_result(tearing_result)
{
}

VerticalSortingResult VerticalSorting::sort()
{
  SBG::LIB::DirectedSBG vertical_dsbg = misc::buildVerticalSortingSBG(_loops_result.scc_result(), _tearing_result.mfvs_result());
  SBG::LIB::PWMap vertical_sort = SBG::LIB::TopologicalSorting{}.calculate(vertical_dsbg, _loops_result.scc_result().rmap());

  return VerticalSortingResult{_tearing_result.modelica_bsbg(), vertical_sort};
}

}  // namespace Causalize

}  // namespace Modelica
