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
 *  Created on: 21 Aug 2018
 *      Author: Pablo Zimmermann
 */

#include <boost/graph/adjacency_list.hpp>
#include <causalize/vector/vector_tarjan.h>
#include <causalize/vector/vector_graph_definition.h>
#include <causalize/vector/vector_matching.h>
#include <causalize/vector/causalization_algorithm.h>
#include <map>
#include <list>
#include <vector>

#define dprint(v) std::cout << #v"=" << v << std::endl //;)



namespace Causalize{

	MDIL inter1a1 (MDIL l1, MDIL l2){
		MDIL rta;
		assert (l1.size() == l2.size()); 
		auto pm2 = l2.begin();
		for (auto m1 : l1){
			if (m1 & *pm2)
				rta.push_back ((m1 & *pm2).get());
		}
		return rta;
	}

	MDIL DomToRanList (IndexPair ip, MDIL doml, MDIL raml){
		MDIL rta;
		auto pram = raml.begin();
		for (auto m : doml){
			ip.Ran() = *pram;
			rta.push_back(m.DomToRan(ip));
			pram++;
		}
		return rta;		
	}
	
	MDIL RanToDomList (IndexPair ip, MDIL raml, MDIL doml){
		MDIL rta;
		auto pdom = doml.begin();
		for (auto m : raml){
			ip.Dom() = *pdom;
			rta.push_back(m.DomToRan(ip));
			pdom++;
		}
		return rta;		
	}

	
	std::list <MDI> resta (std::list<MDI> lmdi, MDI mdi){
		std::list<MDI> rta;
		for (auto r : lmdi){
			std::list <MDI> toAdd = r - mdi;
			for (auto add : toAdd)
					rta.push_back(add);
		}
		return rta;
	}
	
	std::list <MDI> DomToRanList (IndexPair ip, std::list <MDI> doms){
		std::list <MDI> rta;
		for(auto dom : doms){
			rta.push_back (dom.DomToRan(ip));
		}
		return rta;
	}
		
	VectorTarjan::VectorTarjan(VectorCausalizationGraph graph, std::map <VectorVertex, MapMDI> Pair_E, std::map <VectorVertex, MapMDI> Pair_U) : graph(graph){
		int counter = 1, counter2 = 1;
		for (auto p : Pair_E){
			VectorVertex v = p.first;
			for (auto mm : p.second){
				//~ dprint(graph[v].equation);
				TarjanVertex tv = add_vertex (tgraph);
				tgraph[tv].equation = graph[v].equation;
				tgraph[tv].index = counter++;
				tgraph[tv].count = mm.first.Size();
				tgraph[tv].unknown = graph[mm.second.v].unknown;
				tgraph[tv].ip = IndexPair (mm.first, mm.first.DomToRan (mm.second.ip), mm.second.ip.GetOffset(), mm.second.ip.GetUsage());
				tgraph[tv].mdi = mm.first;
				tgraph[tv].number = counter2;
				tgraph[tv].edge = mm.second.e;
			}
			counter2++;				
		}
		
		VectorCausalizationGraph::edge_iterator ei, ei_end; 
		TarjanGraph::vertex_iterator v1, v1_end, v2, v2_end;
		//~ for(boost::tie(v1, v1_end) = vertices(tgraph); v1 != v1_end; v1++){
			//~ dprint('\n');
			//~ dprint(tgraph[*v1].number);
			//~ dprint(tgraph[*v1].equation);
			//~ dprint(tgraph[*v1].unknown());
			//~ dprint(tgraph[*v1].ip.Dom());
			//~ dprint(tgraph[*v1].ip.Ran());
		//~ } 
		foreach_(VectorEdge ei, edges(graph)) {
			for (auto ip : graph[ei].Pairs()){
			
				std::list<MDI> dom; dom.push_back(ip.Dom());
				std::list<MDI> ran; ran.push_back(ip.Ran());
				
				for(boost::tie(v1, v1_end) = vertices(tgraph); v1 != v1_end; v1++){
					if (tgraph[*v1].edge == ei){ // Borrar la parte del matching
						dom = resta (dom, tgraph[*v1].ip.Dom());
						ran = resta (ran, tgraph[*v1].ip.Ran());
					} 
				}
				//~ dprint(size(ran));
				//~ dprint(size(dom));
				if ((ran.size()) != (dom.size())) { // Parche en el caso de 1 con todos
					if ((ran.size()) == 0){
						for (auto _ : dom){
							ran.push_back(ip.Ran());
						}
					}
					else if ((dom.size()) == 0){
						for (auto _ : ran){
							ran.push_back(ip.Dom());
						}
					}
					else assert(false);
				}
							
				if (sum_size(dom) == 0) continue; // Si es 0 no queda nada
				if (sum_size(ran) == 0) continue; // Si es 0 no queda nada
				auto mran = ran.begin();
				for (auto mdom : dom){
					for(boost::tie(v1, v1_end) = vertices(tgraph); v1 != v1_end; v1++){
						if (graph[source(ei, graph)].equation == tgraph[*v1].equation){
							for(boost::tie(v2, v2_end) = vertices(tgraph); v2 != v2_end; v2++){
								if (graph[target(ei, graph)].unknown() == tgraph[*v2].unknown()){
									// Intersección dominios arista con v1
									if (!(tgraph[*v1].ip.Dom() & mdom)) continue; 
									MDI new_dom = (tgraph[*v1].ip.Dom() & mdom).get(); 
									
									// Intersección rangos arista con v2
									if (!(tgraph[*v2].ip.Ran() & *mran)) continue; 							
									MDI new_ran = (tgraph[*v2].ip.Ran() & *mran).get();

									// Interseco ambos rangos para ver las verdaderas dependencias
									if (!(new_dom.DomToRan(ip) & new_ran)) continue;
									if (!(new_ran.RanToDom(ip) & new_dom)) continue;
									new_ran = (new_ran & new_dom.DomToRan(ip)).get();
									new_dom = (new_dom & new_ran.RanToDom(ip)).get();
									
									// Solo vamos a guardar una arista de Dom a Dom
									new_ran = new_ran.RanToDom(tgraph[*v2].ip);
								
									if ((v1 == v2) && (new_dom == new_ran)) continue; // No have sense this kind of edges
									IndexPair new_ip (new_dom, new_ran, ip.GetOffset(), ip.GetUsage());
									IndexPairSet ips;
									ips.insert(new_ip);
									Label lab (ips);
									add_edge (*v1, *v2, lab, tgraph).first;

									//~ tgraph[te].dom = new_dom;
									//~ tgraph[te].ran = new_ran;
									//~ tgraph[te].Label = ips;
									//~ tgraph[te].ip = new_ip;
									
									//~ dprint(tgraph[*v1].number);
									//~ dprint(tgraph[*v2].number);
									//~ dprint('\n');
									//~ dprint(tgraph[te].dom);
									//~ dprint(tgraph[te].ran);
								}
							}
						}
					}
					mran++;
				}
			}
		}
	};

	// Busca el conjunto de nodos que representa el mdi y ese vértice. 
	//   Devuelve un conjunto disjunto de partes. El booleano representa si solo buscamos elementos no visitados anteriormente.
	MDIL VectorTarjan::find (TarjanVertex tv, MDI mdi, bool onlyNV){
		MDIL rta;
		for (auto pair : data){
			MDI mdi2 = pair.first.second;
			auto tv2 = pair.first.first;
			auto td = pair.second;
			if ((tv == tv2) && (mdi & mdi2)){
				if (mdi != mdi2){ // Debo quebrar
					if (td.id != -1){ // Visitado con otro MDI no disjunto, caso no resuelto todavía.
						dprint(td.id);
						dprint(mdi);
						dprint(mdi2);
						dprint(tgraph[tv].number);						
						isOk = false;
					}					
					data.erase(pair.first);
					VertexPart vp;
					for (auto m : mdi2 - mdi){
						vp = std::make_pair(tv2, m);
						data[vp] = td;
					}
					vp = std::make_pair(tv2, (mdi&mdi2).get());
					data[vp] = td;
					rta.push_back((mdi&mdi2).get());
				}
				if (td.id == -1 || !onlyNV)
					rta.push_back(mdi);
			}
		}
		return rta;
	}
	
	void VectorTarjan::DFS(TarjanVertex tv, MDI mdi){
		MDIL mdil = find (tv, mdi); // Función que se encarga de trabajar con los distintos conjuntos
		for (auto mdi : mdil){ // Marco como visitado
			VertexPart at = (std::make_pair (tv, mdi));
			data[at].id = data[at].low = id;
			data[at].onStack = true;
 			stack.push(at);
		}
		id++;
		for (auto mdi : mdil){ // DFS
			VertexPart at = (std::make_pair (tv, mdi)); 
			foreach_(TarjanEdge edge, out_edges(tv,tgraph)) {
				TarjanVertex tv2 = target(edge, tgraph);
				// Pasar pasar de Dom1 a Dom2, paso de Dom1 a Ran2 y de Ran2 a Dom2
				MDI mdi2 = mdi.DomToRan(*tgraph[edge].Pairs().begin()).RanToDom(tgraph[tv2].ip); 
				// --------------------------------------------
				if (!(tgraph[tv2].mdi & mdi2)) continue; // TODO(karupayun): Falta analizar que hacer si estamos cayendo a un MDI que no figuraba anteriormente. Esta línea deberíamos borrarla.
				MDIL mdil2 = find (tv2, (tgraph[tv2].mdi & mdi2).get(), false);
				for (auto m : mdil2){
					VertexPart to = std::make_pair(tv2,m);
					if (data[to].id == -1) DFS (to.first, to.second); // Si no está visitado, lo visitamos
					if (data[to].onStack) {
						data[at].low = std::min (data[at].low, data[to].low); // Si está en stack actualizamos el mínimo
					}
				}
			}
			if (data[at].id == data[at].low){
				ConnectedComponent scc;
				while (1){
					VertexPart node = stack.top();
					stack.pop(); // TODO(karupayun): Si cambió el conjunto esto no sería así. Pensar que hacer para trabajar ese caso difícil.
					data[node].onStack = false;
					data[node].low = data[at].id;
					scc.push_back(node);
					if (node == at) {
						strongly_connected_component.push_back(scc);
						break;
					}
				}
			}
		}
	}
	
	 bool VectorTarjan::GetConnectedComponent(std::list <CausalizeEquations> &scc){
		isOk = true;
		id = 0;
		
		TarjanGraph::vertex_iterator vi, vi_end;
		for(boost::tie(vi, vi_end) = vertices(tgraph); vi != vi_end; vi++){
			TarjanData td;
			td.id = -1;
			td.low = 0;
			td.onStack = false;
			
			VertexPart vp = std::make_pair (*vi, tgraph[*vi].mdi);
			data[vp] = td;
		}
		
		for (auto pair : data){
			if (pair.second.id == -1)
				DFS(pair.first.first, pair.first.second);
		}
	
		/* Si volvemos a un mismo vértice con otro rango del que salimos, lo vamos a consider como un caso no resuelto todavía. Si volvemos con el mismo, es trivial:
			 * Hay N ciclos.
			 * */
	 	for (auto cc : strongly_connected_component){
			//~ dprint("New");
			CausalizeEquations ces;
			for (auto vp:cc){ // vp = std::pair <TarjanVertex, MDI>
				MDI dom = vp.second;
				MDI ran = dom.DomToRan (tgraph[vp.first].ip);
				IndexPair ip (dom, ran, tgraph[vp.first].ip.GetOffset(), tgraph[vp.first].ip.GetUsage()); 
				IndexPairSet ips = {ip};
				//~ dprint(tgraph[vp.first].number);
				//~ dprint(tgraph[vp.first].unknown());
				//~ dprint(vp.second);
				CausalizedVar ce;
				ce.pairs = ips;
				ce.equation = tgraph[vp.first].equation;
				ce.unknown = tgraph[vp.first].unknown;
				ces.push_back(ce);
			}
			scc.push_back(ces);
		}		
		
		return isOk;
	}
} // Causalize

