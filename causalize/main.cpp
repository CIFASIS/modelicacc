
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

#include <ast/ast_types.h>
#include <parser/parser.h>
#include <ast/equation.h>
#include <mmo/mmo_class.h>
#include <util/debug.h>
#include <causalize/for_unrolling/process_for_equations.h>
#include <causalize/causalization_strategy.h>
#include <causalize/vector/causalization_algorithm.h>
#include <causalize/vector/graph_builder.h>
#include <causalize/vector/graph_printer.h>
#include <cstdlib>
#include <stdio.h>
#include <iostream>
#include <boost/variant/get.hpp>
#include <splitfor/splitfor.h>


using namespace std;
using namespace Modelica;
using namespace Modelica::AST;
using namespace Causalize;
//using namespace Modelica::ExpandFor;

int main(int argc, char ** argv)
{

  bool r;
  int opt;
  //bool for_qss;

  while ((opt = getopt(argc, argv, "d:q")) != -1) {
    switch (opt) {
     case 'd':
       if (optarg != NULL && isDebugParam(optarg)) {
         debugInit(optarg);
       } else {
         ERROR("command-line option d has no arguments\n");
       }
       break;
     case 'q':
       //for_qss=true;
       break;
    }
  }

  StoredDef sd;
  if (argv[optind]!=NULL) 
    sd=parseFile(argv[optind],r);
  else
    sd=parseFile("",r);
 
  if (!r) 
    return -1;

  Class ast_c = boost::get<Class>(sd.classes().front());
  MMO_Class mmo(ast_c);
  
  SplitFor sf(mmo);
  sf.splitFor();
  cout << mmo << endl;

  ReducedGraphBuilder gb(mmo);
  CausalizationGraph g = gb.makeGraph();
  GraphPrinter gp(g);
  gp.printGraph("grafo.dot");
  CausalizationStrategyVector cs(g,mmo);
  /*
  if(cs.causalize()){ // Try vectorial causalization first
    if(debugIsEnabled('c')){
     cout << "Result of causalization (variable, [range,] equationID):" << endl;
       cs.print();
    }
    cout << mmo << endl;
    return 0;
  }
 // return 0;
  */
 
  CausalizationStrategy cStrategy(mmo);
  cStrategy.causalize("a");
  DEBUG('c', "Causalized Equations:\n");
  foreach_(const Equation &e, mmo.equations_ref().equations_ref()) {
    if (debugIsEnabled('c'))
      cerr << e << std::endl;
  }
  cout << mmo << endl;

  return 0;
}
