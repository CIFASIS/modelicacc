/*
* This class provides the interface to build the causalization
* graph which is then going to be processed by the causalization
* algorithm. Since there may be more than one way of building it
* we'll have a base abstract class and (maybe) several concrete
* implementations.
*/
#include <mmo/mmo_class.h>
#include <causalize/vector/graph_definition.h>
#include <causalize/state_variables_finder.h>


class GraphBuilder {
public: 
		GraphBuilder(MMO_Class &mmo_cl):mmo_class(mmo_cl){};
		~GraphBuilder(){};
		virtual CausalizationGraph makeGraph() = 0;
protected:
		MMO_Class &mmo_class;
		CausalizationGraph graph;
};

class ReducedGraphBuilder: public GraphBuilder{
public:
		ReducedGraphBuilder(MMO_Class &mmo_cl);
		~ReducedGraphBuilder(){};
		virtual CausalizationGraph makeGraph();
private:
		int getForRangeSize(Modelica::AST::ForEq);
		list<Vertex> equationDescriptorList;
		list<Vertex> unknownDescriptorList;
		StateVariablesFinder state_finder;	
};

