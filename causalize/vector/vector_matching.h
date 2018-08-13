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
    along with Modelica C Compiler.  If not, see <http://www.gnu.org/licenses/>.

******************************************************************************/

#ifndef VECTOR_MATCHING_
#define VECTOR_MATCHING_

#include <causalize/vector/vector_graph_definition.h>

namespace Causalize {

	class Matching{
	typedef std::map <MDI, Matching> MapMDI;

	public:
		Matching(){ };
		Matching(IndexPair ip, VectorVertex v, VectorEdge e): ip(ip), v(v), e(e){}
	  int dfs_matching(VectorCausalizationGraph graph, std::list<Causalize::VectorVertex> &equationDescriptors, std::list<Causalize::VectorVertex> &uDescriptors); // Llamarla desde Algorithm
		Option <MDI> DFS (VectorVertex v, MDI mdi, VectorCausalizationGraph graph);	
		

	private:
		void inicializar_dfs(){ Visitados.clear(); }
		void visit (VectorVertex v, MDI mdi){ Visitados[v].push_back(mdi); }
		std::list <MDI> filter_not_visited (VectorVertex v, MDI mdi);
		std::list <MDI> buscar_NIL (MapMDI lv, VectorCausalizationGraph graph);
		MapMDI get_match_mdis (MapMDI map_unk, MDI unk_mdi);
		void set_mdi_e (VectorVertex v, MDI mdi, IndexPair ip, VectorVertex v_match, VectorEdge e);
		void set_mdi_u (VectorVertex v, MDI mdi, IndexPair ip, VectorVertex v_match, VectorEdge e);
		bool is_dom_unique (VectorVertex ev, VectorEdge e1, IndexPair ip1, MDI mdi, VectorCausalizationGraph graph);
		bool is_ran_unique (VectorVertex uv, VectorEdge e1, IndexPair ip1, MDI mdi, VectorCausalizationGraph graph);
		bool is_dom_matched (VectorVertex ev, MDI mdi, VectorCausalizationGraph graph);
		bool is_ran_matched (VectorVertex uv, MDI mdi, VectorCausalizationGraph graph);
		int heuristica_inicial(VectorCausalizationGraph graph, std::list<Causalize::VectorVertex> &EQVertex);
		bool isOK (VectorCausalizationGraph graph, int matching);



		std::map <VectorVertex, MapMDI> Pair_E; // Con quien se aparean las ecuaciones
		std::map <VectorVertex, MapMDI> Pair_U; // Con quien se aparean las incognitas
		std::map <VectorVertex, std::list <MDI>> Visitados; // Para el DFS
		
		IndexPair ip;
		VectorVertex v; // Vertice Matcheado
		VectorEdge e;	
	};
} // Causalize
#endif
