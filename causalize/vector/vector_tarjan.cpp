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

	
	VectorTarjan::VectorTarjan(VectorCausalizationGraph graph, std::map <VectorVertex, MapMDI> Pair_E, std::map <VectorVertex, MapMDI> Pair_U) : graph(graph){
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
				}				
			}
			TarjanGraph::vertex_iterator vi, vi_end; int counter = 1;
			for(boost::tie(vi, vi_end) = vertices(tgraph); vi != vi_end; vi++){
				dprint(counter);counter++;
				dprint(tgraph[*vi].equation);
				dprint(tgraph[*vi].unknown());
				dprint(tgraph[*vi].ip.Dom());
				dprint(tgraph[*vi].ip.Ran());
			}
			strongly_connected_component = std::list <std::list <TarjanPart> > ();
			lowlinks = std::map <TarjanPart, int> ();
	};
} // Causalize

