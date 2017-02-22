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

#include <causalize/vector/vector_graph_definition.h>
#include <mmo/mmo_class.h>

namespace Causalize {
  class ComputeStructure {
  	public:
	  	ComputeStructure (Causalize::VectorCausalizationGraph g, Modelica::MMO_Class &m);			
		  void Compute();
      MDI GetCandidate(VectorVertex u);
  	private:
	  	Causalize::VectorCausalizationGraph graph;
      bool TestCandidate(VectorVertex, MDI);
      Vertex GetEquation(Edge e);
      Vertex GetUnknown(Edge e);
		  std::list<Causalize::VectorVertex> equationDescriptors, unknownDescriptors;
      Modelica::MMO_Class &mmo;
  };
}
