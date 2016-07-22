#include <causalize/graph/graph_definition.h>
#include <list>
#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/icl/interval_set.hpp>
#include <boost/lexical_cast.hpp>


using namespace boost;
using namespace std;
using namespace boost::icl;
#define MAKE_SPACE for(int __i=0; __i<depth; __i++) stri << " ";
#define TAB_SPACE 2
#define INSERT_TAB depth += TAB_SPACE;
#define DELETE_TAB depth -= TAB_SPACE;

namespace Causalize {
  template <class VertexProperty, class EdgeProperty> 
  class GraphPrinter{

    typedef boost::adjacency_list<boost::listS, boost::listS, boost::undirectedS, VertexProperty, EdgeProperty> Graph;
    typedef typename boost::adjacency_list<boost::listS, boost::listS, boost::undirectedS, VertexProperty, EdgeProperty>::vertex_descriptor Vertex;
    typedef typename boost::adjacency_list<boost::listS, boost::listS, boost::undirectedS, VertexProperty, EdgeProperty>::out_edge_iterator EdgeIterator;

	  public:
		  GraphPrinter(const Graph &g): graph(g) {
      	typename Graph::vertex_iterator vi, vi_end;
	      for(tie(vi, vi_end) = vertices(graph); vi!= vi_end; vi++){
		      if(graph[*vi].type == E){
      			equationDescriptors.push_back(*vi);
      		}else{
      		  unknownDescriptors.push_back(*vi);		
    		  }
    	  }
      };

		  std::string printGraph(std::string name) {
	      stringstream stri;
	      ofstream out(name.c_str());
	      int depth = 0;
        typedef typename list<Vertex>::iterator Iterator;
      
	      stri << "graph G{" << endl;
	      INSERT_TAB
		      MAKE_SPACE
		      stri << "subgraph cluster0{" << endl;
		      INSERT_TAB
			      MAKE_SPACE
			      stri << "label = \"Equations\";" << endl;
			      MAKE_SPACE
			      stri << "edge [style=invis];" << endl;
			      MAKE_SPACE
            stringstream colors2;
			      for(Iterator it=equationDescriptors.begin(); it!=equationDescriptors.end(); it++){
				      Iterator aux = it;
				      aux++;
				      stri << graph[*it].index;
				      if((aux) != equationDescriptors.end()){
					      stri << " -- ";		
				      }else{
					      stri << ";" << endl;		
				      }
              if (out_degree(*it,graph)==0)
                colors2 << "    " << graph[*it].index << "[ color=\"red\" ];" << endl;
			      }
            stri << colors2.str();
		      DELETE_TAB
		      MAKE_SPACE
		      stri << "}" << endl;
	      DELETE_TAB
      
      
	      INSERT_TAB
		      MAKE_SPACE
		      stri << "subgraph cluster1{" << endl;
		      INSERT_TAB
			      MAKE_SPACE
			      stri << "label = \"Unkowns\";" << endl;
			      MAKE_SPACE
			      stri << "edge [style=invis];" << endl;
			      MAKE_SPACE
            stringstream colors;
			      for(Iterator it=unknownDescriptors.begin(); it!=unknownDescriptors.end(); it++){
				      Iterator aux = it;
				      aux++;
				      stri << "\"" << graph[*it].unknowns.front() << "\"";
				      if((aux) != unknownDescriptors.end()){
					      stri << " -- ";		
				      }else{
					      stri << ";" << endl;		
				      }
			      }
		      DELETE_TAB
		      MAKE_SPACE
		      stri << colors.str();
		      stri << "}" << endl;
	      DELETE_TAB
      
	      INSERT_TAB
		      MAKE_SPACE
		      stri << "edge [constraint=false];" << endl;
		      for(Iterator eq_it = equationDescriptors.begin(); eq_it != equationDescriptors.end(); eq_it++){
			      EdgeIterator ei, ei_end;
			      for(tie(ei, ei_end) = out_edges(*eq_it, graph); ei != ei_end; ei++){
				      Vertex unknown = target(*ei, graph);
				      MAKE_SPACE;
				      string name;
    	        stri << graph[*eq_it].index << " -- \"" << graph[unknown].unknowns.front() << "\"";
              EdgeProperty ep = graph[*ei];
              stri << "[label = \"" << ep << "\"];";
			      }
		      }
	      DELETE_TAB
	      stri << "}" << endl;
	      out << stri.str();
	      out.close();
	      return stri.str();
      }
	  private:
		  const Graph &graph;
		  std::list<Vertex> equationDescriptors;
		  std::list<Vertex> unknownDescriptors;
  };
}
