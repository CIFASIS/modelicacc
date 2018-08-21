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

namespace Causalize{

	
	VectorTarjan::VectorTarjan(VectorCausalizationGraph graph, std::map <VectorVertex, MapMDI> &Pair_E, std::map <VectorVertex, MapMDI> &Pair_U) : graph(graph){
			for (auto p : Pair_E){
				VectorVertex v = p.first;
				for (auto mm : p.second){
					TarjanVertex tv = add_vertex (tgraph);
					tgraph[tv].equation = graph[v].equation;
					tgraph[tv].unknown = graph[mm.second.v].unknown();
					tgraph[tv].ip = IndexPair (mm.first, domToRan (mm.first, mm.second.ip), mm.second.ip.GetOffset(), mm.second.ip.GetUsage());
					tgraph[tv].rest = std::list <MDI> ();
					tgraph[tv].rest.push_back (mm.first);
				}				
			}
			
	};
} // Causalize

