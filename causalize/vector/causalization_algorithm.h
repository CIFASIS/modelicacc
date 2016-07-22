#include <causalize/vector/vector_graph_definition.h>
#include <boost/icl/interval_set.hpp>
#include <mmo/mmo_class.h>

using namespace boost::icl;

namespace Causalize {
class CausalizationStrategyVector{
	public:
		CausalizationStrategyVector(Causalize::VectorCausalizationGraph g, Modelica::MMO_Class &m);			
		bool Causalize();
		Modelica::AST::EquationList GetEquations();
		void PrintCausalizationResult();
		void SolveEquations();
	private:
		//void remove_edge_from_array(Vertex, Edge);
    int step;
		Causalize::VectorCausalizationGraph graph;
		int equationNumber;
		int unknownNumber;
		std::list<Causalize::VectorVertex> equationDescriptors, unknownDescriptors;
		std::vector<Causalize::CausalizedVar> equations1toN;
		std::vector<Causalize::CausalizedVar> equationsNto1;
//		void causalize1toN(const Causalize::VectorVertex &unknown, const Causalize::VectorVertex &equation, const IndexPairSet &ips);
		void Causalize1toN(const Unknown unknown, const Equation equation, const IndexPairSet ips);
//		void causalizeNto1(const Causalize::VectorVertex &unknown, const Causalize::VectorVertex &equation, const IndexPairSet &ips);
		void CausalizeNto1(const Unknown unknown, const Equation equation, const IndexPairSet ips);
		//int test_intersection(const Edge&, const Edge&);
    Modelica::MMO_Class &mmo;
    Option<std::pair<VectorEdge,IndexPairSet> > CanCausalizeEquation(VectorEquationVertex eq);
    Option<std::pair<VectorEdge,IndexPairSet> > CanCausalizeUnknown(VectorUnknownVertex eq);
    Vertex GetEquation(Edge e);
    Vertex GetUnknown(Edge e);
};
}
