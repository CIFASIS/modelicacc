/*****************************************************************************

    This file is part of Modelica C Compiler.

    Modelica C Compiler is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Modelica C Compiler is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Modelica C Compiler.  If not, see <http://www.gnu.org/licenses/>.

******************************************************************************/

#define HAS_COUNT
#include <causalize/vector/causalization_algorithm.h>
#include <causalize/vector/vector_graph_definition.h>
#include <causalize/vector/vector_matching.h>
#include <causalize/vector/vector_tarjan.h>
#include <causalize/graph/graph_printer.h>

#include <util/debug.h>
#include <util/solve/solve.h>

#include <boost/tuple/tuple.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/variant/get.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/icl/discrete_interval.hpp>
#include <boost/icl/interval_set.hpp>

#include <mmo/mmo_class.h>
#include <ast/queries.h>

#include <sstream>
#define dprint(v) std::cout << #v"=" << v << std::endl //;)

#define sz(a) int((a).size())

using namespace Modelica;
using namespace std;
using namespace boost;
using namespace boost::icl;

extern bool solve;
namespace Causalize {
CausalizationStrategyVector::CausalizationStrategyVector(VectorCausalizationGraph g, MMO_Class &m): mmo(m){
	graph = g;
  step = 0;
	VectorCausalizationGraph::vertex_iterator vi, vi_end;
	equationNumber = unknownNumber = 0;
	for(boost::tie(vi, vi_end) = vertices(graph); vi != vi_end; vi++){
		VectorVertex current_element = *vi;
		if(graph[current_element].type == kVertexEquation){
      equationNumber += graph[current_element].count;
			equationDescriptors.push_back(current_element);
		}
		else{
      unknownNumber += graph[current_element].count;
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


void 
CausalizationStrategyVector::Causalize1toN(const VectorUnknown unk, const Equation eq, const IndexPairSet ips){
	CausalizedVar c_var;
	c_var.unknown = unk;
	c_var.equation = eq;
	c_var.pairs = ips;
	equations1toN.push_back(c_var);
}

void 
CausalizationStrategyVector::CausalizeNto1(const VectorUnknown unk, const Equation eq, const IndexPairSet ips){
	CausalizedVar c_var;
	c_var.unknown = unk;
	c_var.equation = eq;
	c_var.pairs = ips;
  equationsNto1.insert(equationsNto1.begin(), c_var);
}

bool
CausalizationStrategyVector::Causalize() {	
  int steps = 0;
  bool split = false;
	VectorMatching m(graph, equationDescriptors, unknownDescriptors);
	m.dfs_matching();
	VectorTarjan t(graph, m.getPairE(), m.getPairU());
	std::list <ConnectedComponent> scc = t.GetConnectedComponent();
	for (auto cc : scc){
		dprint("New");
		for (auto vp:cc){
			dprint(vp.first.equation);
			dprint(vp.first.unknown());
			dprint(vp.second);
		}
	}

	//~ return false;
  while(true) {
    bool causalize_some=false;
    assert(equationNumber == unknownNumber);
    if(equationDescriptors.empty() && unknownDescriptors.empty()) {
      // Finished causalizing :)
      if (debugIsEnabled('c'))
        PrintCausalizationResult();
      if (solve) // @karupayun: assert(solve())?
        SolveEquations();
      return true;
    }

    //list<VectorVertex>::size_type numAcausalEqs = equationDescriptors.size();
    list<VectorVertex>::iterator iter, auxiliaryIter;


    //First, we process the equations' side
    auxiliaryIter = equationDescriptors.begin();
    for(iter = auxiliaryIter; iter != equationDescriptors.end(); iter = auxiliaryIter){ // Ecuación
      // Additional iterator to erase while traversing
      auxiliaryIter++;
      EquationVertex eq = *iter;
      ERROR_UNLESS(out_degree(eq, graph) != 0, "Problem is singular, not supported yet\n");
      // Try to look for a set of indexes to causalize
      Option<std::pair<VectorEdge,IndexPairSet> > op = CanCausalize(eq, kVertexEquation, split); // Acá busca causalizar 
      // If we can causalize something
      if (op) {
        // We are going to causalize something
        causalize_some=true;
        split = false;
        // This pair holds which edge(the first component) to use for causalization and which indexes(the second component)
        std::pair<VectorEdge,IndexPairSet> causal_pair = op.get();
        ERROR_UNLESS(causal_pair.second.size()==1, "Causalizing more than a singleton");
        VectorEdge e = causal_pair.first;
        // This is the unknown node connecting to the edge
        UnknownVertex unk = GetUnknown(e);
        equationNumber--;
        unknownNumber--;
        // Save the result of this step of causalization
        Causalize1toN(graph[unk].unknown, graph[eq].equation, causal_pair.second);
        // Update the pairs in the edge that is being causalized
        if (debugIsEnabled('c')) 
          std::cerr << "Causalizing from the equation side " << causal_pair.second << std::endl;
        graph[e].RemovePairs(causal_pair.second);
        // Decrement the number of uncausalized equations/unknowns
        graph[eq].count -= causal_pair.second.begin()->Dom().Size();
        graph[unk].count -= causal_pair.second.begin()->Ran().Size();
        // If the edge has no more pairs in it remove it
        if (graph[e].IsEmpty()) {
          if (debugIsEnabled('c')) {
            std::cout << "Removing the edge\n";
          }
          remove_edge(e, graph);
        }
        // Auxiliary list to later remove empty edges
        std::list<VectorEdge> remove;
        foreach_(VectorEdge e1, out_edges(unk,graph)) {
          // Update the labels from all the edges adjacent to the unknown
          if (debugIsEnabled('c')) {
            std::cout << "Removing unknowns " << causal_pair.second.begin()->Ran() << " from " << graph[e1]<<"\n";
          }
          graph[e1].RemoveUnknowns(causal_pair.second.begin()->Ran());
          if (debugIsEnabled('c')) {
            std::cout << "Result: " << graph[e1] << "\n";
          }
          // If the edge is now empty schedule it for removal
          if (graph[e1].IsEmpty()) {
            if (debugIsEnabled('c')) {
              std::cout << "Removing the edge\n";
            }
            remove.push_back(e1);
          }
        }
        // Now remove all scheduled edges
        foreach_(VectorEdge e1, remove) {
          WARNING_UNLESS(out_degree(GetEquation(e1),graph)>1, "Disconnecting equation node");
          remove_edge(e1, graph);
        }
        // If the equation node is now unconnected and with count==0 we can remove it
        if (out_degree(eq,graph)==0) {
          ERROR_UNLESS(graph[eq].count==0, "Disconnected node with uncausalized equations");
          remove_vertex(eq,graph);
          equationDescriptors.erase(iter);
        }
        // If the unknown node is now unconnected and with count==0 we can remove it
        if (out_degree(unk,graph)==0) {
          ERROR_UNLESS(graph[unk].count==0, "Disconnected node with uncausalized unknowns");
          remove_vertex(unk,graph);
          unknownDescriptors.remove(unk);
        }
        /*stringstream ss;
        ss << "graph_" << step++ << ".dot";
        GraphPrinter<VectorVertexProperty,Label>  gp(graph);
        gp.printGraph(ss.str());*/
      }
    }

  
    //Now, we process the unknowns' side
    auxiliaryIter = unknownDescriptors.begin();
    for(iter = auxiliaryIter; iter != unknownDescriptors.end(); iter = auxiliaryIter){
      // Additional iterator to erase while traversing
      auxiliaryIter++;
      UnknownVertex unk = *iter;
      ERROR_UNLESS(out_degree(unk, graph) != 0, "Problem is singular, not supported yet\n");
      // Try to look for a set of indexes to causalize
      Option<std::pair<VectorEdge,IndexPairSet> > op = CanCausalize(unk, kVertexUnknown, split);
      // If we can causalize something
      if (op) {
        // We are going to causalize something
        causalize_some=true;
        split = false;
        // This pair holds which edge(the first component) to use for causalization and which indexes(the second component)
        std::pair<VectorEdge,IndexPairSet> causal_pair = op.get();
        VectorEdge e = causal_pair.first;
        // This is the equation node connecting to the edge
        EquationVertex eq = GetEquation(e);
        equationNumber--;
        unknownNumber--;
        // Save the result of this step of causalization
        CausalizeNto1(graph[unk].unknown, graph[eq].equation, causal_pair.second);
        // Update the pairs in the edge that is being causalized
        if (debugIsEnabled('c')) 
          std::cerr << "Causalizing from the unknown side " << causal_pair.second << std::endl;
        graph[e].RemovePairs(causal_pair.second);
        // Decrement the number of uncausalized equations/unknowns
        ERROR_UNLESS(causal_pair.second.size()==1, "Causalizing more than a singleton");
        graph[eq].count -= causal_pair.second.begin()->Dom().Size();
        graph[unk].count -= causal_pair.second.begin()->Ran().Size();
        // If the edge has no more pairs in it remove it
        if (graph[e].IsEmpty()) {
          if (debugIsEnabled('c')) {
            std::cout << "Removing the edge\n" << graph[e] << "\n";
          }
          remove_edge(e, graph);
        }
        // Auxiliary list to later remove empty edges
        std::list<VectorEdge> remove;
        foreach_(VectorEdge e1, out_edges(eq,graph)) {
          // Update the labels from all the edges adjacent to the equation
          if (debugIsEnabled('c')) {
            std::cout << "Removing equations " << causal_pair.second.begin()->Dom() << " from " << graph[e1]<<"\n";
          }
          graph[e1].RemoveEquations(causal_pair.second.begin()->Dom());
          if (debugIsEnabled('c')) {
            std::cout << "Result: " << graph[e1] << "\n";
          }
          // If the edge is now empty schedule it for removal
          if (graph[e1].IsEmpty()) {
            if (debugIsEnabled('c')) {
              std::cout << "Removing the edge\n" << graph[e1] << "\n";
            }
            remove.push_back(e1);
          }
        }
        // Now remove all scheduled edges
        foreach_(VectorEdge e1, remove) {
          if (e1!=e)
            WARNING_UNLESS(out_degree(GetUnknown(e1),graph)>1, "Disconnecting unknown node"); //TODO: Review this condition and error message
          remove_edge(e1, graph);
        }
        // If the equation node is now unconnected and with count==0 we can remove it
        if (out_degree(eq,graph)==0) {
          ERROR_UNLESS(graph[eq].count==0, "Disconnected node with uncausalized equations");
          remove_vertex(eq,graph);
          equationDescriptors.remove(eq);
        }
        // If the unknown node is now unconnected and with count==0 we can remove it
        if (out_degree(unk,graph)==0) {
          ERROR_UNLESS(graph[unk].count==0, "Disconnected node with uncausalized unknowns");
          remove_vertex(unk,graph);
          unknownDescriptors.erase(iter);
        }
        /*stringstream ss;
        ss << "graph_" << step++ << ".dot";
        GraphPrinter<VectorVertexProperty,Label>  gp(graph);
        gp.printGraph(ss.str());*/
      }
    }

    if (!causalize_some && !split) { // Try to split ranges
      split=true;
      continue;
    }
    if(!causalize_some && split){
      //we have a LOOP or a FOR equation that we don't
      //handle at least yet, so we resort to the previous
      //algorithm
      ERROR("Loop detected! We don't handle loops yet!\n");
      return false;
    }
    steps++;
    ERROR_UNLESS(steps<50, "Maximum number of steps reached");
  }
}


Option <IndexPair> CausalizationStrategyVector::TestBreak(VectorEquationVertex eq, 
                                                          VertexType vt, 
                                                          VectorCausalizationGraph::out_edge_iterator edge,
                                                          IndexPairSet::iterator candidate_pair)  
{
  VectorCausalizationGraph::out_edge_iterator other, other_end;
  MDI mdi = (vt==kVertexEquation ? candidate_pair->Dom() : candidate_pair->Ran()) ;
  std::set<MDI> leftovers = {mdi};
  //std::cerr << "Trying to break " << mdi << std::endl;
  for(boost::tie(other,other_end) = out_edges(eq,graph); other != other_end; ++other) {
    const IndexPairSet &ips = graph[*other].Pairs();
    IndexPairSet::iterator test;
    // First find on candidate_edge a possible set of pairs
    for (test = ips.begin(); test!=ips.end(); test++) {
      if (test == candidate_pair && other == edge) 
        continue; // Skip the same pair in the same edge
      MDI toRemove = (vt==kVertexEquation ? test->Dom() : test->Ran());
      std::set<MDI> new_leftovers;
      //std::cerr << "Removing " << toRemove << std::endl;
      for(MDI m : leftovers) {
        std::list<MDI> diff = m - toRemove;
        new_leftovers.insert(diff.begin(), diff.end());
      } 
      if (new_leftovers.size()==0) // If the difference is empty we are done
        return Option<IndexPair>();
      leftovers = new_leftovers;
      /*std::cerr << "Left={ ";
      for(MDI m : leftovers) 
        std::cerr << m;
      std::cerr << "}\n";
      */
    }
  }
  mdi = *leftovers.begin();
  if (vt==kVertexEquation) {
    // Here we are only taking the first of the remaining parts. We could choose anyone so we are taking the first
    return IndexPair(mdi,
                   mdi.ApplyUsage(candidate_pair->GetUsage(),candidate_pair->Ran()).ApplyOffset(candidate_pair->GetOffset()),
                   candidate_pair->GetOffset(),
                   candidate_pair->GetUsage());
  } else { 

    return IndexPair(mdi.RevertUsage(candidate_pair->GetUsage(),candidate_pair->Dom()).ApplyOffset(candidate_pair->GetOffset()),
                   mdi,
                   candidate_pair->GetOffset(),
                   candidate_pair->GetUsage());
  }
}

Option<std::pair<VectorEdge,IndexPairSet> > CausalizationStrategyVector::CanCausalizeBreak(VectorEquationVertex eq, VertexType vt) {
  VectorCausalizationGraph::out_edge_iterator vi, vi_end, other, other_end;
  VectorEdge candidate_edge; 
  IndexPairSet::iterator candidate_pair, test;
  IndexPairSet resultingIPS;
  for(boost::tie(vi,vi_end) = out_edges(eq,graph); vi != vi_end; ++vi) {
    // Try to find a pair in candidate_edge
    candidate_edge = *vi;
    const IndexPairSet &ips = graph[*vi].Pairs();
    // First find on candidate_edge a possible set of pairs
    for (candidate_pair = ips.begin(); candidate_pair!=ips.end(); candidate_pair++) {
      if (candidate_pair->Dom().Size()!=candidate_pair->Ran().Size()) // If they are different size cannot causalize
        continue;
      if (Option<IndexPair> ip = TestBreak(eq,vt, vi, candidate_pair)) { // We found something we can break
        return make_pair(candidate_edge,IndexPairSet({ip.get()}));
      }
    }
  } 
  return Option<std::pair<VectorEdge,IndexPairSet> >();    // First find on candidate_edge a possible set of pairs
}

Option<std::pair<VectorEdge,IndexPairSet> > CausalizationStrategyVector::CanCausalize(VectorEquationVertex eq, VertexType vt, bool split) {
  VectorCausalizationGraph::out_edge_iterator vi, vi_end, other, other_end;
  VectorEdge candidate_edge; 
  IndexPairSet::iterator candidate_pair, test;
  IndexPairSet resultingIPS;
  for(boost::tie(vi,vi_end) = out_edges(eq,graph); vi != vi_end; ++vi) {
    // Try to find a pair in candidate_edge
    candidate_edge = *vi;
    const IndexPairSet &ips = graph[*vi].Pairs();
    // First find on candidate_edge a possible set of pairs
    for (candidate_pair = ips.begin(); candidate_pair!=ips.end(); candidate_pair++) {
      IndexPair candidate_ip = *candidate_pair;
      // A N-to-1 or 1-to-N can not be causalized
      if (candidate_ip.Dom().Size()!=candidate_ip.Ran().Size()) 
        continue;
      
      if (TestPairInCandidateEdge(candidate_pair, candidate_edge, vt)) {
        //We found a candidate pair in the candidate edge
        //Check if this pair is allowed for other edges
        bool collision = false;
        for (boost::tie(other,other_end) = out_edges(eq,graph); other != other_end; ++other) {
          // Skip the candidate_edge
          if (candidate_edge==*other) {
            continue;
          }
          if (CollisionPairInEdge(*candidate_pair, *other, vt)) {
            collision=true;
            break;
          }
        }
        //If there is no collision add the candidate_pair S
        if (!collision) {
          resultingIPS.insert(*candidate_pair);
          return make_pair(candidate_edge, resultingIPS);
        }
      }
    }
    //If we found a suitable set of pairs, return this result
  }
  //At this point we couldn't find any causalizable pair in any edge
  if (split)
    return CanCausalizeBreak(eq,vt); 
  return Option<std::pair<VectorEdge,IndexPairSet> >();    // First find on candidate_edge a possible set of pairs
}


bool CausalizationStrategyVector::TestPairInCandidateEdge(IndexPairSet::iterator ip, VectorEdge edge, VertexType vt) {
  IndexPairSet::iterator test;
  //Test the candidate pair in the edge
  for (test = graph[edge].Pairs().begin(); test !=graph[edge].Pairs().end(); test++) {
    // Skip the same pair in the same edge
    if (ip==test) {
      continue;
    }
    IndexPair candidate = *ip;
    IndexPair test_pair = *test;
    if (vt==kVertexEquation) {
      if (candidate.Dom() & test_pair.Dom()) {
        return false;
      } 
    } else {
      if (candidate.Ran() & test_pair.Ran()) {
        return false;
      } 

    }
  }
  //The pair works in this edge with the current S
  return true;
}


bool CausalizationStrategyVector::CollisionPairInEdge(IndexPair ip, VectorEdge edge, VertexType vt) {
  for (IndexPairSet::iterator test = graph[edge].Pairs().begin(); test !=graph[edge].Pairs().end(); test++) {
    if (vt==kVertexEquation) {
      if (ip.Dom() & test->Dom()) {
        //There is a collision, return true
        return true;
      }
    } else if (vt==kVertexUnknown) {
      if (ip.Ran() & test->Ran() ) {
        //There is a collision, return true
        return true;
      }
    }else {
      ERROR("Wrong vertex type");
    }
  }
  //There is no collision, return false
  return false;
}


void CausalizationStrategyVector::SolveEquations() {
  EquationList all;
  vector<CausalizedVar> sorted_vars = equations1toN;
  sorted_vars.insert(sorted_vars.end(),equationsNto1.begin(), equationsNto1.end());
  foreach_(CausalizedVar cv, sorted_vars) {
    Equation equation = cv.equation;
    if(is<ForEq>(equation)) {
      ERROR_UNLESS(cv.pairs.size() == 1, "Solving scalar equation with more than one index pair");
      IndexPair ip = *cv.pairs.begin();
      MDI dom = ip.Dom(), ran = ip.Ran();
      //ERROR_UNLESS(ip.GetOffset().isZeros(), "Solving with offset not implemented");
      ERROR_UNLESS(dom.Size() == ran.Size(), "Solving with ranges of different size");
      ForEq feq = get<ForEq>(equation);
      VarSymbolTable syms = mmo.syms_ref();
      int index = 0;
      for(Index & i : feq.range_ref().indexes_ref()) {
         VarInfo vinfo = VarInfo(TypePrefixes(), "Integer", Option<Comment>(), Modification());
         syms.insert(i.name(),vinfo);
         i.exp_ref() = Expression(Range(dom.Intervals().at(index).lower(),dom.Intervals().at(index).upper()));
         index++;
      }
      ExpList el;
      index = 0;
      Usage us = ip.GetUsage();
      Offset offset = ip.GetOffset();
      for (Interval i: ran.Intervals()) {
        if (boost::icl::size(i)==1 && us[index]==-1) { // The unknown is used in a unitary range
          el.push_back(i.lower());
        } else {// The unknown index is using a iterator
          ERROR_UNLESS(index<us.Size(), "Range not found in usages");
          Index i = feq.range().indexes().at(us[index]);
          if (offset[index]!=0) {
            el.push_back(BinOp(Reference(i.name()),Add, Expression(offset[index]))) ;
          } else { 
            el.push_back(Reference(i.name())) ;
          }
        }
        index++;
      }
      cv.unknown.SetIndex(el);
      if (debugIsEnabled('c')) {
        std::cout << "Solving:\n" << equation << "\nfor variable " << cv.unknown() << "\n";
      }
      std::list<std::string> c_code;
      ClassList cl;
      all.push_back(EquationSolver::Solve(feq, cv.unknown(), syms, c_code, cl, mmo.name() + ".c"));
    } else{
      ExpList varIndexes;;
      if (cv.unknown.dimension == 0) {
        cv.unknown.SetIndex(varIndexes);
      } else {
         ERROR_UNLESS(cv.pairs.size() == 1, "Solving scalar equation with more than one index pair");
         MDI mdi = cv.pairs.begin()->Ran();
         for(Interval i : mdi.Intervals()) {
          ERROR_UNLESS(boost::icl::size(i)==1, "Interval of size>1 used for solving a scalar equation"); 
          varIndexes.push_back(Expression(i.lower()));
         }
      }
      cv.unknown.SetIndex(varIndexes);
      std::list<std::string> c_code;
      ClassList cl;
      if (debugIsEnabled('c')) {
        std::cout << "Solving\n" << equation << "\nfor variable " << cv.unknown() << "\n";
      }
      all.push_back(EquationSolver::Solve(equation, cv.unknown(), mmo.syms_ref(),c_code, cl, mmo.name() + ".c"));
    }
  }
  mmo.equations_ref().equations_ref()=all;
}

Vertex CausalizationStrategyVector::GetEquation(Edge e) {
  return ((graph[(source(e,graph))].type==kVertexEquation))?source(e,graph):target(e,graph);
}


Vertex CausalizationStrategyVector::GetUnknown(Edge e) {
  return ((graph[(target(e,graph))].type==kVertexUnknown))?target(e,graph):source(e,graph);
}


void
CausalizationStrategyVector::PrintCausalizationResult(){
  vector<CausalizedVar> sorted_vars = equations1toN;
  sorted_vars.insert(sorted_vars.end(),equationsNto1.begin(), equationsNto1.end());
  cout << "Result of causalization: \n";
  foreach_(CausalizedVar cv, sorted_vars) {
    cout << "With equation \n";
    cout << cv.equation;
    cout << "\n solve variable " << cv.unknown();
    cout << " in range " << cv.pairs << "\n";
  }
  cout << "Causalization steps: " << sorted_vars.size() << "\n";
}
}
