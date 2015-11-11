#ifndef GRAPH_DEFINITION_
#define GRAPH_DEFINITION_

#undef member
#include <utility>
#include <boost/config.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/icl/interval_set.hpp>

#include <ast/equation.h>
#include <mmo/mmo_class.h>



/* E: equations, U: unknowns*/
enum VertexType{E, U};
enum EquationType {EQEQUALITY, EQFOR};

/*we use Bundled properties*/
struct VertexProperties{
	VertexType type;
	Modelica::AST::Equation equation;
	EquationType eqType;
	std::string variableName;
	bool isState;
	int count; //size of the array or number of equations
	int  index; //for debug purposes
	bool visited;
	//these fields are for compatibility with the previous
	//algorithm
	Modelica::AST::Equation  eq;
	Modelica::AST::Expression unknown;
};

/*
* genericIndex: if the edge represents an occurrence of the form 
* a[i-1] in some equation, then this list contains the expression: i-1.
* indexes: it contains the number of indexes of a variable (in case it is
* an array) that are used in the equation connected by the edge.
*/
struct EdgeProperties{
	boost::icl::interval_set<int> p_e;
	boost::icl::interval_set<int> p_v;
	std::pair<int, int> genericIndex;
	boost::icl::interval_set<int> indexInterval;
  bool operator<  (const EdgeProperties & rhs) const {
    if (p_e <rhs.p_e) return true;
    if (p_v < rhs.p_v ) return true; 
    if (genericIndex<rhs.genericIndex) return true;
    if (indexInterval < rhs.indexInterval) return true; 
    return false;
  }
  bool isBalanced() {
    return p_e.size()==p_v.size();
  }
};

typedef boost::adjacency_list<boost::listS, boost::listS,
		boost::undirectedS, VertexProperties, EdgeProperties> CausalizationGraph;
typedef CausalizationGraph::vertex_descriptor Vertex;
typedef CausalizationGraph::edge_descriptor Edge;

struct CausalizedVar{
	VertexProperties unknown;
	VertexProperties equation;
	EdgeProperties edge;
};

/*Classes for a potential redefinition of the Bundled properties*/

class GenericVertex{
	public:
		GenericVertex(VertexType t);
		VertexType type();
		void setCount(int c);
		int count();
		/*FALTA HACE LOS CASTEOS DINAMICOS*/
	protected:
		VertexType _type;
		int _count;
};

class EquationVertex: public GenericVertex{
	public:
		EquationVertex(Modelica::AST::Equation equation);
		//EquationType equationType();
	private:
		Modelica::AST::Equation equation;
};

class UnknownVertex : public GenericVertex{
	public:
		UnknownVertex(std::string variableName);
		void setState();
		bool isState();
	private:
		bool _isState;
};

#endif
