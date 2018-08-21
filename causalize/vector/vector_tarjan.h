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

#ifndef VECTOR_TARJAN_
#define VECTOR_TARJAN_

#include <causalize/vector/vector_graph_definition.h>
#include <causalize/vector/vector_matching.h>

namespace Causalize {
  struct TarjanVertexProperty: VertexProperty {
  /// @brief Represent the conexion into a Unknown and a Equation
    IndexPair ip;
		std::list <MDI> rest;
  };
	struct TarjanEdgeProperty {
		MDI dom;
		MDI ran;		
	};

	/// @brief This is the definition of the Incidence graph for the vector case.
	typedef boost::adjacency_list<boost::listS, boost::listS, boost::directedS, TarjanVertexProperty, TarjanEdgeProperty> VectorTarjanGraph;
	/// @brief This a node from the vectorized incidence graph
	typedef Causalize::VectorTarjanGraph::vertex_descriptor TarjanVertex;
	/// @brief This a node from the vectorized incidence graph
	typedef VectorCausalizationGraph::edge_descriptor VectorEdge;
	typedef std::pair <TarjanVertex, MDI> TarjanPart;
	typedef std::map <MDI, Match> MapMDI;



	class VectorTarjan{
		
	public:
		VectorTarjan(){ };
		VectorTarjan(VectorCausalizationGraph graph, std::map <VectorVertex, MapMDI> &Pair_E, std::map <VectorVertex, MapMDI> &Pair_U);

	private:

		std::map <TarjanPart, int> lowlinks;
		std::list <std::list <TarjanPart> > strongly_connected_component;
		VectorTarjanGraph	tgraph;
		VectorCausalizationGraph graph;
	};
}; // Causalize
#endif
