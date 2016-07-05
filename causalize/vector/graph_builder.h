/*
* This class provides the interface to build the causalization
* graph which is then going to be processed by the causalization
* algorithm. Since there may be more than one way of building it
* we'll have a base abstract class and (maybe) several concrete
* implementations.
*/
#include <mmo/mmo_class.h>
#include <causalize/vector/vector_graph_definition.h>
#include <causalize/state_variables_finder.h>


namespace Causalize {
class ReducedGraphBuilder {
public:
		ReducedGraphBuilder(MMO_Class &mmo_cl);
		~ReducedGraphBuilder(){};
		virtual VectorCausalizationGraph makeGraph();
private:
		int getForRangeSize(Modelica::AST::ForEq);
		list<Causalize::VectorEquationVertex> equationDescriptorList;
		list<Causalize::VectorUnknownVertex> unknownDescriptorList;
		StateVariablesFinder state_finder;	
		MMO_Class &mmo_class;
		Causalize::VectorCausalizationGraph graph;
};

}
