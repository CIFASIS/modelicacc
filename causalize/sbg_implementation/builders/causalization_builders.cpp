/*******************************************************************************

 This file is part of Set--Based Graph Library.

 SBG Library is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 SBG Library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with SBG Library.  If not, see <http://www.gnu.org/licenses/>.

 ******************************************************************************/

#include "causalize/sbg_implementation/builders/causalization_builders.hpp"
#include "ast/equation.hpp"
#include "ast/expression.hpp"
#include "causalize/sbg_implementation/set_edge.hpp"
#include "causalize/sbg_implementation/set_vertex.hpp"
#include "causalize/sbg_implementation/ast_visitors/residual_equation.hpp"
#include "causalize/sbg_implementation/ast_visitors/variable_renamer.hpp"
#include "util/compact_set.hpp"
#include "util/translation.hpp"

#include <boost/variant/get.hpp>
#include <sbg/bipartite_sbg.hpp>
#include <sbg/map.hpp>
#include <sbg/pw_map.hpp>
#include <sbg/set.hpp>
#include <util/time_profiler.hpp>

#include <string>
#include <variant>

namespace Modelica {

namespace Causalize {

////////////////////////////////////////////////////////////////////////////////
// Algebraic loops detection graph builder -------------------------------------
////////////////////////////////////////////////////////////////////////////////

std::tuple<SBG::LIB::Set, SBG::LIB::PWMap> buildSCCVertices(
  const SBG::LIB::MatchData& data)
{
  SBG::LIB::Set M = data.M();

  M.compact();
  SBG::LIB::Set V = M;
  SBG::LIB::PWMap auxVmap = data.bsbg().Emap().restrict(M);
  SBG::LIB::PWMap Vmap;
  for (const SBG::LIB::Map& m : auxVmap) { 
    SBG::LIB::Set domain = m.domain();
    domain.compact();
    Vmap.emplace(domain, m.law());
  }

  return {V, Vmap};
}

std::tuple<SBG::LIB::PWMap, SBG::LIB::PWMap, SBG::LIB::PWMap> buildSCCEdges(
  const SBG::LIB::MatchData& data, const SBG::LIB::PWMap& Vmap)
{
  const SBG::LIB::BipartiteSBG& bsbg = data.bsbg();
  SBG::LIB::Set M = data.M();

  SBG::LIB::PWMap map1 = bsbg.map1();
  SBG::LIB::PWMap map2 = bsbg.map2();

  SBG::LIB::Set X = bsbg.X();
  SBG::LIB::PWMap map1_toX = map1.restrict(map1.preImage(X));
  SBG::LIB::PWMap map2_toX = map2.restrict(map2.preImage(X));
  SBG::LIB::PWMap mapF = map1_toX.concatenation(map2_toX);

  SBG::LIB::Set Y = bsbg.Y();
  SBG::LIB::PWMap map1_toY = map1.restrict(map1.preImage(Y));
  SBG::LIB::PWMap map2_toY = map2.restrict(map2.preImage(Y));
  SBG::LIB::PWMap mapU = map1_toY.concatenation(map2_toY);

  SBG::LIB::Set free_edges = bsbg.E().difference(M);
  SBG::LIB::PWMap matchedU_inv = mapU.restrict(M).inverse();
  SBG::LIB::PWMap unmatchedU = mapU.restrict(free_edges);
  SBG::LIB::PWMap mapB = matchedU_inv.composition(unmatchedU);
  mapB.compact();

  SBG::LIB::PWMap matchedF_inv = mapF.restrict(M).inverse();
  SBG::LIB::PWMap unmatchedF = mapF.restrict(free_edges);
  SBG::LIB::PWMap mapD = matchedF_inv.composition(unmatchedF);
  mapD.compact();

  SBG::LIB::PWMap Emap = bsbg.Emap().restrict(free_edges);
  Emap.compact();

  return {mapB, mapD, Emap};
}

void partitionEmap(SBG::LIB::DirectedSBG& dsbg)
{
  SBG::LIB::Set V = dsbg.V();
  SBG::LIB::PWMap Vmap = dsbg.Vmap();
  SBG::LIB::PWMap mapB = dsbg.mapB();
  SBG::LIB::PWMap mapD = dsbg.mapD();
  SBG::LIB::PWMap Emap = dsbg.Emap();

  std::size_t arity = V.arity();
  SBG::LIB::NAT j = 1;
  SBG::LIB::PWMap partitioned_Emap;
  dsbg.foreachSetEdge([&](const SBG::LIB::MD_NAT& SE)
  {
    SBG::LIB::Set E = Emap.preImage(SBG::LIB::Set{SE});
    SBG::LIB::Set SV_starts = Vmap.image(mapB.image(E));
    SBG::LIB::Set SV_ends = Vmap.image(mapD.image(E));
    if (SV_starts.cardinal()*SV_ends.cardinal() > 1) {
      SBG::LIB::Set remaining1 = SV_starts;
      while (!remaining1.isEmpty()) {
        SBG::LIB::Set SV1 = SBG::LIB::Set{remaining1.minElem()};
        SBG::LIB::Set V1 = Vmap.preImage(SV1); 

        SBG::LIB::Set remaining2 = SV_ends;
        while (!remaining2.isEmpty()) {
          SBG::LIB::Set SV2 = SBG::LIB::Set{remaining2.minElem()};
          SBG::LIB::Set V2 = Vmap.preImage(SV2); 

          SBG::LIB::Set edges_V1_V2 = mapB.preImage(V1)
            .intersection(mapD.preImage(V2));
          partitioned_Emap.emplace(E.intersection(edges_V1_V2)
            , SBG::LIB::Expression{SBG::LIB::MD_NAT{arity, j}});
          ++j;

          remaining2 = remaining2.difference(SV2);
        }
        remaining1 = remaining1.difference(SV1);
      }
    } else {
      partitioned_Emap.emplace(E
        , SBG::LIB::Expression{SBG::LIB::MD_NAT{arity, j}});
      ++j;
    }
  });

  dsbg = SBG::LIB::DirectedSBG{V, Vmap, mapB, mapD, partitioned_Emap};
}

SBG::LIB::DirectedSBG buildLoopDetectionSBG(const SBG::LIB::MatchData& data)
{
  SBG::Util::Internal::TimeProfiler profiler{"SBG Loop Detection builder"};

  SBG::LIB::Set V;
  SBG::LIB::PWMap Vmap;
  std::tie(V, Vmap) = buildSCCVertices(data);

  SBG::LIB::PWMap mapB;
  SBG::LIB::PWMap mapD;
  SBG::LIB::PWMap Emap;
  std::tie(mapB, mapD, Emap) = buildSCCEdges(data, Vmap);

  SBG::LIB::DirectedSBG dsbg{V, Vmap, mapB, mapD, Emap};
  partitionEmap(dsbg);

  return dsbg;
}

////////////////////////////////////////////////////////////////////////////////
// Algebraic loops breaker graph builder ---------------------------------------
////////////////////////////////////////////////////////////////////////////////

SBG::LIB::DirectedSBG buildTearingSBG(const SBG::LIB::SCCData& data)
{
  SBG::Util::Internal::TimeProfiler profiler{"SBG Tearing builder"};

  // Erase edges connecting different SCC
  SBG::LIB::DirectedSBG dsbg = data.dsbg();
  dsbg.eraseEdges(data.Ediff());

  // Erase vertices that belong to a singleton SCC
  SBG::LIB::PWMap mmap = data.rmap().imageMultiplicity();
  SBG::LIB::Set one{SBG::LIB::MD_NAT{dsbg.V().arity(), 1}};
  dsbg.eraseVertices(mmap.preImage(one));

  return dsbg;
}

////////////////////////////////////////////////////////////////////////////////
// Vertical sort graph builder ------------------------------------------------- 
////////////////////////////////////////////////////////////////////////////////

// Constructors/Destructors ----------------------------------------------------

VerticalSortingBuilder::VerticalSortingBuilder(const SBG::LIB::SCCData& data
  , const SBG::LIB::Set& mfvs)
  : _input_dsbg(data.dsbg()), _input_rmap(data.rmap()), _Ediff(data.Ediff())
    , _residual_vertices(mfvs) {}

// Getters ---------------------------------------------------------------------

const SBG::LIB::DirectedSBG& VerticalSortingBuilder::dsbg() const
{
  return _output_dsbg;
}

const SBG::LIB::PWMap& VerticalSortingBuilder::rmap() const
{
  return _output_rmap;
}

const SBG::LIB::Set& VerticalSortingBuilder::residual_vertices() const
{
  return _residual_vertices;
}

const SBG::LIB::PWMap& VerticalSortingBuilder::guess_offset() const
{
  return _guess_offset;
}

const SBG::LIB::Set& VerticalSortingBuilder::end_points() const
{
  return _end_points;
}

const std::vector<std::pair<AST::Name, VarInfo>>&
  VerticalSortingBuilder::added_variables() const
{
  return _added_variables;
}

// Member functions ------------------------------------------------------------

// SBG functions ---------------------------------------------------------------

void VerticalSortingBuilder::addGuessVertices()
{
  SBG::LIB::Set V = _input_dsbg.V();
  SBG::LIB::PWMap Vmap = _input_dsbg.Vmap();

  SBG::LIB::PWMap mapB = _input_dsbg.mapB();
  SBG::LIB::PWMap mapD = _input_dsbg.mapD();
  SBG::LIB::PWMap Emap = _input_dsbg.Emap();

  // Add guess vertices.
  SBG::LIB::MD_NAT maxv = V.maxElem();
  for (const SBG::LIB::Map& sv : Vmap) { 
    _output_dsbg.addSetVertex(
      _residual_vertices.intersection(sv.domain()).offset(maxv)
    );
  }

  // Delete outgoing edges to the same SCC from residual vertices.
  SBG::LIB::Set outgoing = mapB.preImage(_residual_vertices).difference(_Ediff);
  _output_dsbg.eraseEdges(outgoing);

  // Save _guess_offset.
  _guess_offset = SBG::LIB::PWMap{_residual_vertices}
    + SBG::LIB::PWMap{SBG::LIB::Map{_residual_vertices
      , SBG::LIB::Expression{maxv}}};
  _guess_offset = _guess_offset.combine(SBG::LIB::PWMap{_output_dsbg.V()});

  // Add outgoing deleted edges from residual vertices to the same SCC, to guess
  // vertices.
  if (!outgoing.isEmpty()) {
    for (const SBG::LIB::Map& se : Emap) {
      SBG::LIB::Set domain = outgoing.intersection(se.domain());
      SBG::LIB::PWMap outgoingB = _guess_offset
        .composition(mapB.restrict(domain));
      SBG::LIB::PWMap outgoingD = mapD.restrict(domain);
      _output_dsbg.addSetEdge(outgoingB, outgoingD);
    }
  }

  // Add guess vertices to rmap.
  SBG::LIB::PWMap residual_rmap = _input_rmap.restrict(_residual_vertices);
  SBG::LIB::PWMap guess_rmap = residual_rmap
    .composition(_guess_offset.inverse());
  _output_rmap = _input_rmap.concatenation(std::move(guess_rmap));
}

void VerticalSortingBuilder::addDependencies(
  const SBG::LIB::PWMap& residual_to_endpoint)
{
  SBG::LIB::PWMap residual_id{_residual_vertices};
  SBG::LIB::MD_NAT maxe = _input_dsbg.E().maxElem();
  SBG::LIB::Expression maxe_expr{maxe};
  SBG::LIB::PWMap edges_offset = residual_id
    + SBG::LIB::PWMap{SBG::LIB::Map{_residual_vertices, maxe_expr}};

  // Create new edges from residual vertices to endpoints.
  SBG::LIB::PWMap edges_to_residual = edges_offset.inverse();
  SBG::LIB::PWMap edges_to_endpoint = residual_to_endpoint
    .composition(edges_to_residual); 
  SBG::LIB::Set loops = edges_to_residual.equalImage(edges_to_endpoint);
  SBG::LIB::Set not_loops = edges_offset.domain().difference(loops);
  edges_to_residual = edges_to_residual.restrict(not_loops);
  edges_to_endpoint = edges_to_endpoint.restrict(not_loops);

  // Add new edges from residual vertices to endpoints.
  SBG::LIB::PWMap Vmap = _output_dsbg.Vmap();
  for (const SBG::LIB::Map& sv : Vmap) {
    SBG::LIB::Set domain = edges_to_residual.preImage(sv.domain());
    _output_dsbg.addSetEdge(edges_to_residual.restrict(domain)
      , edges_to_endpoint.restrict(domain));
  }

  // Create new edges from start points to guess vertices.
  SBG::LIB::PWMap edges_to_guess = edges_to_residual + _guess_offset;
  SBG::LIB::PWMap edges_to_start_point;
  edges_to_guess = edges_to_guess.restrict(not_loops);
  edges_to_start_point = edges_to_start_point.restrict(not_loops);

  // Add new edges from start points to guess vertices.
  for (const SBG::LIB::Map& sv : Vmap) {
    SBG::LIB::Set domain = edges_to_guess.preImage(sv.domain());
    _output_dsbg.addSetEdge(edges_to_start_point.restrict(domain)
      , edges_to_guess.restrict(domain));
  }
}

void VerticalSortingBuilder::redirectEdiff(
  const SBG::LIB::PWMap& reps_to_endpoint)
{
  SBG::LIB::PWMap mapB = _input_dsbg.mapB();
  SBG::LIB::PWMap mapD = _input_dsbg.mapD();

  // Delete edges between different SCC.
  _output_dsbg.eraseEdges(_Ediff);

  // Create new edges.
  SBG::LIB::PWMap EdiffB = reps_to_endpoint
    .composition(_input_rmap.composition(mapB.restrict(_Ediff)));
  SBG::LIB::PWMap EdiffD = _guess_offset.composition(reps_to_endpoint
      .composition(_input_rmap.composition(mapD.restrict(_Ediff))));

  // Add new edges to _output_dsbg.
  SBG::LIB::PWMap Emap = _input_dsbg.Emap();
  for (const SBG::LIB::Map& se : Emap) {
    SBG::LIB::Set se_domain = se.domain();
    _output_dsbg.addSetEdge(EdiffB.restrict(se_domain)
      , EdiffD.restrict(se_domain));
  }
}

// Modelica SBG functions ------------------------------------------------------

void VerticalSortingBuilder::addVariables(ModelicaSBG modelica_bsbg)
{
  for (SetEdge& se : modelica_bsbg.set_edges()) {
    SBG::LIB::Set se_res = se.translatedDomain().set()
      .intersection(_residual_vertices);
    if (!se_res.isEmpty()) {
      AST::Name start_mod = "start";
      AST::ClassModification class_mod;
      class_mod.push_back(AST::ElMod{start_mod, AST::ModEq{AST::Expression{1}}});
      SetVertex var_sv = modelica_bsbg.setVertex(se.var_id());
      VarInfo var_info = std::get<VarInfo>(var_sv.info());
      var_info.set_modification(AST::Modification{AST::ModClass{class_mod}});
      _added_variables.push_back(
        {"guess_" + var_sv.name(), var_info}
      );
      _added_variables.push_back(
        {"res_" + var_sv.name(), var_info}
      );
    }
  }
}

ModelicaSBG VerticalSortingBuilder::partition(ModelicaSBG modelica_bsbg)
{
  ModelicaSBG result;

  // Partition equation/variable matchings according to residual vertices, i.e,
  // tearing variables.
  SetVertices svs = modelica_bsbg.set_vertices();
  for (const SetVertex& sv : svs) {
    result.addSetVertex(sv);
  }

  SBG::LIB::Set not_residual = _output_dsbg.V().difference(_residual_vertices);
  SetEdges ses = modelica_bsbg.set_edges();
  for (const SetEdge& se : ses) {
    result.addSetEdge(se.restrict(CompactSet{_residual_vertices}));
    result.addSetEdge(se.restrict(CompactSet{not_residual}));
  }

  return result;
}

void VerticalSortingBuilder::addGuessMatchs(ModelicaSBG& modelica_bsbg)
{
  SBG::LIB::MD_NAT max_elem = _input_dsbg.V().maxElem();
  SetEdges ses = modelica_bsbg.set_edges(); 
  for (const SetEdge& se : ses) {
    SBG::LIB::Set se_res = se.translatedDomain().set()
      .intersection(_residual_vertices);
    std::size_t arity = se_res.arity();
    if (!se_res.isEmpty()) {
      // Add guess equation vertices.
      SetVertex original_eq_sv = modelica_bsbg.setVertex(se.eq_id());
      std::string name = "guess(" + original_eq_sv.name() + ")";
      VariableRenamer renamer{"guess_"};
      AST::Equation guess_eq{AST::Equality{
        se.access(), Apply(renamer, se.access())
      }};
      EquationInfo original_info = std::get<EquationInfo>(
        original_eq_sv.info()
      );
      EquationInfo guess_info{
        original_info.indices(), guess_eq, original_info.scalar()
      };
      CompactSet guess_vertices{se_res};
      guess_vertices.translate(-se.translation());
      int guess_eq_id = modelica_bsbg.addSetVertex(
        guess_vertices, name, guess_info
      );

      // Add guess/equation matching edges.
      CompactSet edges = se.domain();
      Translation se_translation = se.translation();
      Translation t{arity};
      for (std::size_t k = 0; k < arity; ++k) {
        t[k] = max_elem[k] + se_translation[k];
      }
      name = "guess equation of " + se.name();
      modelica_bsbg.addSetEdge(
        se.var_id(), guess_eq_id, edges, t, se.access(), name
      );
    }
  }
}

void VerticalSortingBuilder::modifyResidualMatchs(ModelicaSBG& modelica_bsbg)
{
  for (SetEdge& se : modelica_bsbg.set_edges()) {
    SBG::LIB::Set se_res = se.translatedDomain().set()
      .intersection(_residual_vertices);
    if (!se_res.isEmpty()) {
      // Modify equation expression.
      SetVertex& eq_sv = modelica_bsbg.setVertex(se.eq_id());
      EquationInfo eq_info = std::get<EquationInfo>(eq_sv.info());
      ResidualEqVisitor res_visit{se.access()};
      eq_sv.set_info(EquationInfo{
        eq_info.indices()
        , Apply(res_visit, eq_info.equation())
        , eq_info.scalar()
      });

      // Add residual variable vertices.
      std::string name = "res_" + modelica_bsbg.setVertex(se.var_id()).name();
      CompactSet res_vertices{se_res};
      res_vertices.translate(-se.translation());
      int res_var_id = modelica_bsbg.addSetVertex(res_vertices, name);

      // Modify matched variable.
      se.set_var_id(res_var_id);
      VariableRenamer renamer{"res_"};
      se.set_access(Apply(renamer, se.access()));
    }
  }
}

ModelicaSBG VerticalSortingBuilder::build(ModelicaSBG modelica_bsbg)
{
  SBG::Util::Internal::TimeProfiler profiler{"SBG Vertical Sorting builder"};

  _output_dsbg = _input_dsbg;

  // Add guess vertices.
  addGuessVertices();

  // Calculate start and end points for each SCC.
  SBG::LIB::PWMap reps_to_endpoint = _input_rmap.restrict(_residual_vertices)
    .minAdj(SBG::LIB::PWMap{_residual_vertices});
  _end_points = _guess_offset.composition(reps_to_endpoint).image();
  _end_points = _end_points.disjointCup(
    _input_rmap.fixedPoints().difference(reps_to_endpoint.domain())
  );
  SBG::LIB::PWMap residual_to_endpoint = reps_to_endpoint
    .composition(_input_rmap.restrict(_residual_vertices));
  SBG::LIB::PWMap Vid{_input_dsbg.V()};
  reps_to_endpoint = std::move(reps_to_endpoint).combine(Vid);
  residual_to_endpoint = std::move(residual_to_endpoint).combine(Vid);

  // Add edges between tearing variables.
  addDependencies(residual_to_endpoint);

  // Transform edges that connect different SCC so that the endings correspond
  // to start and end points of the SCC.
  redirectEdiff(reps_to_endpoint);

  addVariables(modelica_bsbg);

  // Modify Modelica SBG.
  modelica_bsbg = partition(modelica_bsbg);
  addGuessMatchs(modelica_bsbg);
  modifyResidualMatchs(modelica_bsbg);

  return modelica_bsbg;
}

} // namespace Causalize

} // namespace Modelica
