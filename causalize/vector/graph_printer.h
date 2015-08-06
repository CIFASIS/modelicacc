#include <causalize/vector/graph_definition.h>
//#include <ostream>

class GraphPrinter{
	public:
		GraphPrinter(const CausalizationGraph &g);
		std::string printGraph(std::string);
	private:
		const CausalizationGraph &graph;
		std::list<Vertex> equationDescriptors;
		std::list<Vertex> unknownDescriptors;
		//ofstream outFile;

};
