
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
#include <cstdlib>
#include <stdio.h>
#include <iostream>
#include <boost/variant/get.hpp>
#include <causalize/vector/splitfor.h>
#include "boost/date_time/posix_time/posix_time.hpp"



using namespace std;
using namespace Modelica;
using namespace Modelica::AST;
using namespace Causalize;
bool solve = true;
bool tarjan = false;

int main(int argc, char ** argv)
{

  bool r;
  int opt;
  bool vectorial = false;
	
  while ((opt = getopt(argc, argv, "d:vst")) != -1) {
    switch (opt) {
     case 'd':
       if (optarg != NULL && isDebugParam(optarg)) {
         debugInit(optarg);
       } else {
         ERROR("command-line option d has no arguments\n");
       }
       break;
     case 'v':
       vectorial = true;
       break;
     case 't':
	   tarjan = true;
	   vectorial = true;
	   break;
     case 's':
       solve = false;
       break;
    }
  }

  StoredDef sd;
  if (argv[optind]!=NULL) 
    sd=Parser::ParseFile(argv[optind],r);
  else
    sd=Parser::ParseFile("",r);
 
  if (!r) 
    return -1;

  Class ast_c = boost::get<Class>(sd.classes().front());
  MMO_Class mmo(ast_c);
  if (vectorial) {
    SplitFor sf(mmo);
    sf.splitFor();
    ReducedGraphBuilder gb(mmo);
    VectorCausalizationGraph g = gb.makeGraph();
    CausalizationStrategyVector cs(g,mmo);
    boost::posix_time::ptime time_start(boost::posix_time::microsec_clock::local_time());
    if(cs.Causalize()){ // Try vectorial causalization first
      boost::posix_time::ptime time_end(boost::posix_time::microsec_clock::local_time());
      boost::posix_time::time_duration diff = time_end - time_start;
      std::cerr << diff.total_nanoseconds()/1e6 << std::endl;
      if(debugIsEnabled('c')){
        cs.PrintCausalizationResult();
      }
      cout << mmo << endl;
      return 0;
    }
    return 0;
  }
  boost::posix_time::ptime time_start(boost::posix_time::microsec_clock::local_time());
  CausalizationStrategy cStrategy(mmo);
  cStrategy.Causalize();
      boost::posix_time::ptime time_end(boost::posix_time::microsec_clock::local_time());
      boost::posix_time::time_duration diff = time_end - time_start;
      std::cerr << diff.total_nanoseconds()/1e6 << std::endl;
  DEBUG('c', "Causalized Equations:\n");
  foreach_(const Equation &e, mmo.equations_ref().equations_ref()) {
    if (debugIsEnabled('c'))
      cerr << e << std::endl;
  }
  cout << mmo << endl;
  return 0;
}
