#include <causalize/vector/vector_graph_definition.h>
#include <boost/icl/interval_set.hpp>
#include <mmo/mmo_class.h>

using namespace boost::icl;

namespace Causalize {
class CausalizationStrategyVector{
	public:
		CausalizationStrategyVector(Causalize::VectorCausalizationGraph g, Modelica::MMO_Class &m);			
		bool causalize();
		Modelica::AST::EquationList getEquations();
		void print();
	private:
		//void remove_edge_from_array(Vertex, Edge);
    int step;
		Causalize::VectorCausalizationGraph graph;
		int equationNumber;
		int unknownNumber;
		std::list<Causalize::VectorVertex> equationDescriptors, unknownDescriptors;
		std::vector<Causalize::CausalizedVar> equations1toN;
		std::vector<Causalize::CausalizedVar> equationsNto1;
		void causalize1toN(const Causalize::VectorVertex &unknown, const Causalize::VectorVertex &equation, const Causalize::VectorEdge &e);
		void causalizeNto1(const Causalize::VectorVertex &unknown, const Causalize::VectorVertex &equation, const Causalize::VectorEdge &e);
		//int test_intersection(const Edge&, const Edge&);
		std::string getName(const Causalize::VectorVertexProperty&, const Causalize::VectorEdgeProperty&);
    Modelica::MMO_Class &mmo;
    Option<std::pair<VectorEdge,IndexPairSet> > CanCausalizeEquation(VectorEquationVertex eq);
    Option<std::pair<VectorEdge,IndexPairSet> > CanCausalizeUnknown(VectorUnknownVertex eq);
};
}
