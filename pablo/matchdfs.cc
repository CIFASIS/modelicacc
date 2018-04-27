/* 
 G = U ∪ V ∪ {NIL}
 where U and V are partition of graph and NIL is a special null vertex
 
 
 Part_E[u] = Lista de (mdi, Label)
 Part_U[v] = Lista de (mdi, Label)
*/


 



function DFS (u)
    visit (u);
	if u != NIL
        for each v in Adj[u]
			if vis = NotVisited (Pair_V[v])
                if vis'=DFS(vis)
                    Pair_V[v,vis'] = u
                    Pair_U[u] = v, vis' 
                    return true
        return false
    return true
//Option<std::pair<VectorEdge,IndexPairSet> > CausalizationStrategyVector::CanCausalize(VectorEquationVertex eq, VertexType vt, bool split) {

struct PartVertex{
	PartVertex (v, mdi);
	Vertex v;
	Vertex mv; // Vertice que te estas uniendo
	VectorEdge e;
	IndexPairSet ips;
	IndexPair ip;
};

typedef map <MDI, PartVertex> MapMDI;
  // if .get();
  
Option <MDI> DFS (Vertex v, MDI mdi){ // visit, not_visited, inv_offset
	if (isNil(v)) return mdi; // Si es Nil retorno el MDI
	vector <MDI> nv_mdi = filter_not_visited(u, mdi); // Para que sea un dfs filtro por no visitados
	visit(v, mdi);
	for (auto mdi_actual : nv_mdi){
		for (auto &edge : out_edges(v)){ // Busco todas las aristas
			Unknown u = unknownFromEdge (edge); // Calculo la incognita de la arista
			vector <MDI> mdis_unk = offsetear_filtrar (mdi, label); // En base al MDI de EQ, offseteo y filtro de lo usado para tener el MDI del unknown
			for (auto mdi_unk : mdis_unk){
				vector <mapMDI> match_mdis = buscar_todo (Pair_U[u], mdi_unk); // Toda la información de los matcheos de U, que se los paso a E
				for (auto match_mdi : match_mdis){
					MDI matcheado_e = DFS (match_mdi.V, match_mdi.ip.Dom()); //O algo así
					if (!empty(matcheado_e)){
						matcheado_u = inv_offset (matcheado_v);
						Pair_V[.set_mdi(arista??, U, matcheado_v); // TODO: toda la info
						Pair_U.set_mdi(arista??, V, matcheado_u); // + toda la info
						return matcheado_e;
					}
				}
			}
		}
	}
	return Option<MDI> (); // Return false
}


int DFS-Match (){
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
