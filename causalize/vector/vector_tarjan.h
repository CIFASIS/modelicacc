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
#include <stack>

namespace Causalize {
  struct TarjanVertexProperty: VectorVertexProperty {
  /// @brief Represent the conexion into a Unknown and a Equation
    IndexPair ip; // La forma de conexión
	MDI mdi; // Equation
	int number;
	VectorEdge edge;
	};
	struct TarjanEdgeProperty: Label {
		MDI dom;
		MDI ran;
		IndexPair ip;		
	};

	/// @brief This is the definition of the Incidence graph for the vector case.
	typedef boost::adjacency_list<boost::listS, boost::listS, boost::directedS, TarjanVertexProperty, Label> TarjanGraph;
	/// @brief This a node from the vectorized incidence graph
	typedef Causalize::TarjanGraph::vertex_descriptor TarjanVertex;
	/// @brief This a node from the vectorized incidence graph
	typedef Causalize::TarjanGraph::edge_descriptor TarjanEdge;
	struct TarjanData{
		int id;
		int low;
		bool onStack;
	};

	typedef std::list <CausalizedVar> CausalizeEquations;
	typedef std::pair <TarjanVertex, MDI> VertexPart;
	typedef std::list < VertexPart > ConnectedComponent;
	typedef std::map <MDI, Match> MapMDI;
	

	class VectorTarjan{
	public:
		VectorTarjan(){ };
		VectorTarjan(VectorCausalizationGraph graph, std::map <VectorVertex, MapMDI> Pair_E, std::map <VectorVertex, MapMDI> Pair_U);
		std::list <CausalizeEquations> GetConnectedComponent();
		TarjanGraph	tgraph;

	private:
		void DFS(TarjanVertex tv, MDI mdi);
		int id;
		MDIL find (TarjanVertex tv, MDI mdi, bool onlyNV = false);

		std::stack <VertexPart> stack;
		std::list <ConnectedComponent> strongly_connected_component;
		std::map <VertexPart, TarjanData> data;
		VectorCausalizationGraph graph;
	};
}; // Causalize
#endif
