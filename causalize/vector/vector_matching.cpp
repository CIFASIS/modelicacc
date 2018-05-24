/***
 * 
 * 
 * TODO: Tema de MatchStruct, está todo comentado
 * 
 * 
 * 
 * 
 * ****************/


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
  inline Match(){ };
	Match (Offset of, VectorVertex v, VectorEdge e): of(of), v(v), e(e){}
	//~ IndexPair ip;
	Offset of;
	VectorVertex v; // Matcheado
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
	void hola (std::list<Causalize::VectorVertex> &equationDescriptors, std::list<Causalize::VectorVertex> &unknownDescriptors){
		return;
	}
	MapMDI get_match_mdis (MapMDI map_unk, MDI unk_mdi){
		MapMDI rta;
		Option <MDI> aux;
		for (auto par : map_unk){
			if (aux = par.first & unk_mdi){
				rta[aux.get()] = par.second;
			}
		}
		return rta;	
	} 
	
	void set_mdi_e (VectorVertex v, MDI mdi, IndexPair ip, VectorVertex v_match, VectorEdge e = VectorEdge()){
		MapMDI rta;
		for (auto par : Pair_E[v]){
			MDI aux = par.first;
			for (auto dif : aux-mdi)
				rta[dif] = par.second;		
		}
		rta[mdi] = Match (ip.GetOffset(),v_match,e);
	}
	
	void set_mdi_u (VectorVertex v, MDI mdi, IndexPair ip, VectorVertex v_match, VectorEdge e = VectorEdge()){
		MapMDI rta;
		for (auto par : Pair_U[v]){
			MDI aux = par.first;
			for (auto dif : aux-mdi)
				rta[dif] = par.second;		
		}
		rta[mdi] = Match (ip.GetOffset(),v_match,e);
	}
	
	Option <MDI> DFS (VectorVertex v, MDI mdi, VectorCausalizationGraph graph){ // visit, not_visited, inv_offset
		if (isNil(v, graph)) return mdi; // Si es Nil retorno el MDI
		std::list <MDI> nv_mdis = filter_not_visited(v, mdi); // Para que sea un dfs filtro por no visitados
		visit(v, mdi);
		for (auto nv_mdi : nv_mdis){
			foreach_(VectorEdge edge, out_edges(v,graph)) { // Busco todas las aristas
				Vertex u = GetUnknown (edge, graph); // Calculo la incognita de la arista
				for (auto ip : graph[edge].Pairs()){
					//TODO (karupayun): Pensar. Necesito aparte del Label el ip correspondiente? Capaz que si. Que necesito??
					Option <MDI> inter_mdi = nv_mdi & ip.Dom();
					if (!inter_mdi) continue;
					MDI unk_mdi = inter_mdi.get().ApplyOffset (ip.GetOffset()); // En base al MDI de EQ, offseteo para tener el MDI del unknown correspondiente
					MapMDI match_mdis = get_match_mdis (Pair_U[u], unk_mdi); // Toda la información de los matcheos de U, que se los paso a E
					for (auto match_mdi : match_mdis){
						Option <MDI> matcheado_e = DFS (match_mdi.second.v, match_mdi.first, graph); 
						if (matcheado_e){
							MDI matcheado_u = matcheado_e.get().ApplyOffset (-match_mdi.second.of);
							MDI mdi_e = matcheado_u.ApplyOffset(ip.GetOffset());
							set_mdi_e(v, mdi_e, ip, u, edge);    
							set_mdi_u(u, matcheado_u, ip, v, edge);
							return mdi_e;
						}
					}
				}
			}
		}
		return Option<MDI> (); // Return false
	}

	int dfs_matching (VectorCausalizationGraph graph, std::list<Causalize::VectorVertex> EQVertex, 
										std::list<Causalize::VectorVertex> UVertex){
		VectorVertexProperty NIL;
		NIL.type = kNilVertex;
		VectorVertex NIL_VERTEX = add_vertex(NIL, graph); // TEST: Lo estará agregando? 
		
		for (auto &ev : EQVertex){
			foreach_(VectorEdge e1, out_edges(ev,graph)) {
				for (auto ip : graph[e1].Pairs()){
					set_mdi_e (ev, ip.Dom(), ip, NIL_VERTEX);  // TODO: No olvidar setear los offset y esas cosas para el DFS
				}
			}
		}
		for (auto &uv : UVertex){
			foreach_(VectorEdge e1, out_edges(uv,graph)) {
				for (auto ip : graph[e1].Pairs()){
					set_mdi_u (uv, ip.Ran(), ip, NIL_VERTEX);
				}
			}
		}
		int matching = 0;

		bool founded = true;
		while (founded){ 
			founded = false;
			for (auto &ev : EQVertex){
				if (founded) break;
				std::list <MDI> eps = buscar_NIL (Pair_E[ev], graph); // Acá tiene que usarse buscar uno!
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

