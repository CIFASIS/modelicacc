
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
#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <boost/variant/static_visitor.hpp>
#include <flatter/mmo_graph.h>
#include <mmo/mmo_class.h>
#include <ast/equation.h>
#include <ast/statement.h>
#include <ast/element.h>
#include <util/type.h>
#include <boost/variant/get.hpp>
#include <util/ast_visitors/eval_expression.h>
#include <util/ast_visitors/replace_expression.h>
#include <util/ast_visitors/constant_expression.h>
#include <flatter/classFinder.h>
#include <set>

using namespace Modelica;
using namespace Modelica::AST;

typedef boost::tuple<Expression,OptExp,OptExp> 	Vars;
typedef std::vector<Vars>	 					VarsList;
typedef boost::tuple<OptExp,VarsList,bool>	 	Solution;
typedef std::vector<Solution> 					SolList;
typedef std::set<Vertex>						VertexSet;
typedef std::set<Edge>							EdgeSet;
typedef std::set<EdgeSet>						EdgeSetSet;

class Connectors {
public:
	
	Connectors(MMO_Class &c);
	   
	void 	createGraph(EquationList & eqs, Option<Name> i , OptExp range);
	void 	createEdge(Vertex  , Vertex , OptExp);
	Vertex 	createConnect(OptExp i);
	Vertex 	createElements(Expression e,OptExp i);
	void 	expandConnect(Vertex l, OptExp lIndex,Vertex r, OptExp rIndex, OptExp range);
	
	
	Option<Vertex> 			findVertex(Expression e);
	Pair<Expression,OptExp>	separate(Expression e);
	
	void		resolve(MMO_Class &c);
	void 		Debug(char *);
	void  		writeGraphViz(char *);
private:
	MMOGraph G;
	MMOGraph G_Debug;
	MMO_Class & class_;
	
	void 		print();
	
	Vertex 		getSink(Edge e,Vertex v);
	Vertex 		getVertex(Edge e);
	Vertex 		getConnect(Edge e);
	OptExp 		getRange(Edge e);
	OptExp 		getIndex(Edge e);
	
	Vertex 		getSinkVertex(Edge e);
	Edge 		getSinkEdge(Edge e);
	bool 		isEneUno(Edge e);
	
	
	
	SolList 	VisitarVertice(Vertex v,OptEdge e,OptExp iter,bool eneuno);
	OptExp 		intersectionIntervals(OptExp a, OptExp b);
	OptExp 		getIntervalo(Edge e);
	
	OptExp 		applyIntervalo(Expression inter,OptExp index);
	OptExp 		proyInter(Expression inter,Edge e);
	OptExp 		proyInterInv(Expression inter,Edge e);
	
	Expression 	applyIndex(Expression index,Expression e);
	Expression 	applyIndexInv(Expression index,Expression e);
	
	SolList 	proySolutions(SolList sols, Edge e);
	SolList 	proySolutionsInv(SolList sols, Edge e);
	
	void 		combine(std::vector<SolList> sols, Option<Solution> temp, SolList & _final);
	OptExpList	cutRange(OptExp a,OptExp b);
	void 		cutEdge(OptEdge oe, OptExp iter_);
	void  		markUsed(Vertex c,OptExp iter_);
	void 		markVisited(Vertex c);
	void 		printSols(SolList sols);
	SolList 	resolveConnect();	
	
	
	void 		createEqualityEquation(OptExp iter,VarsList vars,Name e,bool);
	void 		createFlowEquation(OptExp iter,VarsList vars,MMO_Class & conector,Name e,bool);
	void 		createEquation(Solution sol);
	void 		ceroEquations();
	Expression 	putIndex(Vars v,Option<EvalExpression> eval);
	Expression 	addIndex(Expression name, OptExp i ,Option<EvalExpression> eval);
	bool 		isArray(Expression name);
	Name 		getName(Expression name);
	Expression 	addSufix(Expression name,Name var);
	Expression 	removeUnary(Expression name);
	
	SolList 	c_sols; 
	
};


#endif
