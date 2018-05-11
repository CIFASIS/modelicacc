/* 
 G = U ∪ V ∪ {NIL}
 where U and V are partition of graph and NIL is a special null vertex
 
 
 Part_E[u] = Lista de (mdi, Label)
 Part_U[v] = Lista de (mdi, Label)
*/
#include <map>

typedef int Label;

typedef std::map <MDI, Label> MapMDI;
  // if .get();
  
Option <MDI> DFS (Vertex v, MDI mdi){ // visit, not_visited, inv_offset
	if (isNil(v)) return mdi; // Si es Nil retorno el MDI
	vector <MDI> nv_mdis = filter_not_visited(u, mdi); // Para que sea un dfs filtro por no visitados
	visit(v, mdi);
	for (auto nv_mdi : nv_mdis){
		for (auto &edge : out_edges(v)){ // Busco todas las aristas
			Unknown u = unknownFromEdge (edge); // Calculo la incognita de la arista
			MDI unk_mdi = offsetear (mdi, label); // En base al MDI de EQ, offseteo para tener el MDI del unknown correspondiente
			mapMDI match_mdis = get_match_mdis (Pair_U[u], unk_mdi); // Toda la información de los matcheos de U, que se los paso a E
			for (auto match_mdi : match_mdis){
				Option <MDI> matcheado_e = DFS (match_mdi.second.eq, match_mdi.first); 
				if (matcheado_e){
					MDI matcheado_u = offset (matcheado_e, match_mdi);
					MDI mdi_e = inv_offset (matcheado_v, edge);
					Pair_E[v].set_mdi(mdi_e, edge);
					Pair_U[u].set_mdi(matcheado_e, edge); 
					return mdi_e;
				}
			}
		}
	}
	return Option<MDI> (); // Return false
}


int DFS_Match (){
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
			vector <MDI> eps = buscar_value (Pair_E[ev], NIL_VERTEX); // Acá tiene que usarse buscar uno!
			for (auto ep : eps){
				if (Option <MDI> aux_mdi = DFS (ev, ep)){
					matching += aux_mdi.get().size;
					founded = true;
					break;
				}
			}
		}
	}
	return matching;
}
