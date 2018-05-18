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
    along with Modelica C Compiler.  If not, see <http:  www.gnu.org/licenses/>.

******************************************************************************/

/*
 *  Created on: 7 May 2018
 *      Author: Pablo Zimmermann
 */

#include <boost/graph/adjacency_list.hpp>
#include <causalize/vector/vector_matching.h>
#include <causalize/vector/vector_graph_definition.h>
#include <causalize/vector/causalization_algorithm.h>
#include <map>
#include <list>
#include <vector>

namespace Causalize{

struct Match{
	Match (Label lab, VectorVertex v, VectorEdge e): lab(lab), v(v), e(e){}
	Label lab;
	VectorVertex v;
	VectorEdge e;	
};
  
typedef std::map <MDI, Match> MapMDI;

	Vertex GetEquation(Edge e, VectorCausalizationGraph graph) {
	  return ((graph[(source(e,graph))].type==kVertexEquation))?source(e,graph):target(e,graph);
	}


	Vertex GetUnknown(Edge e, VectorCausalizationGraph graph) {
	  return ((graph[(target(e,graph))].type==kVertexUnknown))?target(e,graph):source(e,graph);
	}

  
	bool isNil (VectorVertex v, VectorCausalizationGraph graph){
		return graph[v].type == kNilVertex;
	}
	// TODO(karupayun): Ver que son el Usage y el Offset
	std::map <VectorVertex, MapMDI> Pair_E;
	std::map <VectorVertex, MapMDI> Pair_U;
	std::map <VectorVertex, std::list <MDI>> Visitados;
	
	void visit (VectorVertex v, MDI mdi){
		Visitados[v].push_back(mdi);		
	}
	
	std::list <MDI> filter_not_visited (VectorVertex v, MDI mdi){
		std::list <MDI> rta (1,mdi);
		for (auto vis : Visitados[v]){
			std::list <MDI> new_list;
			for (auto act_mdi : rta){
				new_list.splice(new_list.end(), act_mdi-vis);
			}
			rta = new_list;
		}
		return rta;
	}
	
	std::list <MDI> buscar_mdi (MapMDI lv, MDI mdi){
		std::list <MDI> rta;
		for (auto par : lv){
			Option <MDI> inter_mdi = par.first & mdi;
			if (inter_mdi){
				rta.push_back (inter_mdi.get());
			}
		}
		return rta;
	}

	std::list <MDI> buscar_NIL (MapMDI lv, VectorCausalizationGraph graph){
		std::list <MDI> rta;
		for (auto par : lv){
			VectorVertex v = par.second.v;
			if (isNil(v, graph)){
				rta.push_back (par.first);
			}
		}
		return rta;
	}
	
	Option <MDI> DFS (VectorVertex v, MDI mdi, VectorCausalizationGraph graph){ // visit, not_visited, inv_offset
		if (isNil(v, graph)) return mdi; // Si es Nil retorno el MDI
		std::list <MDI> nv_mdis = filter_not_visited(v, mdi); // Para que sea un dfs filtro por no visitados
		visit(v, mdi);
		for (auto nv_mdi : nv_mdis){
			foreach_(VectorEdge edge, out_edges(v,graph)) { // Busco todas las aristas
				Vertex u = GetUnknown (edge, graph); // Calculo la incognita de la arista
				//TODO (karupayun): Pensar. Necesito aparte del Label el ip correspondiente? Capaz que si. Que necesito??
				MDI unk_mdi = mdi.ApplyOffset (graph[edge].Pairs()); // En base al MDI de EQ, offseteo para tener el MDI del unknown correspondiente
				mapMDI match_mdis = get_match_mdis (Pair_U[u], unk_mdi); // Toda la información de los matcheos de U, que se los paso a E
				for (auto match_mdi : match_mdis){
					Option <MDI> matcheado_e = DFS (match_mdi.second.eq, match_mdi.first); 
					if (matcheado_e){
						MDI matcheado_u = offset (matcheado_e, match_mdi);
						MDI mdi_e = inv_offset (matcheado_v, edge);
						Pair_E[v].set_mdi(mdi_e, edge, u);
						Pair_U[u].set_mdi(matcheado_e, edge, v); 
						return mdi_e;
					}
				}
			}
		}
		return Option<MDI> (); // Return false
	}

	Option <MDI> DFS (VectorVertex v, MDI mdi, VectorCausalizationGraph graph){ return Option<MDI> ();}

	int dfs_matching (VectorCausalizationGraph graph, std::list<Causalize::VectorVertex> EQVertex, 
										std::list<Causalize::VectorVertex> UVertex){
		VectorVertexProperty NIL;
		NIL.type = kNilVertex;
		VectorVertex NIL_VERTEX = add_vertex(NIL, graph); // TEST: Lo estará agregando? 
		
		for (auto &ev : EQVertex){
			foreach_(VectorEdge e1, out_edges(ev,graph)) {
				for (auto ip : graph[e1].Pairs()){
					Pair_E[ev].set_mdi (ip.Dom(), NIL_VERTEX); // TODO: No olvidar setear los offset y esas cosas para el DFS 
				}
			}
		}
		for (auto &uv : UVertex){
			foreach_(VectorEdge e1, out_edges(uv,graph)) {
				for (auto ip : graph[e1].Pairs()){
					Pair_U[uv].set_mdi (ip.Ran(), NIL_VERTEX); 
				}
			}
		}
		int matching = 0;

		bool founded = true;
		while (founded){ 
			founded = false;
			for (auto &ev : EQVertex){
				if (founded) break;
				std::vector <MDI> eps = buscar_NIL (Pair_E[ev]); // Acá tiene que usarse buscar uno!
				for (auto ep : eps){
					if (Option <MDI> aux_mdi = DFS (ev, ep, graph)){
						matching += aux_mdi.get().Size();
						founded = true;
						break;
					}
				}
			}
		}
		return matching;
	}
	
	struct PalPair{ // De cada Vertex, MDI a esto:
		VectorVertex v;
		VectorEdge e; // Puede estar vacía si el v es NIL
	};

		
  
} // Causalize

