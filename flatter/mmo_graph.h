#include <boost/config.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <ast/expression.h>
#include <util/table.h>

#ifndef MMOGRAPH_DEFINITION_
#define MMOGRAPH_DEFINITION_

template<typename T>
std::vector<T> toVector(std::set<T> v) 
{
	std::vector<T> output;
	std::copy(v.begin(), v.end(), std::back_inserter(output));
	return output;
}

using namespace Modelica;
using namespace Modelica::AST;

struct VertexProperties {
public :
    OptExp name; 
    bool hasIndex;
    OptExp range;
    OptExpList free;
};

struct EdgeProperties {
public :
    OptExp index;   
    bool visited;
};

typedef boost::adjacency_list<boost::vecS,
        boost::vecS, boost::undirectedS, VertexProperties, EdgeProperties> MMOGraph;

typedef MMOGraph::vertex_descriptor Vertex;
typedef MMOGraph::edge_descriptor Edge;
typedef Option<Vertex>	OptVertex;
typedef Option<Edge>	OptEdge;
typedef boost::graph_traits<MMOGraph>::vertex_iterator  MMOGraphVertexIteretor;
typedef boost::graph_traits<MMOGraph>::edge_iterator  MMOGraphEdgeIteretor;
typedef boost::graph_traits < MMOGraph >::out_edge_iterator MMOGraphOutputEdgeIteretor;



#endif
