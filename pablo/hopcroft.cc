/* 
 G = U ∪ V ∪ {NIL}
 where U and V are partition of graph and NIL is a special null vertex
*/
  
// Cada nodo debería ser (v, mdi), pero debería haber un NULL, no??
// U = (u, mdi)
// V = (v, mdi)

struct PartVertex{
	PartVertex (v, mdi);
	Vertex v;
	MDI mdi;
};

#define NIL PartVertex(NIL_VERTEX, NIL_MDI)

// #define NIL = (NIL, mdiNIL)	
// Pair_U[u] = Lista de (vertex_vecino, mdi)

/* Quizá convenga que Dist[Vertex], sea lista de (mdi, dist) */

bool BFS (){
	Queue <PartVertex> queue;
	for (auto &ev : EQvertex){
		for (auto match : Pair_E[ev]){
			MDI mdi = match.first;
    if (match.second == NIL_VERTEX){
		Dist[u].set_mdi (mdi, 0);
		queue.push (PartVertex(ev, mdi));
	}
    else{
		Dist[u].set_mdi (mdi, INF);
	}
    Dist[NIL_VERTEX].set_mdi(NIL_MDI, INF);
    while (!queue.empty()){ 
        PartVertex u = queue.front();
		queue.pop();
        if Dist[u] < Dist[NIL] // Para todo par de U que se interseque, y que la distancia se menor que la de NIL, (u', mdiu'), guardo Dist[u'] = du'
            for each v in Adj[u] // Para todo (v, mdiv) que tengan intersección con los (u', mdiu')
                if Dist[ Pair_V[v] ] == ∞ // Para todo mdi que interseca (v', mdiv') con dist inf
                    Dist[ Pair_V[v] ] = Dist[u] + 1 // Esto es romper (TODO) dist[ (v, mdiv')] = du' + 1
                    Enqueue(Q,Pair_V[v]) // Encolo (v', mdiv')
    
	return Dist[NIL] != ∞
}

function BFS () // Hecha(ponele)
    for each u in U
        if Pair_U[u] == NIL // Para todo par de Pair_U[u], (v_vecino, mdiU)  
            Dist[u] = 0 // Dist[(u,mdiu)] = 0
            Enqueue(Q,u) // Enqueue (u, mdiu)
        else
            Dist[u] = ∞ // Dist[(u,all_mdiu)] = INF;
    Dist[NIL] = ∞ // Dist[NIL] = INF;
    while Empty(Q) == false  
        u = Dequeue(Q) // (u,mdiu)
        if Dist[u] < Dist[NIL] // Para todo par de U que se interseque, y que la distancia se menor que la de NIL, (u', mdiu'), guardo Dist[u'] = du'
            for each v in Adj[u] // Para todo (v, mdiv) que tengan intersección con los (u', mdiu')
                if Dist[ Pair_V[v] ] == ∞ // Para todo mdi que interseca (v', mdiv') con dist inf
                    Dist[ Pair_V[v] ] = Dist[u] + 1 // Esto es romper (TODO) dist[ (v, mdiv')] = du' + 1
                    Enqueue(Q,Pair_V[v]) // Encolo (v', mdiv')
    return Dist[NIL] != ∞

function DFS (u) // DFS (u, mdiu)
    if u != NIL  
        for each v in Adj[u] // For Adj[u] , si interseca con mdiu, sea mdiu' intersec y (v,mdiv)
            if Dist[ Pair_V[v] ] == Dist[u] + 1  // Para cada parte que se interseque de V con esos MDI's. Precalculo Dist[u]?
                if DFS(Pair_V[v]) == true // (V, mdiV) 
                    Pair_V[v] = u // Pair_V[v].pb (mdiV, u)
                    Pair_U[u] = v // Pair_U[u]
                    return true
        Dist[u] = ∞
        return false
    return true


int DFS (PartVertex pv){
		if (!isNil(pv.v)){
			Vertex v = pv.v;
			for (auto &adj : out_vertex(u)){
				
			}
			
		}
}

bool isNil (PartVertex pv){
	return pv.v == NIL_VERTEX;
}
// Supuestamente están: set_mdi, buscar_value
// Faltan: NIL_VERTEX (NIL_MDI)
//findMDIs, NIL_VERTEX, isNIL
int Hopcroft-Karp (){
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
		while (BFS()){
			for (auto &ev : EQvertex){
				vector <MDI> eps = buscar_value (Pair_E[ev], NIL_VERTEX);
				for (auto ep : eps){
					matching += DFS(pv);
				}
			}
		}
		return matching;
}

function Hopcroft-Karp
    for each u in U
        Pair_U[u] = NIL // Pair_U[u].pb(MDIentero, NIL) // DUDA1: "De donde saco el MDIentero?"
    for each v in V
        Pair_V[v] = NIL // Pair_V[V].pb(MDIentero, NIL)
    matching = 0 
    while BFS() == true
        for each u in U
            if Pair_U[u] == NIL // Buscamos para la parte de U que quedó en NIL
                if DFS(u) == true // matching += DFS(parte_MDI_NIL, u)
                    matching = matching + 1
    return matching
