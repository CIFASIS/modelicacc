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

#include <causalize/vector/graph_builder.h>
#include <causalize/vector/vector_graph_definition.h>
#include <causalize/vector/contains_vector.h>
#include <util/ast_visitors/eval_expression.h>


#include <boost/graph/adjacency_list.hpp>
#include <boost/icl/interval_set.hpp>
#include <mmo/mmo_class.h>
#include <util/debug.h>
#include <ast/queries.h>

//#ifdef ENABLE_DEBUG_MSG
//#define DEBUG_MSG(str) do {std::cout << str << std::endl;} while( false )
//#else
//#define DEBUG_MSG(str) do {} while( false )
//#endif
#define DEBUG_MSG(str) do {std::cout << str << std::endl;} while( false )

using namespace std;
using namespace boost::icl;
using namespace Modelica;
using namespace Modelica::AST;

namespace Causalize { 
ReducedGraphBuilder::ReducedGraphBuilder(MMO_Class &mmo_cl): mmo_class(mmo_cl), state_finder(mmo_cl) {
}

VectorCausalizationGraph ReducedGraphBuilder::makeGraph() {
  /* Create nodes for the Equations*/
  foreach_ (Equation e, mmo_class.equations().equations()) {
    static int index = 0;
    VectorVertexProperty vp;
    vp.type = kVertexEquation;
    vp.equation = e;
    if (is<ForEq>(e)) {
      vp.count=getForRangeSize(get<ForEq>(e));
    } else if (is<Equality>(e)) {
      vp.count=1;
    } else {
      ERROR("Only causalization of for and equality equations");
    }
    vp.index=index++;
    equationDescriptorList.push_back(add_vertex(vp,graph));
  }
 /* Create nodes for the unknowns: We iterate through the VarSymbolTable
  * and create one vertex per unknown */
  state_finder.findStateVariables();
  foreach_ (Name var, mmo_class.variables()) {
    static int index = 0;
    const VarSymbolTable &syms = mmo_class.syms_ref();
    VarInfo varInfo = syms[var].get();
    if (!isConstant(var,syms) && !isBuiltIn(var,syms) && !isDiscrete(var,syms) && !isParameter(var,syms)) {
      if (varInfo.modification() && is<ModEq>(varInfo.modification().get())) // if the var has a fixed value over time is not a unknown
        continue;
      VectorVertexProperty vp;
      vp.type=kVertexUnknown;
      vp.index=index++;
      vp.unknown = VectorUnknown(varInfo, var);
      if ("Real"==varInfo.type()) {
        int totalUnknowns = 1;
        if (!varInfo.indices())  {  //Is a scalar unknown
          vp.count=totalUnknowns;
        } else { //Is a vector unknown
          ExpList indexes = varInfo.indices_.get();
          EvalExpression ev(mmo_class.syms_ref());
          foreach_(Expression i, indexes) {
            int indexSize = Apply(ev,i);
            totalUnknowns *= indexSize;
            vp.unknown.dimensionList.push_back(indexSize);
          }
          vp.count = totalUnknowns;
        }
      } else 
        ERROR("Unknown type: %s", varInfo.type().c_str());
      unknownDescriptorList.push_back(add_vertex(vp, graph));
    }
  }
   if(debugIsEnabled('c')){
     DEBUG_MSG("Equations");
     foreach_ (VectorEquationVertex eq, equationDescriptorList){
       DEBUG_MSG(graph[eq].index << ": " << graph[eq].equation) ;
     }
     DEBUG_MSG("Unknowns");
     foreach_(VectorUnknownVertex un, unknownDescriptorList){
       DEBUG_MSG(graph[un].index << ": " << graph[un].unknown()) ;
     }
   }


  foreach_ (VectorEquationVertex eq, equationDescriptorList){
    foreach_(VectorUnknownVertex un, unknownDescriptorList){
      Expression unknown = graph[un].unknown();
      VarSymbolTable syms = mmo_class.syms_ref();
      Equation e = graph[eq].equation;
      if (is<Equality>(e)) {
        Causalize::ContainsVector occurrs(unknown, graph[un], syms);
        Equality eqq = boost::get<Equality>(e);
        const bool rl = Apply(occurrs,eqq.left_ref());
        const bool rr = Apply(occurrs,eqq.right_ref()); 
        if(rl || rr) {
          Label ep(occurrs.GetOccurrenceIndexes());
          add_edge(eq, un, ep, graph);
        } 
      } else if (is<ForEq>(e)) {
        ForEq feq = get<ForEq>(e);
        ERROR_UNLESS(feq.elements().size()==1, "For equation with more than one equation not supported");
        Equation inside = feq.elements().front();
        ERROR_UNLESS(is<Equality>(inside), "Only equality equation inside for loops supported");
        Equality eqq = boost::get<Equality>(inside);
        IndexList ind = feq.range().indexes();
        VarSymbolTable syms_for = mmo_class.syms_ref();
        Causalize::ContainsVector occurrs_for(graph[un], syms_for, ind);
        const bool rl = Apply(occurrs_for,eqq.left_ref());
        const bool rr = Apply(occurrs_for,eqq.right_ref()); 
        if(rl || rr) {
          Label ep(occurrs_for.GetOccurrenceIndexes());
          add_edge(eq, un, ep, graph);
        } 
      } else
        ERROR_UNLESS(is<Equality>(e), "Only causalization of equality and for equation is supported");
    }
  }
  DEBUG('c', "\n");
  return graph;
}


int ReducedGraphBuilder::getForRangeSize(ForEq feq) {
  IndexList ind = feq.range().indexes();
  Index i = ind.front();
  int equations = 1;
  foreach_(Index i, ind) {
    if (!i.exp())
      ERROR("graph_builder:\n No expression on for equation");
    Expression exp = i.exp().get();
    if (is<Brace>(exp)) {
      equations *= get<Brace>(exp).args().size();
    } else if (is<Range>(exp)) {
      Range range = get<Range>(exp);
      ERROR_UNLESS(!range.step(), "graph_builder: FOR ranges with leaps not supported yet");
      EvalExpression ev(mmo_class.syms_ref());
      equations *= Apply(ev,range.end_ref())-Apply(ev,range.start_ref())+1;
    } else {
      ERROR("Expression in FOR Index not supported\n");
    }
  }
  return equations;
}
}
