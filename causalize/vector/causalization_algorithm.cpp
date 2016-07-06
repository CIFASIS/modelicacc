#include <causalize/vector/causalization_algorithm.h>
#include <causalize/vector/vector_graph_definition.h>
//#include <causalize/vector/graph_printer.h>
#include <util/debug.h>
#include <util/solve/solve.h>

#include <boost/graph/adjacency_list.hpp>
#include <boost/variant/get.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/icl/discrete_interval.hpp>
#include <sstream>
#include <mmo/mmo_class.h>

#define sz(a) int((a).size())

using namespace Modelica;
using namespace std;
using namespace boost;
using namespace boost::icl;

namespace Causalize {
CausalizationStrategyVector::CausalizationStrategyVector(VectorCausalizationGraph g, MMO_Class &m): mmo(m){
	graph = g;
	VectorCausalizationGraph::vertex_iterator vi, vi_end;
	equationNumber = unknownNumber = 0;
	for(boost::tie(vi, vi_end) = vertices(graph); vi != vi_end; vi++){
		VectorVertex current_element = *vi;
		if(graph[current_element].type == E){
      // TODO:
			//equationNumber += graph[current_element].count;
			equationDescriptors.push_back(current_element);
		}
		else{
      // TODO: 
			//unknownNumber += ( graph[current_element].count == 0 ) ? 1 : graph[current_element].count;
			unknownDescriptors.push_back(current_element);
		}
	}
	DEBUG('c', "Number of equations %d\n"
	           "Number of unknowns %d\n", 
			   equationNumber, unknownNumber);
	
	if(equationNumber != unknownNumber){
		ERROR("The model being causalized is not balanced.\n"
			  "There are %d equations and %d variables\n", 
			  equationNumber, unknownNumber);		
	}
}

  /*
int  
CausalizationStrategyVector::test_intersection(const Edge &e1, const Edge &e2){
  //std::cout << "Comparing: " << graph[e1].indexInterval << " with " << graph[e2].indexInterval << "\n";
  if (graph[e1].genericIndex.first==1 && graph[e2].genericIndex.first==1 &&
      graph[e1].genericIndex.second==0 && graph[e2].genericIndex.second == 0) {
    if (graph[e1].indexInterval == graph[e2].indexInterval) {
      return 2;
    }
	  return (intersects(graph[e1].indexInterval, graph[e2].indexInterval) ? 1 : 0);
  }
  if (graph[e1].genericIndex.first==1 && graph[e2].genericIndex.first==1) { // i+b1 with i+b2
	  return intersects(
     discrete_interval<int>::closed(first(graph[e1].indexInterval)+graph[e1].genericIndex.second,last(graph[e1].indexInterval)+graph[e1].genericIndex.second),
     discrete_interval<int>::closed(first(graph[e2].indexInterval)+graph[e2].genericIndex.second,last(graph[e2].indexInterval)+graph[e2].genericIndex.second));
  }
  ERROR("Intersection of index with multiplier not supported yet");
	if(graph[e1].genericIndex.first > 1 || graph[e2].genericIndex.first > 1){
		//we transform the first and the last element of each interval
		//we represent e1 interval as [a,b] and e2 interval as [c,d]
		int a = first(graph[e1].indexInterval), b = last(graph[e1].indexInterval);
		int c = first(graph[e2].indexInterval), d = last(graph[e2].indexInterval);

		DEBUG('g', "Testing intersection between: [%d, %d](%d, %d), [%d, %d](%d,%d)\n", a,b,graph[e1].genericIndex.first,
		   graph[e1].genericIndex.second,c,d, graph[e2].genericIndex.first, graph[e2].genericIndex.second);
		
		if(graph[e1].genericIndex.first > 1){
			a = graph[e1].genericIndex.first * a + graph[e1].genericIndex.second;		
			b = graph[e1].genericIndex.first * b + graph[e1].genericIndex.second;
		}
		if(graph[e2].genericIndex.first > 1){
			c = graph[e2].genericIndex.first * c + graph[e2].genericIndex.second;
			d = graph[e2].genericIndex.first * d + graph[e2].genericIndex.second;		
		}
		//we check if we have an interval intersection between
		//[a,b] and [c,d]
		if(c == a || c == b	|| d == a || d == b){
			//we have a match in some extreme!
			return true;
		}
		else if((c < a && d > a) || (c > a && b > c)){
			if(d < b){
				int d_ = d - graph[e1].genericIndex.second; 
				if(d_ % graph[e1].genericIndex.first == 0){
					return true;
				}
			}else{
				int b_ = b - graph[e2].genericIndex.second;
				if(b_ % graph[e2].genericIndex.first == 0){
					return true;
				}
			}
		}
		return false;
	}
  abort();
}
void
CausalizationStrategyVector::remove_edge_from_array(Vertex unknown, Edge targetEdge){
	assert(boost::icl::size(graph[targetEdge].indexInterval) != 0);
	DEBUG('g', "Removing edge for unknown: %s\n", graph[unknown].variableName.c_str());
	CausalizationGraph::out_edge_iterator it, end, auxiliaryIter;
	tie(auxiliaryIter, end) = out_edges(unknown, graph);
	for(it = auxiliaryIter; it != end; it = auxiliaryIter){
		auxiliaryIter++;
		if(*it == targetEdge){continue;}
		if(test_intersection(targetEdge, *it)==2){
			remove_edge(*it, graph);
		}
	}
	remove_edge(targetEdge, graph);
}
  */

void 
CausalizationStrategyVector::causalize1toN(const VectorVertex &u, const VectorVertex &eq, const VectorEdge &e){
	CausalizedVar c_var;
	c_var.unknown = graph[u];
	c_var.equation = graph[eq];
	c_var.edge = graph[e];
  //std::cerr << "*******\n Causalizing1toN " << c_var.unknown.variableName << " in range " << graph[e].p_v <<  " with eq \n" << graph[eq].equation << "\n";
	equations1toN.push_back(c_var);
}

void 
CausalizationStrategyVector::causalizeNto1(const VectorVertex &u, const VectorVertex &eq, const VectorEdge &e){
	CausalizedVar c_var;
	c_var.unknown = graph[u];
	c_var.equation = graph[eq];
	c_var.edge = graph[e];
  //std::cerr << "Causalizing " << c_var.unknown.variableName << " with eq " << graph[eq].equation << "\n";
	equationsNto1.insert(begin(equationsNto1), c_var);		
}

bool
CausalizationStrategyVector::causalize(){	
	assert(equationNumber == unknownNumber);
	if(equationDescriptors.empty()) { 
    // Finished causalizing :)
    EquationList all;
    vector<CausalizedVar> sorted_vars = equations1toN;
    sorted_vars.insert(sorted_vars.end(),equationsNto1.begin(), equationsNto1.end());
  	foreach_(CausalizedVar cv, sorted_vars) {
	  	string name;
      Equation e = mmo.equations_ref().equations_ref().at(cv.equation.index);
		  if(is<ForEq>(e)) {
	  	  //name = getName(cv.unknown, cv.edge);
        if (is<ForEq>(e)) {
          ForEq &feq = get<ForEq>(e);
          VarSymbolTable syms = mmo.syms_ref();
          VarInfo vinfo(TypePrefixes(),"Integer");
          syms.insert(feq.range().indexes().front().name(),vinfo);
          Expression index = Reference(feq.range().indexes().front().name());
          if (cv.edge.p_v != cv.edge.p_e) {
            if (cv.edge.p_e.begin()->lower()<cv.edge.p_v.begin()->lower()) 
              index = BinOp(index,Add,1);
            else
              index = BinOp(index,Sub,1);
            //std::cerr <<  "Eq range " << (cv.edge.p_e) << " var range " << cv.edge.p_v << "\n";
          } else {
            //std::cerr <<  "Eq range " << (cv.edge.p_e) << " var range " << cv.edge.p_v << "\n";
          }
          Expression var = (is<Call>(cv.unknown.unknowns.front()) ?
                            Call("der",ExpList(1,Reference(Ref(1,RefTuple(get<0>(get<Reference>(get<Call>(cv.unknown.unknowns.front()).args().front()).ref().front()),ExpList(1,index))))))
                            : Expression(0));
            //std::cerr <<  "Solving variable " << var << "\n";
            //std::cerr <<  "with eq " << feq << "\n";
           std::list<std::string> c_code;
           ClassList cl;
           if (debugIsEnabled('c')) 
             std::cerr << "Solving:\n" << e << "\nfor variable " << var << "\n";
           EquationList el = EquationSolver::solve(EquationList(1,feq),ExpList(1,var), syms,c_code,cl);
           e = el.front();
           all.push_back(e);
        } else {
          ERROR("Trying to solve an array variable with a non for equation");
        }
		  } else{
	  	  Expression ref;
	      if(cv.unknown.count == 0)
		      ref = cv.unknown.unknowns.front();
	      else if (cv.edge.p_v.size()==1) {
          Expression var = cv.unknown.unknowns.front();
          if (is<Call>(var) && get<Call>(var).name()=="der") {
            Call &call = get<Call>(var);
            Expression &arg = call.args_ref().front(); 
            ERROR_UNLESS(is<Reference>(arg),"Argument to der must be a reference");
            //get<1>(get<Reference>(arg).ref_ref().front()) = cv.edge.p_v.begin()->lower();
            Reference &ref_arg = get<Reference>(arg);
            get<1>(ref_arg.ref_ref().front())=  ExpList(1,cv.edge.p_v.begin()->lower());
		        ref = var;
          } else if (is<Reference>(var)) {
            ERROR("Unkonwn of wrong type");
          } else {
            ERROR("Unkonwn of wrong type");
          }
        } else 
          ERROR("Trying to solve a single variable with no fixed index");
        std::list<std::string> c_code;
        ClassList cl;
        if (debugIsEnabled('c')) 
          std::cerr << "Solving\n" << e << "\nfor variable " << name << "\n";
        EquationList el = EquationSolver::solve(EquationList(1,e),ExpList(1,ref), mmo.syms_ref(),c_code, cl);
        all.insert(all.end(),el.begin(),el.end());
		  } 
	  } 		
    mmo.equations_ref().equations_ref()=all;
	
    return true;
  }
	
	list<VectorVertex>::size_type numAcausalEqs = equationDescriptors.size();
	list<VectorVertex>::iterator iter, auxiliaryIter;
	auxiliaryIter = equationDescriptors.begin();
	for(iter = auxiliaryIter; iter != equationDescriptors.end(); iter = auxiliaryIter){
		auxiliaryIter++;
		VectorVertex eq = *iter;
		ERROR_UNLESS(out_degree(eq, graph) != 0, "Problem is singular, not supported yet1\n");
	  if(out_degree(eq,graph) == 1) {
      VectorEdge e = *out_edges(eq, graph).first;
			VectorVertex unknown = target(e,graph);
	    equationNumber--;
		  unknownNumber--;
      std::list<VectorEdge> remove;
		  causalize1toN(unknown, eq, e);
			remove_edge(e, graph);
      foreach_(VectorEdge e1, out_edges(unknown,graph)) {
        graph[e1].p_v -= graph[e].p_v;
        if (graph[e1].isBalanced()) {
          // TODO: I have to update the p_e propertie as well
          WARNING("Have to update p_e also\n");
        }
        if (graph[e1].p_v.size()==0)  // Remove empty edges
		      remove.push_back(e1);
      }
      foreach_(VectorEdge e1, remove) 
			  remove_edge(e1, graph);
		  equationDescriptors.erase(iter);
			unknownDescriptors.remove(unknown);
    } else {
      // Try the intersection
    }
    /*int static step=0;
    GraphPrinter gp(graph);
    char buff[128];
    sprintf(buff,"graph_%d.dot",step++);
    gp.printGraph(buff);
    */
	}
	
	//now we process the unknowns' side
	auxiliaryIter = unknownDescriptors.begin();
	for(iter = auxiliaryIter; iter != unknownDescriptors.end(); iter = auxiliaryIter){
		auxiliaryIter++;
		VectorVertex unknown = *iter;
		ERROR_UNLESS(out_degree(unknown, graph) != 0, "Problem is singular, not supported yet1\n");
		if(out_degree(unknown, graph) == 1){
      VectorEdge e = *out_edges(unknown, graph).first;
			VectorVertex eq = target(e,graph);
	    equationNumber--;
		  unknownNumber--;
      std::list<VectorEdge> remove;
		  causalizeNto1(unknown, eq, e);
			remove_edge(e, graph);
      foreach_(VectorEdge e1, out_edges(eq,graph)) {
        graph[e1].p_e -= graph[e].p_e;
        if (graph[e1].isBalanced()) {
          // TODO: I have to update the p_e propertie as well
          WARNING("Have to update p_v also\n");
        }
        if (graph[e1].p_e.size()==0)  // Remove empty edges
		      remove.push_back(e1);
      }
      foreach_(VectorEdge e1, remove) 
			  remove_edge(e1, graph);
		  equationDescriptors.remove(eq);
			unknownDescriptors.erase(iter);
    } else {
      // Try the intersection
    }
	}
	if(numAcausalEqs == equationDescriptors.size()){
		//we have a LOOP or a FOR equation that we don't 
		//handle at least yet, so we resort to the previous
		//algorithm
		WARNING ("Loop detected! We don't handle loops yet!\n");
		return false;
	}
	return causalize();
}

string 
CausalizationStrategyVector::getName(const VectorVertexProperties &uk, const VectorEdgeProperties &ed){
	stringstream name;

	if(uk.count == 0){
		name << uk.unknowns.front();
	} else if (ed.p_v.size()==1) {
		name << uk.unknowns.front() << "[" << ed.p_v.begin()->lower() << "]";		
  }
  /*else if(ed.indexInterval.size() == 1){
		name << uk.variableName << "[" << ed.genericIndex.first * first(ed.indexInterval) +  ed.genericIndex.second << "]";
	}else{
		name << uk.variableName << "[";
    if (ed.genericIndex.first!=1) 
      name << ed.genericIndex.first << "*";
    name << "i";
    if (ed.genericIndex.second!=0)
      name << "+" << ed.genericIndex.second;
    name << "]";		
	}
  */

  //TODO:
	/*if(uk.isState)
		return "der(" + name.str() + ")";
  */

	return name.str();	
}

void
CausalizationStrategyVector::print(){
  vector<CausalizedVar> sorted_vars = equations1toN;
  sorted_vars.insert(sorted_vars.end(),equationsNto1.begin(), equationsNto1.end());
	/*foreach_(CausalizedVar cvar,sorted_vars) { 
		string name;
		name = getName(cvar.unknown, cvar.edge);
		if(cvar.edge.indexInterval.size() > 1){
			interval_set<int>::iterator isi = cvar.edge.indexInterval.begin();
			cout << "(" << name <<	"," << *isi << "," << cvar.equation.index << ")" << endl; 
		}else{
			cout << "(" << name << ", " << cvar.equation.index << ")" << endl;		
		}
	}
  */
}
}
