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

	bool VectorTarjan::Identical (TarjanVertex v1, VectorEdge ei, IndexPair ip){
		if (graph[source(ei, graph)].equation != tgraph[v1].equation) return false;
		if (graph[target(ei, graph)].unknown() != tgraph[v1].unknown()) return false;
		if (ip.GetOffset() != tgraph[v1].ip.GetOffset()) return false;
		if (ip.GetUsage() != tgraph[v1].ip.GetUsage()) return false;
		return true;
	}
	
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
			int counter = 1;
			for (auto p : Pair_E){
				VectorVertex v = p.first;
				for (auto mm : p.second){
					dprint(graph[v].equation);
					TarjanVertex tv = add_vertex (tgraph);
					tgraph[tv].equation = graph[v].equation;
					tgraph[tv].unknown = graph[mm.second.v].unknown();
					tgraph[tv].ip = IndexPair (mm.first, mm.first.DomToRan (mm.second.ip), mm.second.ip.GetOffset(), mm.second.ip.GetUsage());
					tgraph[tv].rest = std::list <MDI> ();
					tgraph[tv].rest.push_back (mm.first);
					tgraph[tv].number = counter++;
				}				
			}
			
			VectorCausalizationGraph::edge_iterator ei, ei_end; 
			TarjanGraph::vertex_iterator v1, v1_end, v2, v2_end; 
			foreach_(VectorEdge ei, edges(graph)) {
				for (auto ip : graph[ei].Pairs()){
					std::list<MDI> dom; dom.push_back(ip.Dom());
					std::list<MDI> ran; ran.push_back(ip.Ran());
					
					for(boost::tie(v1, v1_end) = vertices(tgraph); v1 != v1_end; v1++){
						if (Identical(*v1, ei, ip)){
							dom = resta (dom, tgraph[*v1].ip.Dom());
							ran = resta (ran, tgraph[*v1].ip.Ran());
							//~ MDIL ran2 = DomToRanList (ip, dom, ran);
							//~ ran = inter1a1 (ran, ran2);
														
							//~ MDIL dom2 = RanToDomList (ip, ran, dom);
							//~ dom = inter1a1 (dom, dom2);

							//~ break;
						} 
					}
					
					//~ dprint(size(ran));
					//~ dprint(size(dom));
					if (size(ran) != size(dom)) { // Parche en el caso de 1 con todos
						if (size(ran) == 0){
							for (auto _ : dom){
								ran.push_back(ip.Ran());
							}
						}
						else if (size(dom) == 0){
							for (auto _ : ran){
								ran.push_back(ip.Dom());
							}
						}
						else assert(false);
					}
						
					
					if (sum_size(dom) == 0) continue; // Si es 0 no queda nada
					if (sum_size(ran) == 0) continue; // Si es 0 no queda nada
					//~ return;
					auto mran = ran.begin();
					// TODO(karupayun): Ver el ejemplo de complicated. bin/causalize -v test/causalize/OneDHeatTransferTI_FD_complicated.mo
					for (auto mdom : dom){
						for(boost::tie(v1, v1_end) = vertices(tgraph); v1 != v1_end; v1++){
							if (graph[source(ei, graph)].equation == tgraph[*v1].equation){
								if (!(tgraph[*v1].ip.Dom() & mdom)) continue;
								MDI new_dom = (tgraph[*v1].ip.Dom() & mdom).get();
								for(boost::tie(v2, v2_end) = vertices(tgraph); v2 != v2_end; v2++){
									if (graph[target(ei, graph)].unknown() == tgraph[*v2].unknown()){
										if (!(tgraph[*v2].ip.Ran() & *mran)) continue;
										MDI new_ran = (tgraph[*v2].ip.Ran() & *mran).get();
										if (!(new_dom.DomToRan(ip) & new_ran)) continue;
										if (!(new_ran.RanToDom(ip) & new_dom)) continue;
										new_ran = (new_ran & new_dom.DomToRan(ip)).get();
										new_dom = (new_dom & new_ran.RanToDom(ip)).get();
										TarjanEdge te = add_edge (*v1, *v2, tgraph).first;
										tgraph[te].dom = new_dom;
										tgraph[te].ran = new_ran;
										dprint('\n');
										dprint(tgraph[*v1].number);
										dprint(tgraph[*v2].number);
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

			
			for(boost::tie(v1, v1_end) = vertices(tgraph); v1 != v1_end; v1++){
				dprint('\n');
				dprint(tgraph[*v1].number);
				dprint(tgraph[*v1].equation);
				dprint(tgraph[*v1].unknown());
				dprint(tgraph[*v1].ip.Dom());
				dprint(tgraph[*v1].ip.Ran());
			}

			
			strongly_connected_component = std::list <std::list <TarjanPart> > ();
			lowlinks = std::map <TarjanPart, int> ();
	};
} // Causalize

