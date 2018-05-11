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

#include <causalize/vector/vector_matching.h>
#include <causalize/vector/vector_graph_definition.h>
#include <map>
#include <vector>

namespace Causalize{
  
typedef std::map <MDI, Label> MapMDI;
  
	bool isNil (VectorVertex v, VectorCausalizationGraph graph){
		return graph[v].type == kNilVertex;
	}
	
	std::map <VectorVertex, MapMDI> Pair_E;
	std::map <VectorVertex, MapMDI> Pair_U;
	
	vector <MDI> buscar_mdi (MapMDI lv, MDI mdi){
		vector <MDI> rta;
		for (auto par : lv){
			if (interseca (par.first, mdi)){
				rta.push_back (intersection (par.first, mdi));
			}
		}
		return rta;
	}

	vector <MDI> buscar_value (MapMDI lv){
		vector <MDI> rta;
		for (auto par : lv){
			IndexPairSet ips = par.second.ips;
			for (auto ip : ips){
				if (isNil(ip.second){
					rta.push_back (par.first);
				}
			}
		}
		return rta;
	}
	
	
	Option <MDI> DFS (VectorVertex v, MDI mdi, VectorCausalizationGraph graph){ // visit, not_visited, inv_offset
		if (isNil(v, graph)) return mdi; // Si es Nil retorno el MDI
		std::vector <MDI> nv_mdis = filter_not_visited(v, mdi); // Para que sea un dfs filtro por no visitados
		visit(v, mdi);
		for (auto nv_mdi : nv_mdis){
			for (auto &edge : out_edges(v, graph)){ // Busco todas las aristas
				Unknown u = unknownFromEdge (edge); // Calculo la incognita de la arista
				MDI unk_mdi = offsetear (mdi, label); // En base al MDI de EQ, offseteo para tener el MDI del unknown correspondiente
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


	int dfs_matching (VectorCausalizationGraph graph){
		for (auto &ev : EQvertex){
			foreach_(VectorEdge e1, out_edges(ev,graph)) {
				for (auto ip : e1.pairs()){
					Pair_E[ev].set_mdi (ip.Dom(), NIL_VERTEX); // TODO: No olvidar setear los offset y esas cosas para el DFS 
				}
			}
		}
		for (auto &uv : Uvertex){
			foreach_(VectorEdge e1, out_edges(uv,graph)) {
				for (auto ip : e1.pairs()){
					Pair_U[uv].set_mdi (ip.Ran(), NIL_VERTEX); 
				}
			}
		}
		int matching = 0;

		bool founded = true;
		while (founded){ 
			founded = false;
			for (auto &ev : EQvertex){
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
		
  
} // Causalize

