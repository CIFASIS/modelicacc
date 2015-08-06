#include <causalize/vector/graph_builder.h>
#include <causalize/vector/graph_printer.h>
#include <causalize/vector/graph_definition.h>
#include <causalize/vector/contains_vector.h>
#include <util/ast_visitors/evalexp.h>


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
using namespace boost;
using namespace boost::icl;
using namespace Modelica;
using namespace Modelica::AST;

ReducedGraphBuilder::ReducedGraphBuilder(MMO_Class &mmo_cl):GraphBuilder(mmo_cl), state_finder(mmo_cl) {
}

CausalizationGraph ReducedGraphBuilder::makeGraph() {
  /* Create nodes for the Equations*/
  foreach_ (Equation e, mmo_class.equations().equations()) {
    static int index = 0;
    VertexProperties vp;
    vp.type = E;
    vp.equation = e;
    if (is<ForEq>(e)) {
      vp.count=getForRangeSize(get<ForEq>(e));
      vp.eqType=EQFOR;
    } else if (is<Equality>(e)) {
      vp.eqType=EQEQUALITY;
      vp.count=1;
    } else {
      ERROR("Only causalization of for and equality equaitons");
    }
    vp.index=index++;
    Vertex eqDescriptor = add_vertex(vp,graph);
    equationDescriptorList.push_back(eqDescriptor);
  }
 /* Create nodes for the unkowns: We iterate through the VarSymbolTable 
  * and create one vertex per unknown */
  state_finder.findStateVariables();
  foreach_ (Name var, mmo_class.variables()) {
    static int index = 0;
    const VarSymbolTable &syms = mmo_class.syms_ref();
    VarInfo varInfo = syms[var].get();
    if (!isConstant(var,syms) && !isBuiltIn(var,syms) && !isDiscrete(var,syms) && !isParameter(var,syms)) {
      VertexProperties vp;
      vp.type=U;
      vp.variableName=var;
      vp.index=index++;
      vp.isState=varInfo.state();
      if ("Real"==varInfo.type()) {
        if (!varInfo.indices())  {
          vp.count=0;
        } else if (varInfo.indices().get().size()==1) {
          EvalExp ev(mmo_class.syms_ref()); 
          vp.count = boost::apply_visitor(ev,varInfo.indices().get().front());
        } else {
          ERROR("ReducedGraphBuilder::makeGraph Arrays of arrays are not supported yet\n");  
        }
      }
      unknownDescriptorList.push_back(add_vertex(vp, graph));
    }
  }
   if(debugIsEnabled('c')){
     DEBUG_MSG("Equations");
     foreach_ (Vertex eq, equationDescriptorList){
       DEBUG_MSG(graph[eq].index << ": " << graph[eq].equation) ;
     }
     DEBUG_MSG("Unknowns");
     foreach_(Vertex un, unknownDescriptorList){
       string var_name;
       if(graph[un].isState){
         var_name = "der(" + graph[un].variableName + ")";
       }else{
         var_name = graph[un].variableName;
       }
       DEBUG_MSG(graph[un].index << ": " << var_name) ;
     }
   }


  foreach_ (Vertex eq, equationDescriptorList){
    foreach_(Vertex un, unknownDescriptorList){
      Expression ref;
      if (graph[un].isState)
        ref = Call("der",ExpList(1,Reference(Option<Name>(),graph[un].variableName, Option<ExpList>())));
      else
        ref=Reference(Option<Name>(),graph[un].variableName,Option<ExpList>());
      
      VarSymbolTable syms = mmo_class.syms_ref();
      if (graph[eq].count > 1) {
        // is a for equation
        ForEq fe = get<ForEq>(graph[eq].equation);
        Index i = fe.range().indexes().front();
        Name var = i.name();
        VarInfo v(TypePrefixes(0),"Real");
        syms.insert(var,v);
      }
      Modelica::contains_vector occurrs(ref,graph[un], syms);
      Equation e = graph[eq].equation;
      if (is<Equality>(e)) {
        Equality eqq = boost::get<Equality>(e);
        //std::cerr << eqq.left_ref() << "***********\n";
        const bool rl = boost::apply_visitor(occurrs,eqq.left_ref());
        //std::cerr << eqq.right_ref() << "***********\n";
        const bool rr = boost::apply_visitor(occurrs,eqq.right_ref()); 
        //std::cerr << "Result: " << rl << " " << rr << "\n";
        if(rl || rr) {
          foreach_(EdgeProperties ep, occurrs.getOccurrenceIndexes()) {
            add_edge(eq, un, ep, graph);
            DEBUG('c', "(%d, %d) ", graph[eq].index, graph[un].index);
          }
        } 
      } else if (is<ForEq>(e)) {
        ForEq feq = get<ForEq>(e);
        ERROR_UNLESS(feq.elements().size()==1, "For equation with more than one equation not supported");
        Equation inside = feq.elements().front();
        ERROR_UNLESS(is<Equality>(inside), "Only equality equation inside for loops supported");
        Equality eqq = boost::get<Equality>(inside);
        IndexList ind = feq.range().indexes();
        ERROR_UNLESS(ind.size() == 1, "graph_builder:\n For Loop with more than one index is not supported yet\n");
        Index i = ind.front();
        ERROR_UNLESS(i.exp(), "graph_builder:\n No expression on for equation");
        Expression exp = i.exp().get();
        ERROR_UNLESS(is<Range>(exp), "Only range expression in for equations");
        Range range = get<Range>(exp);
        ERROR_UNLESS(!range.step(), "Range with step not supported");

        VarSymbolTable syms_for = mmo_class.syms_ref();
        syms_for.insert(i.name(),VarInfo(TypePrefixes(0),"Integer"));
        Modelica::contains_vector occurrs_for(ref,graph[un], syms_for);
        occurrs_for.setForIndex(range.start(),range.end());
        
        const bool rl = boost::apply_visitor(occurrs_for,eqq.left_ref());
        const bool rr = boost::apply_visitor(occurrs_for,eqq.right_ref()); 
        if(rl || rr) {
          //std::cerr << "Var: " << graph[un].variableName << "\n";
          //std::cerr << "Eq: " << e << "\nleft=" << rl << " right = " << rr << "\n";
          bool some=false;
          foreach_(EdgeProperties ep, occurrs_for.getOccurrenceIndexes()) {
            some=true;
            add_edge(eq, un, ep, graph);
            DEBUG('c', "(%d, %d) ", graph[eq].index, graph[un].index);
          }
          if (!some) {
            ERROR("Something went wrong computing ocurrences");
          }
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
  ERROR_UNLESS(ind.size() == 1, "graph_builder:\n For Loop with more than one index is not supported yet\n");
  Index i = ind.front();
  ERROR_UNLESS(i.exp(), "graph_builder:\n No expression on for equation");
  Expression exp = i.exp().get();
  if (is<Brace>(exp))
    return get<Brace>(exp).args().size();
  if (is<Range>(exp)) {
    Range range = get<Range>(exp);
    ERROR_UNLESS(!range.step(), "graph_builder: FOR ranges with leaps not supported yet");
    EvalExp ev(mmo_class.syms_ref()); 
    return boost::apply_visitor(ev,range.end_ref())-boost::apply_visitor(ev,range.start_ref())+1;
  }
  ERROR("Expression in FOR Index not supported\n");
  return 0;
}
