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
	Match (IndexPair ip, Offset of, VectorVertex v, VectorEdge e): ip(ip), of(of), v(v), e(e){}
	IndexPair ip;
	Offset of;
	//~ Usage us;
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
	
	void inicializar_dfs(){
		Visitados.clear();
	}
	void contVisit(){
		for (auto it : Visitados)
			std::cout << "\nVISITADOS-i-SIZE    " << it.second.size() << std::endl;
	}
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
			for (auto dif : aux-mdi){
				rta[dif] = par.second;	
				//~ std::cout << "DIF    " << dif << std::endl;
				//~ std::cout << "Aux    " << aux << std::endl;
				//~ std::cout << "Mdi    " << mdi << std::endl;
			}	
		}
		rta[mdi] = Match (ip, ip.GetOffset(),v_match,e);
		Pair_E[v] = rta;
	}
	
	void set_mdi_u (VectorVertex v, MDI mdi, IndexPair ip, VectorVertex v_match, VectorEdge e = VectorEdge()){
		MapMDI rta;
		for (auto par : Pair_U[v]){
			MDI aux = par.first;
			for (auto dif : aux-mdi)
				rta[dif] = par.second;		
		}
		rta[mdi] = Match (ip, ip.GetOffset(),v_match,e);
		Pair_U[v] = rta;
	}
	
	MDI domToRan (MDI dom, IndexPair ip){
		MDI rta = dom.ApplyUsage(ip.GetUsage(), ip.Ran());
		rta = rta.ApplyOffset(ip.GetOffset());
		return rta;
	}
	
	MDI ranToDom (MDI ran, IndexPair ip){
		MDI rta = ran.RevertUsage(ip.GetUsage(), ip.Dom());
		rta = rta.ApplyOffset(-ip.GetOffset());
		return rta;
	}
	
	void testDR (IndexPair ip){
		if (ip.Dom() != ranToDom(ip.Ran(), ip) || ip.Ran() != domToRan(ip.Dom(), ip))
			std::cout << "--------------No son iguales!!!!!--------------\n" << "Dom: " << ip.Dom() << " Ran: " << ip.Ran() << 
			" DomToRan: " << domToRan(ip.Dom(), ip) << " RanToDom: " << ranToDom(ip.Ran(), ip) << std::endl;
	}
	
	int size_MDIS (std::list <MDI> &mdis){
		int rta = 0;
		for (auto mdi : mdis){
			rta += mdi.Size();
			//~ std::cout << mdi << "     "  << mdi.Size() << std::endl;

		}
		return rta;
	}
	
	bool differents (VectorEdge e1, VectorEdge e2, IndexPair ip1, IndexPair ip2){
		return e1 != e2 || ip1.Dom() != ip2.Dom() || ip1.Ran() != ip2.Ran();
	}
	
	bool dom_unique (VectorVertex ev, VectorEdge e1, IndexPair ip1, MDI mdi, VectorCausalizationGraph graph){
		std::list <MDI> resto;
		std::list <MDI> aux;
		resto.push_back (mdi);
		foreach_(VectorEdge edge, out_edges(ev,graph)) {
			IndexPairSet ips = graph[edge].Pairs();
			for (auto ip : ips){
				if (differents(e1, edge, ip1, ip)){
					//~ std::cout << ip << std::endl;
					for (auto r : resto){
						std::list <MDI> toAdd = r - ip.Dom();
						for (auto add : toAdd)
							aux.push_back(add);
					}
					resto = aux;
					aux.clear();
				}
			}
		}
		//~ std::cout << "DOM-UNIQUE     " << size_MDIS(resto) << "    " << resto << std::endl;
		return size_MDIS(resto);
	}
	
	bool ran_unique (VectorVertex uv, VectorEdge e1, IndexPair ip1, MDI mdi, VectorCausalizationGraph graph){
		std::list <MDI> resto;
		std::list <MDI> aux;
		resto.push_back (mdi);
		foreach_(VectorEdge edge, out_edges(uv,graph)) {
			IndexPairSet ips = graph[edge].Pairs();
			for (auto ip : ips){
				if (differents(e1, edge, ip1, ip)){
					//~ std::cout << ip << std::endl;

					for (auto r : resto){
						std::list <MDI> toAdd = r - ip.Ran();
						for (auto add : toAdd)
							aux.push_back(add);
					}
					resto = aux;
					aux.clear();
				}
			}
		}
		//~ std::cout << "RAN-UNIQUE     " << size_MDIS(resto) << "    " << resto << std::endl;

		return size_MDIS(resto);
	}
	
	bool dom_matched (VectorVertex ev, MDI mdi, VectorCausalizationGraph graph){
		std::list<MDI> nil_mdi = buscar_NIL (Pair_E[ev], graph);
		int tamano = mdi.Size();
		for (auto it : nil_mdi){
			if(auto aux = it & mdi)
				tamano -= aux.get().Size();
		}
		//~ std::cout << "TAMAÑO-DOM    " << tamano << std::endl;
		return tamano != 0; // Lo que no es NIL.		
	}
	bool ran_matched (VectorVertex uv, MDI mdi, VectorCausalizationGraph graph){
		std::list<MDI> nil_mdi = buscar_NIL (Pair_U[uv], graph);
		int tamano = mdi.Size();
		for (auto it : nil_mdi){
			if(auto aux = it & mdi)
				tamano -= aux.get().Size();
		}
		//~ std::cout << "TAMAÑO-RAN    " << tamano << std::endl;

		return tamano != 0; // Lo que no es NIL.	
	}
	
	int heuristica_inicial(VectorCausalizationGraph graph, std::list<Causalize::VectorVertex> &EQVertex){ // Elije el matching inicial de una forma heurística para ahorrarse casos complicados
		for (auto &ev : EQVertex){
			foreach_(VectorEdge edge, out_edges(ev,graph)) {
				for (auto ip : graph[edge].Pairs()){
					VectorVertex uv = target (edge,graph);
					if (dom_matched(ev, ip.Dom(), graph)) continue;
					if (ran_matched(uv, ip.Ran(), graph)) continue;
					if (dom_unique(ev, edge, ip, ip.Dom(), graph) || ran_unique(uv, edge, ip, ip.Ran(), graph)) {
						set_mdi_e(ev, ip.Dom(), ip, uv, edge);
						set_mdi_u(uv, ip.Ran(), ip, ev, edge);	
						return ip.Dom().Size();					
					}
				}
			}
		}
		return 0;
	}
	
	bool isOK (VectorCausalizationGraph graph, int matching){
		
		VectorCausalizationGraph::vertex_iterator vi, vi_end;
		int equationNumber = 0, unknownNumber = 0;
		for(boost::tie(vi, vi_end) = vertices(graph); vi != vi_end; vi++){
			VectorVertex current_element = *vi;
			if(graph[current_element].type == kVertexEquation){
				equationNumber += graph[current_element].count;
			}
			else{
				unknownNumber += graph[current_element].count;
			}
		}
		if(equationNumber != matching){
			printf("The model being causalized is not full-matched.\n"
			  "There are %d equations and the matching is %d\n", 
			  equationNumber, matching);
			return false;
		}
		return true;
	}
	
	Option <MDI> DFS (VectorVertex v, MDI mdi, VectorCausalizationGraph graph){ // visit, not_visited, inv_offset
		//~ std::cout << graph[v].equation <<  "      "  << mdi << std::endl;

		if (isNil(v, graph)) return mdi; // Si es Nil retorno el MDI
		//~ std::cout << "MDI     " << mdi << std::endl;
		std::list <MDI> nv_mdis = filter_not_visited(v, mdi); // Para que sea un dfs filtro por no visitados

		visit(v, mdi);

		for (auto nv_mdi : nv_mdis){

			foreach_(VectorEdge edge, out_edges(v,graph)) { // Busco todas las aristas

				VectorVertex u = GetUnknown (edge, graph); // Calculo la incognita de la arista
				for (auto ip : graph[edge].Pairs()){

					//TODO (karupayun): Pensar. Necesito aparte del Label el ip correspondiente? Capaz que si. Que necesito??
					//~ std::cout << "nv_MDI   " << nv_mdi << std::endl;
					//~ std::cout << "ip.Dom()   " << ip.Dom() << std::endl;

					Option <MDI> inter_mdi = nv_mdi & ip.Dom();
					if (!inter_mdi) continue;
					//~ std::cout << "Inter_MDI   " << inter_mdi.get() << std::endl;
					MDI unk_mdi = domToRan(inter_mdi.get(), ip);
					//~ MDI unk_mdi = inter_mdi.get().ApplyOffset (ip.GetOffset()); // En base al MDI de EQ, offseteo para tener el MDI del unknown correspondiente
					

					MapMDI match_mdis = get_match_mdis (Pair_U[u], unk_mdi); // Toda la información de los matcheos de U, que se los paso a E

					for (auto match_mdi : match_mdis){
					
						MDI dfs_mdi_e = ranToDom(match_mdi.first, match_mdi.second.ip);
						//~ MDI dfs_mdi_e = match_mdi.first.ApplyOffset(-match_mdi.second.of);
						Option <MDI> dfs_matcheado_e = DFS (match_mdi.second.v, dfs_mdi_e, graph); 
						if (dfs_matcheado_e){
							MDI matcheado_u = domToRan(dfs_matcheado_e.get(), match_mdi.second.ip);
							//~ MDI matcheado_u = dfs_matcheado_e.get().ApplyOffset (match_mdi.second.of);
							MDI mdi_e = ranToDom(matcheado_u, ip);
							//~ MDI mdi_e = matcheado_u.ApplyOffset(-ip.GetOffset());
							//~ std::cout << " Matcheado_E   " << dfs_matcheado_e.get() << std::endl;
							//~ std::cout << " Matcheado_U   " << matcheado_u << std::endl;
							//~ std::cout << " MDI_E   " << mdi_e << std::endl;
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

	int dfs_matching (VectorCausalizationGraph graph, std::list<Causalize::VectorVertex> &EQVertex, 
										std::list<Causalize::VectorVertex> &UVertex){
		VectorVertexProperty NIL;
		NIL.type = kNilVertex;
		VectorVertex NIL_VERTEX = add_vertex(NIL, graph); // TEST: Lo estará agregando? 
		for (auto &ev : EQVertex){
			foreach_(VectorEdge e1, out_edges(ev,graph)) {
				for (auto ip : graph[e1].Pairs()){
					

						set_mdi_e (ev, ip.Dom(), ip, NIL_VERTEX);  // TODO: No olvidar setear los offset y esas cosas para el DFS
					//~ if (!ip.GetUsage().isUnused()){	
						
						//~ std::cout << "EQ   " << graph[ev].equation << std::endl;
						//~ std::cout << "Unk   " << graph[target(e1, graph)].unknown() << std::endl;
						//~ std::cout << "IP.DOM()   " << ip.Dom() << std::endl;
						//~ std::cout << "IP.RAN()   " << ip.Ran() << std::endl;
						//~ std::cout << "IP.OFF()   "; 
						//~ for (auto it : ip.GetOffset())
						  //~ std::cout << it << " ";
						//~ std::cout << std::endl;
						//~ std::cout << "IP.USA()   "; 
						//~ for (auto it : ip.GetUsage())
						  //~ std::cout << it << " ";
						//~ std::cout << std::endl;
						testDR (ip);
						//~ std::cout << "IP.domToRan()   " << domToRan(ip.Dom(), ip) << std::endl;
						//~ std::cout << "IP.ranToDom()  " << ranToDom(ip.Ran(), ip) << std::endl;
					//~ }
				}
			}
		}
		//~ return 0;
		for (auto &uv : UVertex){
			foreach_(VectorEdge e1, out_edges(uv,graph)) {
				for (auto ip : graph[e1].Pairs()){
					set_mdi_u (uv, ip.Ran(), ip, NIL_VERTEX);
				}
			}
		}
	
		int matching = 0; // TODO:karupayun( TESTEAR LA HEURISTICA)
		while (true){ // Matchea con la heurística inicial suponiendo que las aristas van completan.
			int new_matching = heuristica_inicial(graph, EQVertex);
			if (!new_matching) break;
			matching += new_matching;
			//~ std::cout << "TAMAÑO-MATCH    " << matching << std::endl;

		}
		//~ return matching;
		
		bool founded = !isOK(graph, matching);
		
		while (founded){ 

			founded = false;
			inicializar_dfs(); 

			for (auto &ev : EQVertex){
				if (founded) break;
				std::list <MDI> eps = buscar_NIL (Pair_E[ev], graph); // Acá tiene que usarse buscar uno!
				
				for (auto ep : eps){
					if (Option <MDI> aux_mdi = DFS (ev, ep, graph)){
						//~ std::cout << "\nEQ VERTEX:   " << graph[ev].equation << std::endl;
						matching += aux_mdi.get().Size();
						founded = true;
						//~ std::cout << "MATCHING   " << matching << std::endl;
						break;
					}
				}
			}
		}
		//~ return matching;
		//~ for (auto &ev : EQVertex)
				//~ for (auto mmdi : Pair_E[ev]){
						//~ std::cout << "\nEQ: " << graph[ev].equation << " MDI: " << mmdi.first << " UNK: " << graph[mmdi.second.v].unknown() << std::endl << std::endl; 
				//~ }
		for (auto &uv : UVertex)
				for (auto mmdi : Pair_U[uv]){
						std::cout << "\nMatcheamos la Incognita: " << graph[uv].unknown() << " en el rango: " << mmdi.first << " con la ecuación:\n" << graph[mmdi.second.v].equation << " en el rango " << ranToDom(mmdi.first, mmdi.second.ip) << std::endl << std::endl; 
				}
		isOK (graph, matching);
						std::cout << matching << std::endl;

		return matching;
	}
	
	struct PalPair{ // De cada Vertex, MDI a esto:
		VectorVertex v;
		VectorEdge e; // Puede estar vacía si el v es NIL
	};

		
  
} // Causalize

