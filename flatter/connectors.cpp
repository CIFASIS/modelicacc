
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

#include <flatter/connectors.h>
#include <iostream>
using namespace std;
using namespace Modelica;
using namespace Modelica::AST
;
#define apply(X,Y) 		boost::apply_visitor(X,Y)
#define NameToRef(X)  	Reference(Reference(), X , Option<ExpList>())
#define RefIndex(X,Y)  	Reference(Reference(), X , Option<ExpList>(Y))
#define PrintOpt(N)		(N?N.get():"{}")
std::vector<std::string> NameVec;

Connectors::Connectors(MMO_Class &c):class_(c){}

void Connectors::resolve(MMO_Class &c)
{ 	
	class_ = c;
	createGraph(c.equations_ref().equations_ref(), Option<Name>(), OptExp());
	G_Debug = G;
	

	c_sols = resolveConnect();
	
	foreach_(Solution sol,c_sols) 
		createEquation(sol);	
	ceroEquations();	
}

/*
 * Funciones de Debug y otros
 * 
 */

void Connectors::Debug(char * filename)
{
	if (filename) writeGraphViz(filename);
		
	cerr << "Generated Connect Graph: " << endl;
	print();
	cerr << "-----------------------------------------" << endl;
	
	cerr << "Solutions: " << endl;
	printSols(c_sols);
	cerr << "-----------------------------------------" << endl;
}

Expression removePlus(Expression e)
{
	if (is<UnaryOp>(e)) {
		UnaryOp b = boost::get<UnaryOp>(e);
		if (b.op() == Plus) return b.exp();
	} 
	return e;	
} 

void Connectors::writeGraphViz(char * filename)
{
	
	cerr << "Writing GraphViz file " << endl;
	std::ofstream dotfile (filename);
	dotfile << "graph G {" << endl;
	
	dotfile << "\tsubgraph cluster_0 {" << endl;
	MMOGraphVertexIteretor vi , vi_end;
	for (tie(vi, vi_end) = vertices(G_Debug); vi != vi_end; ++vi) 
		if (G_Debug[*vi].name)
			dotfile << "\t\tnode" << *vi << " [label=\"" << removePlus(G_Debug[*vi].name.get()) << "\"];" << endl;	
			
	dotfile << "\tedge[style=invis];" << endl;		
	for (tie(vi, vi_end) = vertices(G_Debug); vi != vi_end; ++vi) 
		if (G_Debug[*vi].name)
			dotfile << "node" << *vi << ( vi + 1 != vi_end ? " -- " : ";" ); 
			
	dotfile << endl << "\t}" << endl;
	
	dotfile << "\tsubgraph cluster_1 {" << endl;
	for (tie(vi, vi_end) = vertices(G_Debug); vi != vi_end; ++vi) 
		if (!G_Debug[*vi].name)
			dotfile << "\t\tnode" << *vi << " [label=\"" << PrintOpt((G_Debug[*vi].range)) << "\"];" << endl;
			
	dotfile << "\tedge[style=invis];" << endl;		
	for (tie(vi, vi_end) = vertices(G_Debug); vi != vi_end; ++vi) 
		if (!G_Debug[*vi].name)
			dotfile << "node" << *vi << ( vi + 1 != vi_end ? " -- " : ";" );
					
	dotfile << endl << "\t}" << endl;
	
	MMOGraphEdgeIteretor i,end;
	for (tie(i, end) = edges(G_Debug); i != end; ++i) {
		dotfile << "\tnode" << source(*i, G_Debug) << " -- " << "node" << target(*i, G_Debug) << " [label=\""  <<  PrintOpt(G_Debug[*i].index) << "\"]; " << endl;
	}
	
	dotfile << "}" << endl;
	dotfile.close();
}

void Connectors::printSols(SolList sols)
{	
	// Just for debuging
	foreach_ (Solution s,sols) {
		std::cerr << "{" << (get<0>(s) ? get<0>(s).get() : " {} ")  << ","  << get<2>(s) << ", {" ;
		foreach_(Vars v, get<1>(s)) {
			std::cerr << "(" << get<0>(v) <<  ", " << (get<1>(v) ? get<1>(v).get() : "{}") <<  ", " << (get<2>(v) ? get<2>(v).get() : "{}") << ")"  ;
		}
		std::cerr << "}} \n" ;
	}
}

void Connectors::print()
{   
	// Just for debuging
	MMOGraphVertexIteretor vi , vi_end;
	for (tie(vi, vi_end) = vertices(G_Debug); vi != vi_end; ++vi)
		if (G_Debug[*vi].name) cerr <<  *vi << ": " << G_Debug[*vi].name.get() << "(" << G_Debug[*vi].hasIndex << ")" << endl;
		else cerr <<  *vi << ": C -> " << (G_Debug[*vi].range ? G_Debug[*vi].range.get() : "{}" ) << endl;
	MMOGraphEdgeIteretor i,end;
	for (tie(i, end) = edges(G_Debug); i != end; ++i) {
		cerr << source(*i, G_Debug) << " -> " << target(*i, G_Debug) << " Index: "  <<  PrintOpt(G_Debug[*i].index) << endl;
	}
	
	for (tie(vi, vi_end) = vertices(G); vi != vi_end; ++vi)
		if (G[*vi].name) {
			cerr <<  *vi << ": " << G[*vi].name.get() << " Free: ";
			foreach_(OptExp e, G[*vi].free)
				cerr << PrintOpt(e) << ","; 
			cerr << endl;	
		}		
		
	
}

/**
 ***********************************************************************
 * 
 * 		FUNCIONES PARA EL CREAR EL GRAFO 
 * 
 ***********************************************************************
 ** 
*/

void Connectors::createGraph(EquationList & eqs, Option<Name> name , OptExp range)
{
	EquationList to_delete;
	foreach_(Equation & eq, eqs) {
		if (is<Connect>(eq)) {
			to_delete.push_back(eq);
			Connect co = boost::get<Connect>(eq);			
			Expression eleft=co.left(), eright=co.right();
			if (name) {
				Modelica::replace r = Modelica::replace(NameToRef(name.get()), NameToRef("i"));
				eleft = apply(r,eleft);
				eright = apply(r,eright);
			}
			
			Pair<Expression,OptExp> left = separate(eleft);
			Pair<Expression,OptExp> right = separate(eright);
			
			Vertex l = createElements(get<0>(left),get<1>(left));
			Vertex r = createElements(get<0>(right),get<1>(right));
			Vertex c = createConnect(range);
			
			if (l == r) expandConnect(l,get<1>(left),r,get<1>(right),range);
			else if (range && is<Range>(range.get()) && boost::get<Range>(range.get()).step()) expandConnect(l,get<1>(left),r,get<1>(right),range);
			else { 
				createEdge(l,c,get<1>(left));
				createEdge(r,c,get<1>(right));
			}	
							
		} else if (is<ForEq>(eq)) {
			ForEq feq = boost::get<ForEq>(eq);			
			Index ind = feq.range().indexes().front();
			Option<Name> n = Option<Name>(ind.name());
			createGraph(feq.elements_ref(),n,ind.exp());
			eq = feq;
			if (feq.elements_ref().size() == 0) to_delete.push_back(eq);
		}
	}
	foreach_(Equation  eq, to_delete) {
		EquationList::iterator it = std::find(eqs.begin(),eqs.end(),eq);
		if (it != eqs.end()) 
			eqs.erase(it);
	}
	
}

void Connectors::expandConnect(Vertex l, OptExp lIndex,Vertex r, OptExp rIndex, OptExp range)
{
	if (range && is<Range>(range.get())) {
		Range rge = boost::get<Range>(range.get());
		EvalExp evalIndex = EvalExp(class_.syms_ref());
		Real step = 1;
		if (rge.step()) 
			step = boost::apply_visitor(evalIndex,rge.step().get());
		for(Real i = boost::apply_visitor(evalIndex,rge.start_ref()); i <= boost::apply_visitor(evalIndex,rge.end_ref()); i = i + step  ){
			EvalExp eval = EvalExp(class_.syms_ref(),"i",i);
			OptExp opl,opr; 
			if (lIndex) opl = OptExp(boost::apply_visitor(eval,lIndex.get()));
			if (rIndex) opr = OptExp(boost::apply_visitor(eval,rIndex.get()));
			Vertex c = createConnect(range);
			createEdge(l,c,opl);
			createEdge(r,c,opr);
		}
		
	} else std::cerr << "Error: For without range expression or other type" << std::endl;
}

void Connectors::createEdge(Vertex s , Vertex c,OptExp i)
{
	Expression uno = 1;
	EdgeProperties * ep = new EdgeProperties;
	ep->index = (i ? i : OptExp(uno));
	ep->visited = false;
	boost::add_edge(s,c,*ep,G);
}


Vertex Connectors::createConnect(OptExp i)
{
	VertexProperties * vp = new VertexProperties;
	vp->name = OptExp();
	vp->hasIndex = false;
	vp->range = i;
	return boost::add_vertex(*vp,G);
}

Vertex Connectors::createElements(Expression e,OptExp i)
{
	Option<Vertex> v = findVertex(e);
	if (v) return v.get();
	
	OptExp r;
	Name n = getName(e);
	Option<VarInfo> vi = class_.syms_ref()[n];
	if (vi) {
		VarInfo v = vi.get();
		if (v.indices()) {
			Expression rr = Range(1,v.indices().get().front());
			r = rr;
		}	
	}
	
	VertexProperties * vp = new VertexProperties;
	vp->name = e;
	vp->hasIndex = i ? true : false;
	vp->range = OptExp();
	vp->free = OptExpList(1,r);
	return boost::add_vertex(*vp,G);	
}

Option<Vertex> Connectors::findVertex(Expression e)
{
	MMOGraphVertexIteretor vi , vi_end;
	for (tie(vi, vi_end) = vertices(G); vi != vi_end; ++vi)
		if(G[*vi].name && G[*vi].name.get() == e) return Option<Vertex>(*vi);
	return Option<Vertex>();	
}

Pair<Expression,OptExp> Connectors::separate(Expression e)
{
	Reference reference;
	UnaryOpType op = Plus;
	bool isU = false;
	if (is<UnaryOp>(e)) {
		UnaryOp u = boost::get<UnaryOp>(e);
		if (is<Reference>(u.exp())) {
			isU = true;
			reference = boost::get<Reference>(u.exp());
			op = u.op();
		} else std::cerr << "ERROR: Deberia llegar una Reference" << std::endl; 	
	} else if (is<Reference>(e)) {
		reference = boost::get<Reference>(e);
	}
	Ref refs = reference.ref();
	if (refs.size() > 1) 
		std::cerr << "ERROR: No deberia haber llamadas a miembros en connectors" << std::endl; 	
	RefTuple rf = refs.front();	
	OptExp opti;
	if (get<1>(rf).size() > 0)
		opti = get<1>(rf).front();
	Expression r = Reference(Reference(), get<0>(rf), Option<ExpList>());
	if (isU) r = UnaryOp(r,op);	 
	return Pair<Expression,OptExp>(r,opti);
}


/**
 ***********************************************************************
 * 
 * 		FUNCIONES PARA EL ARGOTIRMO
 * 
 ***********************************************************************
 ** 
*/

Vertex Connectors::getSink(Edge e,Vertex v)
{
	if (source(e,G) == v) return target(e,G);
	return source(e,G);
}

Vertex Connectors::getVertex(Edge e)
{
	if (G[source(e,G)].name) return source(e,G);
	return target(e,G);
}

Vertex Connectors::getConnect(Edge e)
{
	if (!G[source(e,G)].name) return source(e,G);
	return target(e,G);
}

OptExp Connectors::getRange(Edge e)
{
	return G[getConnect(e)].range;
}


OptExp Connectors::getIndex(Edge e)
{
	return G[e].index;
}

//**********************************************************************

Vertex Connectors::getSinkVertex(Edge e)
{
	Vertex c = getConnect(e);
	MMOGraphOutputEdgeIteretor i,end;
	for (tie(i, end) = out_edges(c,G); i != end; ++i) 
		if (*i != e ) return getVertex(*i);
	std::cerr << "Error buscando vertice destino para " << e << endl;
	exit(-1);	
}

Edge Connectors::getSinkEdge(Edge e)
{
	Vertex c = getConnect(e);
	MMOGraphOutputEdgeIteretor i,end;
	for (tie(i, end) = out_edges(c,G); i != end; ++i) 
		if (*i != e ) return *i;
	std::cerr << "Error buscando edge destino para " << e << endl;
	exit(-1);	
}

bool Connectors::isEneUno(Edge e)
{
	constExp ce = constExp();
	Expression i1 = getIndex(e).get();
	Expression i2 = getIndex(getSinkEdge(e)).get();
	return  (apply(ce,i1) ^ apply(ce,i2)) && getRange(e);
}



/*OptEdge Connectors::getEdge(Vertex v1,Vertex v2) 
{
	MMOGraphEdgeIteretor i,end;
	for (tie(i, end) = edges(G); i != end; ++i) 
		if ( (source(*i,G) == v1 && target(*i,G) == v2) || (source(*i,G) == v2 && target(*i,G) == v1) ) return OptEdge(*i);
	return OptEdge();
}*/


/* Aplica el rango del for sobre el indice de la variable para calcular el intervalo
 *  Subindice: i + 1
 *  For i in 1:10
 *  Return 2:11
 */ 
OptExp Connectors::getIntervalo(Edge e)
{
	OptExp index = getIndex(e);
	OptExp range = getRange(e);
 	if (G[e].visited) return OptExp();
	if (index && range) {
		Range r = boost::get<Range>(range.get());
		EvalExp val = EvalExp(class_.syms_ref());
		EvalExp sval = EvalExp(class_.syms_ref(),"i",apply(val,r.start_ref()));
		EvalExp eval = EvalExp(class_.syms_ref(),"i",apply(val,r.end_ref()));
		
		Expression start = apply(sval,index.get());
		Expression end = apply(eval,index.get());
		
		Range result = Range(start,end);
		return OptExp(result);		
	}
	return index;	
}

/* Interseccion de dos intervalos
 * [2:10]  /\ [5:7] == [5:7]
 * [2:10]  /\ [5:20] == [5:10]
 */
 
OptExp Connectors::intersectionIntervals(OptExp a, OptExp b)
{
	if (!a) return a;
	if (!b) return b;
	EvalExp eval = EvalExp(class_.syms_ref());
	Real start1,start2,end1,end2;
	
	if (!is<Range>(a.get())) {
		start1 = end1 = apply(eval,a.get());
	} else {
		Range ae = boost::get<Range>(a.get());
		start1 = apply(eval,ae.start_ref());
		end1 = apply(eval,ae.end_ref());
	}
	
	if (!is<Range>(b.get())) {
		start2 = end2 = apply(eval,b.get());
	} else {
		Range be = boost::get<Range>(b.get());
		start2 = apply(eval,be.start_ref());
		end2 = apply(eval,be.end_ref());
	}
	if (start1 == end1 && start2 == end2 && start1 == end1) return OptExp(start1);
	if (start1 == end1 && start2 == end2 && start1 != end1) return OptExp();
	Real start = start1 < start2 ? start2 : start1; 
	Real end  =  end1   > end2   ? end2   : end1; 
	if (start > end) return OptExp();
	Expression s = start;
	Expression e = end;
	if (start == end) return OptExp(s);
	Range r = Range(s,e);
	return OptExp(r);	
}


/* Proyecta un intervalo dado sobre el subindice del Vertice 
 * Subindice i + 1  |  i
 * Intervalo [2:3]  |  5
 * Return [3:4]     |  5
 * */ 
OptExp Connectors::proyInter(Expression inter,Edge e)
{
	if (isEneUno(e)) {
		constExp ce = constExp();
		Expression i1 = getIndex(e).get();
		/*if (apply(ce,i1)) */ return applyIntervalo(getIntervalo(e).get(),getIndex(e));
	} 
	return applyIntervalo(inter,getIndex(e));
}	


/* Proyecta un intervalo dado sobre el subindice del Vertice 
 * Subindice i + 1  |  i
 * Intervalo [2:3]  |  5
 * Return [1:2]     |  5
 * */ 

 
OptExp Connectors::proyInterInv(Expression inter,Edge e)
{
	OptExp index = getIndex(e);
	if (index) 
		if (is<BinOp>(index.get())) {
			BinOp b = boost::get<BinOp>(index.get());
			if (b.op() == Add) b.op_ref() = Sub;
			else b.op_ref() = Add;
			Expression ex = b;
			index = ex;
		}
	return applyIntervalo(inter,index);
}
	
OptExp Connectors::applyIntervalo(Expression inter,OptExp index)	
{
	if (index && is<Range>(inter)) {
		Range r = boost::get<Range>(inter);
		EvalExp val = EvalExp(class_.syms_ref());
		EvalExp sval = EvalExp(class_.syms_ref(),"i",apply(val,r.start_ref()));
		EvalExp eval = EvalExp(class_.syms_ref(),"i",apply(val,r.end_ref()));
		Expression start = apply(sval,index.get());
		Expression end = apply(eval,index.get());
		if (start == end) return OptExp(start);
		Range result = Range(start,end);
		return OptExp(result);
	} else if (index) {
		EvalExp val = EvalExp(class_.syms_ref());
		EvalExp sval = EvalExp(class_.syms_ref(),"i",apply(val,inter));
		Expression start = apply(sval,index.get());
		return OptExp(start);
	}
	return index;
}

/* reemplaza una variable indice por una expression 
 * Indice 		i + 1
 * Expression  	i + 5
 * Return 		(i + 5) + 1 
 */
 
Expression Connectors::applyIndex(Expression index,Expression e)
{
	Modelica::replace r = Modelica::replace(NameToRef("i"),index);
	return apply(r,e);
}

/* reemplaza una variable indice por una expression. Antes invierte el indice 
 * Indice 		i + 1
 * Expression  	i + 5
 * Return 		(i + 5) -1 
 */
Expression Connectors::applyIndexInv(Expression index,Expression e)
{
	if (is<BinOp>(index)) {
		BinOp b = boost::get<BinOp>(index);
		if (b.op() == Add) b.op_ref() = Sub;
		else b.op_ref() = Add;
		index = b;
	}
	Modelica::replace r = Modelica::replace(NameToRef("i"),index);
	return apply(r,e);
}


/* Proyecta una solucion. Aplica proyecciondes de intervalos e indices
 *  segun la artista proporcionada
 */
SolList Connectors::proySolutions(SolList sols, Edge e)
{
	SolList solutions;
	OptExp index = getIndex(e);
	foreach_(Solution s, sols) {
		if (!get<2>(s)) {
			OptExp iter; if (get<0>(s)) iter = proyInter(get<0>(s).get(),e);
			VarsList  vls;
			foreach_(Vars var, get<1>(s)) {
				OptExp i = get<1>(var);
				if (i && index) i = applyIndexInv(index.get(),i.get());
				vls.push_back(Vars( get<0>(var) , i , get<2>(var) ));
			}
			solutions.push_back(Solution(iter,vls,get<2>(s)));
		} else
			solutions.push_back(s);
	}
	return solutions;
}

/* Proyecta una solucion INVERTIDA. Aplica proyecciondes de intervalos e indices
 *  segun la artista proporcionada
 */
SolList Connectors::proySolutionsInv(SolList sols, Edge e)
{
	SolList solutions;
	OptExp index = getIndex(e);
	foreach_(Solution s, sols) {
		if (!get<2>(s)) {
			OptExp iter; if (get<0>(s)) iter = proyInterInv(get<0>(s).get(),e);
			VarsList  vls;
			foreach_(Vars var, get<1>(s)) {
				OptExp i = get<1>(var);
				if (i && index) i = applyIndex(index.get(),i.get());
				vls.push_back(Vars( get<0>(var) , i , get<2>(var)));
			}
			solutions.push_back(Solution(iter,vls,get<2>(s)));
		} else 
			solutions.push_back(s);
	}
	return solutions;
}

/*
 *  Diferencia de intervalos: a -b
 * 
 */ 

OptExpList Connectors::cutRange(OptExp a,OptExp b) 
{
	if (!a) return OptExpList();
	if (is<Range>(a.get())) {
		Range r = boost::get<Range>(a.get());
		EvalExp val = EvalExp(class_.syms_ref());
		Real start = apply(val,r.start_ref());
		Real end = apply(val,r.end_ref());
		if (is<Range>(b.get())) {
			Range r2 = boost::get<Range>(b.get());
			Real i_start = apply(val,r2.start_ref());
			Real i_end = apply(val,r2.end_ref());
			if (start == i_start && i_end == end) { return OptExpList();}
			else if (start >= i_start && end > i_end) {
				Expression fstart = i_end + 1;
				Expression fend = end;
				Expression aux = Range(fstart,fend);
				return OptExpList(1,OptExp(aux));;
			} else if (start < i_start && end <= i_end) {
				Expression fstart = start;
				Expression fend = i_start - 1;
				Expression aux = Range(fstart,fend);
				return OptExpList(1,OptExp(aux));;
			} else if (start < i_start && end > i_end) {
				OptExpList res;
				Expression fstart = start;
				Expression fend = i_start - 1;
				Expression a = Range(fstart,fend); 
				res.push_back(a);
				fstart = i_end + 1;
				fend = end;
				a = Range(fstart,fend);
				res.push_back(a);
				return res;
			} else return OptExpList(1,a);
		} else {
			Real num = apply(val,b.get());
			if (start == num && num == end) {
				return OptExpList();
			} else if (start == num) {
				Expression fstart = start + 1;
				Expression fend = end;
				Expression aux = Range(fstart,fend);
				return OptExpList(1,OptExp(aux));
			} else if (end == num) { 
				Expression fstart = start;
				Expression fend = end - 1;
				Expression aux = Range(fstart,fend);
				return OptExpList(1,OptExp(aux));
			} else if (start < num && num < end) {
				OptExpList res;
				Expression fstart = start;
				Expression fend = num - 1;
				Expression a = Range(fstart,fend); 
				res.push_back(a);
				fstart = num + 1;
				fend = end;
				a = Range(fstart,fend);
				res.push_back(a);
				return res;
			} else return OptExpList(1,a);
		}
	} return OptExpList();	
}

/* Corta una arista
 * 
 */
 
void Connectors::cutEdge(OptEdge oe, OptExp iter_)
{
	markUsed(getConnect(oe.get()),iter_);
	if (!iter_ && oe) { 
		markVisited(getConnect(oe.get()));
	} else if (iter_) {
		OptExp i = getRange(oe.get());
		OptExpList r = cutRange(i,iter_); 
		if (r.size() == 1) 
			G[getConnect(oe.get())].range = r.front();
		else if (r.size() > 1) {
			markVisited(getConnect(oe.get()));
			foreach_(OptExp rr, r) {
				Edge e = oe.get();
				Vertex s = getVertex(e);
				Vertex t = getSinkVertex(e);
				Vertex c = createConnect(rr);
				createEdge(s,c,getIndex(e));
				createEdge(t,c,getIndex(getSinkEdge(e)));
			}				
		} else {
			markVisited(getConnect(oe.get()));
		}		
	}
}
/* Marca un nodo conect como finalizado, es decir las dos aristas 
 */
void Connectors::markVisited(Vertex c)
{
	MMOGraphOutputEdgeIteretor i,end;
	for (tie(i, end) = out_edges(c,G); i != end; ++i)
		G[*i].visited = true;
	G[c].range = OptExp(); 
}

/* Marca un nodo conect como finalizado, es decir las dos aristas 
 */
void Connectors::markUsed(Vertex c,OptExp iter_)
{
	MMOGraphOutputEdgeIteretor i,end;
	for (tie(i, end) = out_edges(c,G); i != end; ++i) {
		Vertex v = getVertex(*i);
		OptExp in = proyInter(iter_.get(),*i);
		OptExpList temp;
		foreach_(OptExp f, G[v].free) temp += cutRange(f,in); 
		G[v].free = temp;
	}
}

/* Funcion principal para resolver el problema
 * 
 */


SolList Connectors::VisitarVertice(Vertex v,OptEdge oe,OptExp iter_,bool eneuno)
{
	SolList sfinal;
	Vars varsV = Vars(G[v].name.get(),OptExp(NameToRef("i")),iter_);  // Solucion basica 
	

	/* Proyecto el intervalo segun la arista entrante */
	OptExp iter = iter_ ? proyInter(iter_.get(),oe.get()) : OptExp() ; 
	OptExpList aux_inter = OptExpList(1,iter); 

	//cerr << "Visito el nodo " << G[v].name.get() << " con " << PrintOpt(iter_) << " proy " << PrintOpt(iter)  << endl ; 

	
	if (oe) {
		if (isEneUno(oe.get())) cutEdge(oe,iter);
		else cutEdge(oe,iter_);	
	}
	OptExp I; EdgeSet U;
	do {
		// Interseccion de todos los intervalos.
		//cerr << "por acaestoy girando" << endl;
		MMOGraphOutputEdgeIteretor i,end;
		bool eneaux = eneuno;
		U.clear();
		I = ( aux_inter.size()> 0 ? aux_inter.front(): OptExp() );
		for (tie(i, end) = out_edges(v,G); i != end; ++i) {
			if (!I) I = !oe ? getIntervalo(*i) : OptExp();	
			OptExp aux = intersectionIntervals(I,getIntervalo(*i));
			if (aux) {
				eneaux |= isEneUno(*i);
				U.insert(*i);
				I = aux;	
			}	
		}
		/* Si hay interseccion */		
		if (I && U.size() > 0) { 
			std::vector<SolList> sols;
			/* Calculo soluciones: Tengo que cambiar proyecciones y demás */
			foreach_(Edge e1,U) {
				Vertex t = getSinkVertex(e1); 
				SolList ss = VisitarVertice(t, OptEdge(getSinkEdge(e1)) , proyInterInv(I.get(),e1),eneaux);
				sols.push_back(proySolutions(ss,e1)); // Proyectar segun e1 en intervalo e indices
			}
						
			SolList _final_f;
			combine(sols,Option<Solution>(),_final_f); // Combino las soluciones de diferentes nodos. 

			// Agrego (v,i) como solucion
			SolList _final;
			foreach_(Solution s,_final_f) {
				VarsList vv = get<1>(s); 
				if (is<Range>(I.get())) vv.push_back(Vars(G[v].name.get(),OptExp(NameToRef("i")),I));
				else vv.push_back(Vars(G[v].name.get(),I,I));   
				_final.push_back(Solution(get<0>(s), vv, eneaux || get<2>(s) ));
			}	 
			
			// Si vengo desde una arista proyecto todo
			if (oe) _final = proySolutionsInv(_final,oe.get());
			sfinal += _final;
	
			/* Corto los intervalos */
			OptExpList temp;
			if (oe) I = proyInterInv(I.get(),oe.get());
			foreach_ (OptExp i, aux_inter) temp += cutRange(i,I); 
			aux_inter = temp;			
		}			
	} while (I && U.size() > 0);
	
	// Si queda parte del intervalo genero una solucion con el nodo solo
	if (aux_inter.size() > 0 && oe) {
		foreach_(OptExp au, aux_inter) {
			VarsList vv = VarsList(1,Vars(G[v].name.get(),OptExp(NameToRef("i")),au)); 
			SolList _stemp; _stemp.push_back(Solution(au, vv,eneuno));
			if (oe) _stemp = proySolutionsInv(_stemp,oe.get());
			sfinal += _stemp;
		}
	}

	return sfinal;	
}

void Connectors::combine(std::vector<SolList> sols, Option<Solution> temp, SolList & _final)
{
	if (sols.empty()) { _final.push_back(temp.get()); }
	else {
		SolList sol = sols.front();
		sols.erase(sols.begin()); 
		foreach_ (Solution s, sol) {
			Option<Solution> stemp = s;
			if (temp) {			
				Solution t = temp.get();
				OptExp iter = intersectionIntervals( get<0>(s), get<0>(t) );
				VarsList vars = get<1>(s) + get<1>(t);
				stemp = Solution(iter,vars, get<2>(s) || get<2>(t) );
			}
			combine(sols,stemp,_final);
		}
	}
	return;  
}

SolList Connectors::resolveConnect()
{
	SolList solutions;
	MMOGraphVertexIteretor vi , vi_end;
	for (tie(vi, vi_end) = vertices(G); vi != vi_end; ++vi) 
		if (G[*vi].name) 
			solutions += VisitarVertice(*vi,OptEdge(), OptExp(),false);
		

	
	//printSols(solutions);	
	return solutions;
	
}

/**
 ***********************************************************************
 * 
 * 		FUNCIONES PARA EXPANDIR LAS SOLUCIONES
 * 
 ***********************************************************************
 ** 
*/

Name Connectors::getName(Expression name)
{
	if (is<UnaryOp>(name))
		name = boost::get<UnaryOp>(name).exp();
	return get<0>(boost::get<Reference>(name).ref().front());	
}

bool Connectors::isArray(Expression name)
{
	if (is<UnaryOp>(name))
		name = boost::get<UnaryOp>(name).exp();
	Name n = get<0>(boost::get<Reference>(name).ref().front());	
	Option<VarInfo> vi = class_.syms_ref()[n];
	if (vi) {
		VarInfo v = vi.get();
		if (v.indices()) return true;
	}
	return false;
}


Expression Connectors::removeUnary(Expression name)
{
	if (is<UnaryOp>(name)) 
		return boost::get<UnaryOp>(name).exp();
	return name;			
}

Expression Connectors::addSufix(Expression name,Name var)
{
	if (var.empty()) return  name;	
	UnaryOpType op = Plus;
	bool isU = false;
	if (is<UnaryOp>(name)) {
		op = boost::get<UnaryOp>(name).op();
		name = boost::get<UnaryOp>(name).exp();		
		isU = true;
	}	
	Name n = get<0>(boost::get<Reference>(name).ref().front());
	ExpList index = get<1>(boost::get<Reference>(name).ref().front());
	n = n + "_" + var;	
	Expression e = RefIndex(n,index);
	if (isU) e = UnaryOp(e,op);
	return e;
}


Expression Connectors::putIndex(Vars v,Option<EvalExp> eval)
{
	Expression name = get<0>(v);
	OptExp i = get<1>(v);
	return addIndex(name,i,eval);
}	

Expression Connectors::addIndex(Expression name,OptExp i,Option<EvalExp> eval)
{
	UnaryOpType op = Plus;
	bool isU = false;
	if (is<UnaryOp>(name)) {
		op = boost::get<UnaryOp>(name).op();
		name = boost::get<UnaryOp>(name).exp();		
		isU = true;
	}
	Name n = get<0>(boost::get<Reference>(name).ref().front());
	Expression ii = i.get();
	ExpList l; l.push_back(ii);
	Expression e = RefIndex(n,l);
	if (eval) {		
		ExpList l; l.push_back(apply(eval.get(),ii));
		e = RefIndex(n, l);
	}	 
	if (isU) e = UnaryOp(e,op);
	return e;
}
 
 
Equation makeForEq(Range r, EquationList eqs)
{
	Index i = Index("i",OptExp(r));
	IndexList is; is.push_back(i);
	return ForEq(Indexes(is),eqs);
}
 
/* esta funcion crea la igualdad a cero de los conectores sin uso
 */

void Connectors::ceroEquations() 
{
	ClassFinder re = ClassFinder();
	foreach_(Name e, class_.variables()) {
		VarInfo v = class_.syms_ref()[e].get();
		OptTypeDefinition m = re.resolveType(class_,v.type());	
		if ( m ) {
			typeDefinition td = m.get();
			Type::Type t_final = get<1>(td);
			if ( is<Type::Class>(t_final)) {
				MMO_Class conector =  *(boost::get<Type::Class>(t_final).clase());	
				
				OptExpList list;
				OptVertex v1 = findVertex( UnaryOp(NameToRef(e),Plus) );
				if (v1) list += G[v1.get()].free;
				OptVertex v2 = findVertex( UnaryOp(NameToRef(e),Minus) );
				if (v2) list += G[v2.get()].free;
				
				if (!v1 && !v2 ) { // Si no estan en el grafo no se usaron en ningun connect
					OptExp temp;
					if (v.indices()) {
						Expression temp1 =  Range(1,v.indices().get().front());
						temp = temp1;
					}	
					list.push_back(temp);
				}
				
				foreach_(Name n , conector.variables()) {
					VarInfo i = conector.syms_ref()[n].get();
					if (i.isPrefix(flow)) {
						foreach_(OptExp r , list) {
							ExpList indices;
							
							if (r) indices = ExpList(1,NameToRef("i"));
							Expression t = RefIndex(e + "_" + n , indices);
							
							if (r && is<Range>(r.get())) {
								Range rr = get<Range>(r.get());
								EquationList eqs;
								Expression cero = 0;
								eqs.push_back(Equality(t,cero));
								class_.addEquation(makeForEq(rr,eqs));	
							} else {
								Expression cero = 0;
								class_.addEquation(Equality(t,cero));	
							}
						}
					}
				}	
			}	
		}
	}
}


void Connectors::createEquation(Solution sol)
{

	//static int i = 0;
	ClassFinder re = ClassFinder();
	OptExp iter = get<0>(sol);
	VarsList vars = get<1>(sol);
	bool multiNode = get<2>(sol);
	
	Expression name = get<0>(vars.front());
	if (is<UnaryOp>(name))
		name = boost::get<UnaryOp>(name).exp();
	
	Name n = get<0>(boost::get<Reference>(name).ref().front());
	Option<VarInfo> vi = class_.syms_ref()[n];
	if (vi) {
		VarInfo v = vi.get();		
		OptTypeDefinition m = re.resolveType(class_,v.type());	
		if (m) {
			typeDefinition td = m.get();
			Type::Type t_final = get<1>(td);
			if ( is<Type::Class>(t_final)) {
				/* Obtengo el tipo del conector */
				MMO_Class conector =  *(boost::get<Type::Class>(t_final).clase());	
				foreach_(Name e , conector.variables()) {
					VarInfo i = conector.syms_ref()[e].get();
					if (i.isPrefix(flow)) 
						createFlowEquation(iter,vars,conector,e,multiNode);
					else
						createEqualityEquation(iter,vars,e,multiNode);
				}
			} else {
				createEqualityEquation(iter,vars,"",multiNode);
			}
		}
	}	
}

void Connectors::createFlowEquation(OptExp iter,VarsList vars,MMO_Class & conector,Name e,bool multiNode)
{
//	if (!iter) return;
	if (!multiNode) {
	Vars v = vars.front();
	vars.erase(vars.begin());		
		if (is<Range>(iter.get())) {
			EquationList eqs;
			Range r = boost::get<Range>(iter.get());
			Expression eq1 = get<0>(v);
			if (isArray(get<0>(v))) 
				eq1 = putIndex(v,Option<EvalExp>());
			eq1 = addSufix(eq1,e);	
			foreach_(Vars ve,vars) {
				Expression eq2 = get<0>(ve);
				if (isArray(get<0>(ve))) 
					eq2 = putIndex(ve,Option<EvalExp>());
				eq2 = addSufix(eq2,e);	
				eq1 = BinOp(eq1,Add,eq2);
			}
			Expression cero = 0;
			eqs.push_back(Equality(eq1,cero));
			class_.addEquation(makeForEq(r,eqs));	
		} else {
			EvalExp eval = EvalExp(class_.syms_ref());
			Real i = apply(eval,iter.get());
			EvalExp ieval = EvalExp(class_.syms_ref(),"i",i);
			
			EquationList eqs;
			Expression eq1 = get<0>(v);
			if (isArray(get<0>(v))) 
				eq1 = putIndex(v,ieval);
			eq1 = addSufix(eq1,e);	
			foreach_(Vars ve,vars) {
				Expression eq2 = get<0>(ve);
				if (isArray(get<0>(ve))) 
					eq2 = putIndex(ve,ieval);
				eq2 = addSufix(eq2,e);	
				eq1 = BinOp(eq1,Add,eq2);	
			}
			Expression cero = 0;
			class_.addEquation(Equality(eq1,cero));
		}
	} else {
		OptExp eq1;
		foreach_(Vars v,vars) {
			if (is<Range>(get<2>(v).get())) {
				Range range = boost::get<Range>(get<2>(v).get());
				Expression t = get<0>(v);
                               	t = addIndex(t,get<2>(v), Option<EvalExp>());
				t = Call("sum",addSufix(t,e));
				if (!eq1) eq1 = t;
				else {
					Expression temp = BinOp(eq1.get(),Add,t);
					eq1 = temp;
				}

				//EvalExp eval = EvalExp(class_.syms_ref());
				//Real start = apply(eval,range.start_ref());
				//Real end = apply(eval,range.end_ref());
				//for (Real i = start; i <= end;i++) {
				//	EvalExp ieval = EvalExp(class_.syms_ref(),"i",i);
				//	Expression t = get<0>(v);
				//	if (isArray(get<0>(v))) 
				//		t = putIndex(v,ieval);
				//	t = addSufix(removeUnary(t),e);	
				//	if (!eq1) eq1 = t;
				//	else {
				//		Expression temp = BinOp(eq1.get(),Add,t);
				//		eq1 = temp;
				//	}
					
				//}
			} else {
				EvalExp eval = EvalExp(class_.syms_ref());
				Real i = apply(eval,get<2>(v).get());
				EvalExp ieval = EvalExp(class_.syms_ref(),"i",i);
				Expression t = get<0>(v);
				if (isArray(get<0>(v))) 
					t = putIndex(v,ieval);
				t = addSufix(removeUnary(t),e);	
				if (!eq1) eq1 = t;
				else {
					Expression temp = BinOp(eq1.get(),Add,t);
					eq1 = temp;
				}
				
			}
		}
		Expression cero = 0;
		class_.addEquation(Equality(eq1.get(),cero));
	}
}

void Connectors::createEqualityEquation(OptExp iter,VarsList vars , Name e,bool multiNode)
{
//	if (!iter) return;
	if (!multiNode) {
		Vars v = vars.front();
		vars.erase(vars.begin());
		if (is<Range>(iter.get())) {
			EquationList eqs;
			Range r = boost::get<Range>(iter.get());
			Expression eq1 = get<0>(v);
			if (isArray(get<0>(v))) 
				eq1 = putIndex(v,Option<EvalExp>());
			eq1 = addSufix(removeUnary(eq1),e);	
			foreach_(Vars ve,vars) {
				Expression eq2 = get<0>(ve);
				if (isArray(get<0>(ve))) 
					eq2 = putIndex(ve,Option<EvalExp>());
				eq2 = addSufix(removeUnary(eq2),e);	
				eqs.push_back(Equality(eq1,eq2));
			}
			class_.addEquation(makeForEq(r,eqs));	
		} else {
			EvalExp eval = EvalExp(class_.syms_ref());
			Real i = apply(eval,iter.get());
			EvalExp ieval = EvalExp(class_.syms_ref(),"i",i);
			EquationList eqs;
			Expression eq1 = get<0>(v);
			if (isArray(get<0>(v))) 
				eq1 = putIndex(v,ieval);
			eq1 = addSufix(removeUnary(eq1),e);	
			foreach_(Vars ve,vars) {
				Expression eq2 = get<0>(ve);
				if (isArray(get<0>(ve))) 
					eq2 = putIndex(ve,ieval);
				eq2 = addSufix(removeUnary(eq2),e);	
				class_.addEquation(Equality(eq1,eq2));
			}
		}
	} else {
		OptExp eq1;
		foreach_(Vars v,vars) {
			if (is<Range>(get<2>(v).get())) {
				Range range = boost::get<Range>(get<2>(v).get());
				EvalExp eval = EvalExp(class_.syms_ref());
				Real start = apply(eval,range.start_ref());
				Real end = apply(eval,range.end_ref());
				Expression istart = start + 1;
				Expression iend = end;
				EvalExp ieval = EvalExp(class_.syms_ref(),"i",start);
				if (!eq1) {
					EquationList eqs;					
					Range r2 = Range(istart , iend );
					eq1 = get<0>(v);
                                        if (isArray(get<0>(v))) 
                                                eq1 = putIndex(v,ieval);
                                        eq1 = addSufix(removeUnary(eq1.get()),e);     

					Expression eq2 = get<0>(v);
                                        if (isArray(get<0>(v))) 
                                                eq2 = putIndex(v,Option<EvalExp>());
                                        eq2 = addSufix(removeUnary(eq2),e);     
                                        eqs.push_back(Equality(eq1.get(),eq2));
                                        class_.addEquation(makeForEq(r2,eqs)); 
				} else {
					EquationList eqs;
					Expression eq2 = get<0>(v);
	                                if (isArray(get<0>(v))) 
        	                                eq2 = putIndex(v,Option<EvalExp>());
                	                eq2 = addSufix(removeUnary(eq2),e);     
                        	        eqs.push_back(Equality(eq1.get(),eq2));
					class_.addEquation(makeForEq(range,eqs)); 
				}
				/*for (Real i = start; i <= end;i++) {
					EvalExp ieval = EvalExp(class_.syms_ref(),"i",i);
					Expression t = get<0>(v);
					if (isArray(get<0>(v))) 
						t = putIndex(v,ieval);
					t = addSufix(removeUnary(t),e);	
					if (!eq1) eq1 = t;
					else 
						class_.addEquation(Equality(eq1.get(),t));											
				}*/
			} else {
				EvalExp eval = EvalExp(class_.syms_ref());
				Real i = apply(eval,get<2>(v).get());
				EvalExp ieval = EvalExp(class_.syms_ref(),"i",i);
				Expression t = get<0>(v);
				if (isArray(get<0>(v))) 
					t = putIndex(v,ieval);
				t = addSufix(removeUnary(t),e);	
				if (!eq1) eq1 = t;
				else 
					class_.addEquation(Equality(eq1.get(),t));	
			}
		}
		
	}	
}




/*
template<typename Set> std::set<Set> powerset(const Set& s, size_t n)
{
    typedef typename Set::const_iterator SetCIt;
    typedef typename std::set<Set>::const_iterator PowerSetCIt;
    std::set<Set> res;
    if(n > 0) {
        std::set<Set> ps = powerset(s, n-1);
        for(PowerSetCIt ss = ps.begin(); ss != ps.end(); ss++)
            for(SetCIt el = s.begin(); el != s.end(); el++) {
                Set subset(*ss);
                subset.insert(*el);
                res.insert(subset);
            }
        res.insert(ps.begin(), ps.end());
    } else
        res.insert(Set());
    return res;
}
template<typename Set> std::set<Set> powerset(const Set& s)
{
    return powerset(s, s.size());
}

bool setSort (EdgeSet i,EdgeSet j) { return (i.size() > j.size()); }
*/
