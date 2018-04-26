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
	Vertex v; // Vertice que te estas uniendo
	VectorEdge e;
	IndexPairSet ips;
	IndexPair ip;
};

typedef map <MDI, PartVertex> MapMDI;
  
MDI DFS (Vertex v, MDI mdi){ // visit, not_visited, inv_offset
	visit(v, mdi);
	if (!isNil(v, mdi)){ // Filtrar la parte no nil, mepa. Pensar
		for (auto &edge : out_edges(v)){ // Busco todas las aristas
			vector <MDI> mdis_unk = offsetear (mdi, label); // En base al MDI de EQ, offseteo y filtro de lo usado para tener el MDI del unknown
			Unknown u = unknownFromEdge (edge); // Calculo la incognita de la arista
			for (auto mdi_unk : mdis_unk){
				vector <MDI> nvis = not_visited (u ,mdi_unk); // Para que sea un dfs filtro por no visitados
				visit (unk, mdi_unk); // Visito
				for (auto mdiu : nvis){ // Para cada MDIu no visitado tenés que ver con quien matchea.
//TODO(karupayun): Recuperar no solo el MDI sino todo.------------------------------------------------------------------------------
					vector <MDI> match_mdi = buscar_value (Pair_U[u], mdiu); 
					MDI matcheado_e = (DFS (match_mdi) = );
					if (!empty(matcheado_v)){
					  matcheado_u = inv_offset (matcheado_v);
					  Pair_V[.set_mdi(arista??, U, matcheado_v);
					  Pair_U.set_mdi(arista??, V, matcheado_u);
					}
//----------------------------------------------------------------------------------------------------------------------------------
				}
			}
		}
	}
}


int DFS-Match (){
	for (auto &ev : EQvertex){
		foreach_(VectorEdge e1, out_edges(ev,graph)) {
			for (auto ip : e1.pairs()){
				Pair_E[ev].set_mdi (ip.Dom(), NIL_VERTEX); 
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
	for (auto &ev : EQvertex){
		vector <MDI> eps = buscar_value (Pair_E[ev], NIL_VERTEX); // Acá tiene que usarse buscar uno!
		for (auto ep : eps){
			matching += DFS(ev, ep);
		}
	}
	return matching;
}
