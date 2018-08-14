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
		typedef std::map <MDI, Match> MapMDI;
//---------------------------------- Funciones Auxiliares ----------------------------------/
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

	void VectorMatching::check(VectorVertex ev, IndexPair ip){
		if (ip.Dom() == ranToDom(ip.Ran(), ip) && ip.Ran() == domToRan(ip.Dom(), ip)) return;
		std::cout << "EQ   " << graph[ev].equation << std::endl;
		std::cout << "IP.DOM()   " << ip.Dom() << std::endl;
		std::cout << "IP.RAN()   " << ip.Ran() << std::endl;
		std::cout << "IP.OFF()   "; 
		for (auto it : ip.GetOffset()) std::cout << it << " ";
		std::cout << std::endl;
		std::cout << "IP.USA()   "; 
		for (auto it : ip.GetUsage()) std::cout << it << " ";
		std::cout << std::endl;
		std::cout << "IP.domToRan()   " << domToRan(ip.Dom(), ip) << std::endl;
		std::cout << "IP.ranToDom()  " << ranToDom(ip.Ran(), ip) << std::endl;
	}	

	// Cuantos MDI's tenemos en la lista
	int size_MDIS (std::list <MDI> &mdis){
		int rta = 0;
		for (auto mdi : mdis){
			rta += mdi.Size();
		}
		return rta;
	}
	
	bool differents (VectorEdge e1, VectorEdge e2, IndexPair ip1, IndexPair ip2){
		return e1 != e2 || ip1.Dom() != ip2.Dom() || ip1.Ran() != ip2.Ran();
	}
//---------------------------------- ------------------------------------------------------/



	bool VectorMatching::isNil (VectorVertex v){
		return graph[v].type == kNilVertex;
	}
	
	// Filtra los no-visitados para el dfs
	std::list <MDI> VectorMatching::filter_not_visited (VectorVertex v, MDI mdi){
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
	
	// Busca solo los MDI que no estén matcheados
	std::list <MDI> VectorMatching::buscar_NIL (MapMDI lv){
		std::list <MDI> rta;
		for (auto par : lv){
			VectorVertex v = par.second.v;
			if (isNil(v)){
				rta.push_back (par.first);
			}
		}
		return rta;
	}

	MapMDI VectorMatching::get_match_mdis (MapMDI map_unk, MDI unk_mdi){
		MapMDI rta;
		Option <MDI> aux;
		for (auto par : map_unk){
			if (aux = par.first & unk_mdi){
				rta[aux.get()] = par.second;
			}
		}
		return rta;	
	} 
	
	// dado V y MDI, matcheo V-MDI con el V_match usando la arista e
	void VectorMatching::set_mdi_e (VectorVertex v, MDI mdi, IndexPair ip, VectorVertex v_match, VectorEdge e = VectorEdge()){
		MapMDI rta;
		for (auto par : Pair_E[v]){
			MDI aux = par.first;
			for (auto dif : aux-mdi){
				rta[dif] = par.second;	
			}	
		}
		rta[mdi] = Match (ip, v_match,e);
		Pair_E[v] = rta;
	}
	
	void VectorMatching::set_mdi_u (VectorVertex v, MDI mdi, IndexPair ip, VectorVertex v_match, VectorEdge e = VectorEdge()){
		MapMDI rta;
		for (auto par : Pair_U[v]){
			MDI aux = par.first;
			for (auto dif : aux-mdi)
				rta[dif] = par.second;		
		}
		rta[mdi] = Match (ip, v_match, e);
		Pair_U[v] = rta;
	}



// --------------------------------------------- Heurística Inicial --------------------------------------------------//
	// Usado en la heurística, matcheamos la arista si tiene alguna parte del dominio que nadie más tiene
	bool VectorMatching::is_dom_unique (VectorVertex ev, VectorEdge e1, IndexPair ip1, MDI mdi){
		std::list <MDI> resto;
		std::list <MDI> aux;
		resto.push_back (mdi);
		foreach_(VectorEdge edge, out_edges(ev,graph)) {
			IndexPairSet ips = graph[edge].Pairs();
			for (auto ip : ips){
				if (differents(e1, edge, ip1, ip)){
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
		return size_MDIS(resto);
	}
	
	bool VectorMatching::is_ran_unique (VectorVertex uv, VectorEdge e1, IndexPair ip1, MDI mdi){
		std::list <MDI> resto;
		std::list <MDI> aux;
		resto.push_back (mdi);
		foreach_(VectorEdge edge, out_edges(uv,graph)) {
			IndexPairSet ips = graph[edge].Pairs();
			for (auto ip : ips){
				if (differents(e1, edge, ip1, ip)){
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
		return size_MDIS(resto);
	}
	
	// Usado en la heurística, matchea aristas en su totalidad, por lo que si una parte ya está usada no se usa.
	bool VectorMatching::is_dom_matched (VectorVertex ev, MDI mdi){
		std::list<MDI> nil_mdi = buscar_NIL (Pair_E[ev]);
		int tamano = mdi.Size();
		for (auto it : nil_mdi){
			if(auto aux = it & mdi)
				tamano -= aux.get().Size();
		}
		return tamano != 0; 		
	}
	
	bool VectorMatching::is_ran_matched (VectorVertex uv, MDI mdi){
		std::list<MDI> nil_mdi = buscar_NIL (Pair_U[uv]);
		int tamano = mdi.Size();
		for (auto it : nil_mdi){
			if(auto aux = it & mdi)
				tamano -= aux.get().Size();
		}
		return tamano != 0; 	
	}
	
	// Elije el matching inicial de una forma heurística para ahorrarse casos complicados
	int VectorMatching::heuristica_inicial(){ 
		for (auto &ev : eqDescriptors){
			foreach_(VectorEdge edge, out_edges(ev,graph)) {
				for (auto ip : graph[edge].Pairs()){
					VectorVertex uv = target (edge,graph);
					if (is_dom_matched(ev, ip.Dom())) continue;
					if (is_ran_matched(uv, ip.Ran())) continue;
					if (is_dom_unique(ev, edge, ip, ip.Dom()) || is_ran_unique(uv, edge, ip, ip.Ran())) {
						set_mdi_e(ev, ip.Dom(), ip, uv, edge);
						set_mdi_u(uv, ip.Ran(), ip, ev, edge);	
						return ip.Dom().Size();					
					}
				}
			}
		}
		return 0;
	}
// ------------------------------------------------------------------------------------------------------------------ //

	bool VectorMatching::isOK (int matching, bool print_message = false){
		
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
			if (print_message)
				printf("The model being causalized is not full-matched.\n"
					"There are %d equations and the matching is %d\n", 
					equationNumber, matching);
			return false;
		}
		return true;
	}
	
	Option <MDI> VectorMatching::DFS (VectorVertex v, MDI mdi){ // visit, not_visited, inv_offset
		if (isNil(v)) return mdi; // Si es Nil retorno el MDI
		std::list <MDI> nv_mdis = filter_not_visited(v, mdi); // Para que sea un dfs filtro por no visitados
		visit(v, mdi);
		for (auto nv_mdi : nv_mdis){
			foreach_(VectorEdge edge, out_edges(v,graph)) { // Busco todas las aristas
				VectorVertex u = target(edge,graph); // Calculo la incognita de la arista
				for (auto ip : graph[edge].Pairs()){
					Option <MDI> inter_mdi = nv_mdi & ip.Dom();
					if (!inter_mdi) continue;
					MDI unk_mdi = domToRan(inter_mdi.get(), ip);
					MapMDI match_mdis = get_match_mdis (Pair_U[u], unk_mdi); // Toda la información de los matcheos de U, que se los paso a E
					for (auto match_mdi : match_mdis){
						MDI dfs_mdi_e = ranToDom(match_mdi.first, match_mdi.second.ip);
						Option <MDI> dfs_matcheado_e = DFS (match_mdi.second.v, dfs_mdi_e); 
						if (dfs_matcheado_e){
							MDI matcheado_u = domToRan(dfs_matcheado_e.get(), match_mdi.second.ip);
							MDI mdi_e = ranToDom(matcheado_u, ip);
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

	int VectorMatching::dfs_matching (){
		VectorVertexProperty NIL;
		NIL.type = kNilVertex;
		VectorVertex NIL_VERTEX = add_vertex(NIL, graph);
		for (auto &ev : eqDescriptors){
			foreach_(VectorEdge e1, out_edges(ev,graph)) {
				for (auto ip : graph[e1].Pairs()){
					check(ev, ip);
					set_mdi_e (ev, ip.Dom(), ip, NIL_VERTEX);
				}
			}
		}
		for (auto &uv : uDescriptors){
			foreach_(VectorEdge e1, out_edges(uv,graph)) {
				for (auto ip : graph[e1].Pairs()){
					set_mdi_u (uv, ip.Ran(), ip, NIL_VERTEX);
				}
			}
		}
	
		int matching = 0; 
		while (true){ // Matchea con la heurística inicial suponiendo que las aristas van completan.
			int new_matching = heuristica_inicial();
			if (!new_matching) break;
			matching += new_matching;
		}
	
		bool founded = !isOK(matching);
		while (founded){ 

			founded = false;
			inicializar_dfs(); 

			for (auto &ev : eqDescriptors){
				if (founded) break;
				std::list <MDI> eps = buscar_NIL (Pair_E[ev]); // Acá tiene que usarse buscar uno!
				for (auto ep : eps){
					if (Option <MDI> aux_mdi = DFS (ev, ep)){
						matching += aux_mdi.get().Size();
						founded = true;
						break;
					}
				}
			}
		}
		for (auto &uv : uDescriptors)
				for (auto mmdi : Pair_U[uv]){
						std::cout << "\nMatcheamos la Incognita: " << graph[uv].unknown() << " en el rango: " << mmdi.first << " con la ecuación:\n" << graph[mmdi.second.v].equation << " en el rango " << ranToDom(mmdi.first, mmdi.second.ip) << std::endl << std::endl; 
				}
		isOK (matching, true);
		return matching;
	}
	
 
} // Causalize

