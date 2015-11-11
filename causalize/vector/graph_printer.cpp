#include <causalize/vector/graph_definition.h>
#include <causalize/vector/graph_printer.h>

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

GraphPrinter::GraphPrinter(const CausalizationGraph &g): graph(g)
{
	CausalizationGraph::vertex_iterator vi, vi_end;
	for(tie(vi, vi_end) = vertices(graph); vi!= vi_end; vi++){
		if(graph[*vi].type == E){
			equationDescriptors.push_back(*vi);
		}else{
		unknownDescriptors.push_back(*vi);		
		}
	}
}

string
GraphPrinter::printGraph(string name){
	// stringstream stri;
	// ofstream out(name.c_str());
	// int depth = 0;

	// stri << "graph G{" << endl;
	// INSERT_TAB
	// 	MAKE_SPACE
	// 	stri << "subgraph cluster0{" << endl;
	// 	INSERT_TAB
	// 		MAKE_SPACE
	// 		stri << "label = \"Equations\";" << endl;
	// 		MAKE_SPACE
	// 		stri << "edge [style=invis];" << endl;
	// 		MAKE_SPACE
 //      stringstream colors2;
	// 		for(list<Vertex>::iterator it=equationDescriptors.begin(); it!=equationDescriptors.end(); it++){
	// 			list<Vertex>::iterator aux = it;
	// 			aux++;
	// 			stri << graph[*it].index;
	// 			if((aux) != equationDescriptors.end()){
	// 				stri << " -- ";		
	// 			}else{
	// 				stri << ";" << endl;		
	// 			}
 //        if (out_degree(*it,graph)==0)
 //          colors2 << "    " << graph[*it].index << "[ color=\"red\" ];" << endl;
	// 		}
 //      stri << colors2.str();
	// 	DELETE_TAB
	// 	MAKE_SPACE
	// 	stri << "}" << endl;
	// DELETE_TAB


	// INSERT_TAB
	// 	MAKE_SPACE
	// 	stri << "subgraph cluster1{" << endl;
	// 	INSERT_TAB
	// 		MAKE_SPACE
	// 		stri << "label = \"Unkowns\";" << endl;
	// 		MAKE_SPACE
	// 		stri << "edge [style=invis];" << endl;
	// 		MAKE_SPACE
 //      stringstream colors;
	// 		for(list<Vertex>::iterator it=unknownDescriptors.begin(); it!=unknownDescriptors.end(); it++){
	// 			list<Vertex>::iterator aux = it;
	// 			aux++;
 //        if (graph[*it].unknowns.size())
	// 			  stri << "\"" << graph[*it].unknowns.front() << "\"";
 //        else {
	// 			  if(graph[*it].isState){
	// 				  stri << "\"der(" << graph[*it].variableName << ")\"";
 //  				}else{
	//   				stri << "\"" << graph[*it].variableName;
 //            if (graph[*it].count)
	//   				  stri << "[" << graph[*it].count << "]";
 //            stri << "\"";
	// 	  		}
 //        }
	// 			if((aux) != unknownDescriptors.end()){
	// 				stri << " -- ";		
	// 			}else{
	// 				stri << ";" << endl;		
	// 			}
 //        //if (out_degree(*it,graph)==0)
 //          //colors << "    \"" << graph[*it].unknowns.front() << "\" [ color=\"red\" ];" << endl;
	// 		}
	// 	DELETE_TAB
	// 	MAKE_SPACE
	// 	stri << colors.str();
	// 	stri << "}" << endl;
	// DELETE_TAB

	// INSERT_TAB
	// 	MAKE_SPACE
	// 	stri << "edge [constraint=false];" << endl;
	// 	for(list<Vertex>::iterator  eq_it = equationDescriptors.begin(); eq_it != equationDescriptors.end(); eq_it++){
	// 		CausalizationGraph::out_edge_iterator ei, ei_end;
	// 		for(tie(ei, ei_end) = out_edges(*eq_it, graph); ei != ei_end; ei++){
	// 			Vertex unknown = target(*ei, graph);
	// 			MAKE_SPACE;
	// 			string name;
 //       if (graph[unknown].unknowns.size()) {
 //      	  stri << graph[*eq_it].index << " -- \"" << graph[unknown].unknowns.front() << "\"";
 //        } else {
	// 			  if(graph[unknown].isState){
	// 				  name = "der(" + graph[unknown].variableName + ")";		
	// 			  }else{
	// 			    name = graph[unknown].variableName;
 //            if (graph[unknown].count) {
	//   				  name += "[" ;//+ graph[*i/t].count + "]";
 //              name += boost::lexical_cast<std::string>(graph[unknown].count);
	//   				  name += "]" ;//+ graph[*i/t].count + "]";
 //            }
	
	// 			  }
 //      	  stri << graph[*eq_it].index << " -- \"" << name << "\"";
 //        }
	// 		stri << "[label = \"";
	// 		stri << (graph[*ei]).p_e;
	// 		stri << " <-> ";
	// 		stri << (graph[*ei]).p_v;
	// 		stri << "\"]";
	// 		if(graph[*ei].indexInterval.size() == 0){
	// 				stri << "[label = \"[1:1]\"]";
 //        	} else if(graph[*ei].indexInterval.size() == 1){
	// 				stri << "[label = \"[" << first(graph[*ei].indexInterval) << ":" << first(graph[*ei].indexInterval) << "]\"]";
	// 		} else if (graph[*ei].indexInterval.size() > 1){
	// 				interval_set<int>::iterator isi = graph[*ei].indexInterval.begin();
	// 				stri << " [label = \"";
 //          if (graph[*ei].genericIndex.first!=1)
 //            stri << graph[*ei].genericIndex.first << "*";
 //          stri << "i";
 //          if (graph[*ei].genericIndex.second!=0) 
 //          **  stri << "+" << graph[*ei].genericIndex.second;
          
          
	// 				stri << *isi << "\"]";
	// 			}
	// 			stri << ";" << endl;
	// 		}
	// 	}
	// DELETE_TAB
	// stri << "}" << endl;
	// out << stri.str();
	// out.close();
	// return stri.str();
}
