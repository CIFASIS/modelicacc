#include <causalize/vector/graph_definition.h>
#include <boost/icl/interval_set.hpp>
#include <mmo/mmo_class.h>

using namespace boost::icl;

class CausalizationStrategyVector{
	public:
		CausalizationStrategyVector(CausalizationGraph g, Modelica::MMO_Class &m);			
		bool causalize();
		Modelica::AST::EquationList getEquations();
		void print();
	private:
		//void remove_edge_from_array(Vertex, Edge);
		CausalizationGraph graph;
		int equationNumber;
		int unknownNumber;
		std::list<Vertex> equationDescriptors, unknownDescriptors;
		std::vector<CausalizedVar> equations1toN;
		std::vector<CausalizedVar> equationsNto1;
		void causalize1toN(const Vertex &unknown, const Vertex &equation, const Edge &e);
		void causalizeNto1(const Vertex &unknown, const Vertex &equation, const Edge &e);
		//int test_intersection(const Edge&, const Edge&);
		std::string getName(const VertexProperties&, const EdgeProperties&);
    Modelica::MMO_Class &mmo;
};
