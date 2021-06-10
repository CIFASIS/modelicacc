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

#include <iostream>
#include <string>

#include <flatter/connectors.h>
#include <util/ast_visitors/contains_expression.h>
#include <util/ast_visitors/eval_expression.h>
#include <util/ast_visitors/eval_expression_flatter.h>
#include <util/logger.h>

using namespace std;
using namespace Modelica;
using namespace Modelica::AST;
using namespace SBG;

// Initialization --------------------------------------------------------------------------------

member_imp(Connectors, MMO_Class, mmoclass);

member_imp(Connectors, SBGraph, G)
member_imp(Connectors, int, maxdim)
member_imp(Connectors, vector<int>, vCount);
member_imp(Connectors, vector<int>, eCount);
member_imp(Connectors, int, ECount);
member_imp(Connectors, PWLMap, ccG);

member_imp(Connectors, EquationList, notConnectEqs);
member_imp(Connectors, EquationList::iterator, itNotConn);

member_imp(Connectors, UnordCT<Name>, varsNms);
member_imp(Connectors, OrdCT<Name>, counters);

Connectors::Connectors(MMO_Class &c) : mmoclass_(c), ECount_(0), maxdim_(1) {}

void Connectors::init() 
{
  // Get "largest" number of dimensions from variables
  foreach_ (Name n, mmoclass_.variables()) {
    Option<VarInfo> ovi = mmoclass_.getVar(n);
    if (ovi) {
      VarInfo vi = *ovi;
      Option<ExpList> oinds = vi.indices();
      if (oinds) {
        ExpList inds = *oinds;
        set_maxdim(max(maxdim_, (NI1) inds.size()));
      }
    }
  }  

  vector<NI1> maxdimVector(maxdim_, 1);
  set_vCount(maxdimVector);
  set_eCount(maxdimVector);

  // Intialize iterators
  set_itNotConn(notConnectEqs_.begin());

  // Fill varsNms
  std::pair<Name, VarInfo> aux;
  UnordCT<Name> vars;

  foreach_ (aux, mmoclass_.syms()) 
    vars.insert(get<0>(aux));

  set_varsNms(vars);
}

// Debugging -------------------------------------------------------------------------------------

void Connectors::debug(std::string filename)
{
  LOG << "Connect Graph vertices & edges:" << endl << endl;

  // Print vertices
  foreach_ (SetVertexDesc vi, vertices(G_)) {
    Name n = G_[vi].name;
    Set vs = G_[vi].vs_();
    LOG << n << ": " << vs << endl;
  }

  // Print edges
  foreach_ (SetEdgeDesc ei, edges(G_)) {
    Name n = G_[ei].name;
    PWLMap es1 = G_[ei].es1_();
    PWLMap es2 = G_[ei].es2_();
    LOG << n << ": " << es1 << ", " << es2 << endl;
  }

  LOG << endl;

  // Write debugging information to log file
  GraphPrinter gp(G_, -1);

  gp.printGraph(filename);
  LOG << "Generated Connect Graph written to " << filename << endl << endl;

  LOG << "Connected components of SBG: " << filename << endl; 
  LOG << ccG_ << endl;
}

// Algorithm -------------------------------------------------------------------------------------

// Given a model, construct the Connect Graph, apply the connected components algorithms, and
// return a new flattenned model
void Connectors::solve()
{
  init();

  Pair<bool, EquationList> gr = createGraph(mmoclass_.equations_ref().equations_ref());
  if(get<0>(gr)){
    notConnectEqs_ = get<1>(gr);
    itNotConn_ = notConnectEqs_.end();

    PWLMap res = connectedComponents(G_);
    set_ccG(res);

    foreach_ (Name nm, mmoclass_.variables()) {
      Option<VarInfo> ovi = mmoclass_.getVar(nm);
      if (ovi) {
        VarInfo vi = *ovi;
        Name ty = vi.type();
        if (ty != "Real" && ty != "Integer") {
          mmoclass_.rmVar(nm);
        }
      }
    }

    //generateCode(ccG_);

    cout << mmoclass_ << "\n";
  }

  else {
    SBGraph emptyG;
    set_G(emptyG);

    MMO_Class emptyMmo;
    set_mmoclass(emptyMmo);
  }
}

// Set-vertex creation ---------------------------------------------------------------------------

// Evaluate expression
Real Connectors::getValue(Expression exp)
{
  VarSymbolTable symbols = mmoclass_.syms();
  EvalExpression eval_exp(symbols);
  return Apply(eval_exp, exp);
}

// Helper to write less
Set Connectors::buildSet(MultiInterval mi)
{
  AtomSet as(mi);
  Set s;
  s.addAtomSet(as);

  return s;
}

// Build a Set from a variable. All variables are updated to 
// have the maximum number of dimensions according to the model.
// To be used in createVertex
Set Connectors::buildSet(VarInfo v)
{
  vector<NI1> newVCount;
  vector<NI1>::iterator itNewVC = newVCount.begin();

  Option<ExpList> dims = v.indices();
  MultiInterval v_intervals;
  ExpList inds; 
  ExpList::iterator itinds = inds.begin();

  // Multi-dimensional variable
  if (dims) {
    inds = *dims;
    itinds = inds.begin();
  }

  const VarSymbolTable auxsyms = mmoclass_.syms();
  EvalExpFlatter evexp(auxsyms);

  // Fill dimensions if necessary
  foreach_ (NI1 offset, vCount_) {
    Interval i;

    // Use declared dimensions for the variable
    if (itinds != inds.end()) {
      Interval auxi = Apply(evexp, *itinds);
      i = Interval(offset, auxi.step_(), offset + auxi.lo_() - 1);

      ++itinds;
    }

    // Fill other dimensions
    else
      i = Interval(offset, 1, offset);

    v_intervals.addInter(i);

    itNewVC = newVCount.insert(itNewVC, i.hi_() + 1);
    ++itNewVC;
  }

  set_vCount(newVCount);
  return buildSet(v_intervals);
}

// Create a set vertex for each variable in a connect,
// if it isn't already created. Variables which aren't
// part of a connect are ignored.
Option<SetVertexDesc> Connectors::createVertex(Name n)
{
  // Vertex already created
  foreach_ (SetVertexDesc Vdesc, vertices(G_)) {
    if (G_[Vdesc].name == n)
      return Option<SetVertexDesc>(Vdesc);
  }
 
  // Vertex doesn't exist 
  Option<VarInfo> ovi = mmoclass_.getVar(n);
  if (ovi) {
    VarInfo vi = *ovi;
    Set s = buildSet(vi);

    SetVertex V(n, s); 
    SetVertexDesc Vdesc = boost::add_vertex(G_);
    G_[Vdesc] = V;

    return Option<SetVertexDesc>(Vdesc);
  }

  LOG << "ERROR: Variable " << n << " should be defined" << endl;
  return Option<SetVertexDesc>();
}

// Set-edge creation -----------------------------------------------------------------------------

// Build a Set from a variable. All counters will have the maximum dimension.
// To be used in createEdgeDom
MultiInterval Connectors::buildEdgeMultiInterval(VarInfo v, int offset)
{
  vector<NI1>::iterator itEC = eCount_.begin();
  vector<NI1> newECount;
  vector<NI1>::iterator itNewEC = newECount.begin();

  Option<ExpList> dims = v.indices();
  MultiInterval v_intervals;
  ExpList inds; 
  ExpList::iterator itinds = inds.begin();

  // Previous dimensions remain the same
  for (int i = 0; i < offset; ++i) {
    itNewEC = newECount.insert(itNewEC, *itEC);

    ++itEC;
    ++itNewEC;
  }

  // Multi-dimensional variable
  if (dims) {
    inds = *dims;
    itinds = inds.begin();
  }

  const VarSymbolTable auxsyms = mmoclass_.syms();
  EvalExpFlatter evexp(auxsyms);

  // Values of dimensions of interest
  foreach_ (Expression e, inds) {
    Interval i;
    Interval auxi = Apply(evexp, *itinds);
    i = Interval(*itEC + auxi.lo_() - 1, auxi.step_(), *itEC + auxi.hi_() - 1);

    v_intervals.addInter(i);

    itNewEC = newECount.insert(itNewEC, i.hi_() + 1);
    ++itNewEC;
    if (itEC != eCount_.end())
      ++itEC;
  }

  // Convert it to maximum dimension
  while (itEC != eCount_.end()) {
    itNewEC = newECount.insert(itNewEC, *itEC);

    ++itNewEC;
    ++itEC;
  }

  set_eCount(newECount);
  return v_intervals;
}

// Given a subscript list r, traverse it to find in which
// dimension nm is used.
int Connectors::locateCounterDimension(ExpOptList r, Name nm) 
{
  int dim = 0;

  if (r) {
    foreach_ (Expression ri, *r) {
      Reference r(nm);
      Expression e(r);
      ContainsExpression co(e);

      if (Apply(co, ri))
        return dim;

      ++dim;
    }
  }

  return -1;
}

// Given a multi-interval whose dimension isn't the maxdim_,
// complete it to have the maximum dimension in the model
MultiInterval Connectors::fillDims(MultiInterval mi, int olddim, int dim) 
{
  vector<NI1> newECount;
  vector<NI1>::iterator itNewEC = newECount.begin();
  int i = 0;
  foreach_ (NI1 ei, eCount_) {
    if (i < olddim || i > dim) {
      Interval idim(ei, 1, ei);
      MultiInterval midim;
      midim.addInter(idim); 

      if (i < olddim)
        mi = midim.crossProd(mi);

      else
        mi = mi.crossProd(midim);

      itNewEC = newECount.insert(itNewEC, ei + 1);
      ++itNewEC;
    }

    else {
      itNewEC = newECount.insert(itNewEC, ei);
      ++itNewEC;
    }

    ++i;
  }

  set_eCount(newECount);
  return mi;
}

// Create dom for PWLMaps that represent set-edges.
// It creates a new Set which values are the ones
// of the current counters
// ENHANCEMENT: simplify this code, lulz
Set Connectors::createEdgeDom(ExpOptList r) 
{
  MultiInterval miCounters;


  // Connect out of any loop
  if (counters_.size() == 0) {
    vector<NI1> newECount;
    vector<NI1>::iterator itNewEC = newECount.begin();

    foreach_ (NI1 eci, eCount_) {
      miCounters.addInter(Interval(eci, 1, eci));
      itNewEC = newECount.insert(itNewEC, eci + 1);

      ++itNewEC;
    }

    set_eCount(newECount);
  }

  else {
    // Connect is inside some loop
    int olddim = locateCounterDimension(r, *(counters_.begin()));
    foreach_ (Name count, counters_) {
      int dim = locateCounterDimension(r, count);
      // Variable is declared in the loop, but it isn't used in the connect
      // This is considered an error in this compiler
      if (dim < 0) {
        MultiInterval emptyMI;
        miCounters = emptyMI;
        break;
      }

      else {
        Option<VarInfo> ovi = mmoclass_.syms()[count];
        if (ovi) {
          VarInfo vi = *ovi;
          MultiInterval mi = buildEdgeMultiInterval(vi, dim);
          miCounters = miCounters.crossProd(mi);
        }

        // At this point the counters should be defined
        else {
          LOG << "MCC ERROR: Counter " << count << " should be defined" << endl;
          Set emptySet;
          return emptySet;
        }
      }

      // Complete remaining dimensions
      if ((int) counters_.size() != maxdim_)
        miCounters = fillDims(miCounters, olddim, dim);

      olddim = dim;
    }
  }

  return buildSet(miCounters);
}

// Returns a linear map lm, for which im(lm, i1) = i2
// In each dimension, they should have the same number of elements
// (if not, they aren't related by a linear function)
// ENHANCEMENT: support discontinuidades
LMap Connectors::createLM(MultiInterval mi1, MultiInterval mi2)
{
  OrdCT<NI2> resg;
  OrdCT<NI2>::iterator itresg = resg.begin();
  OrdCT<NI2> reso;
  OrdCT<NI2>::iterator itreso = reso.begin();

  OrdCT<Interval> miinters1 = mi1.inters_();
  OrdCT<Interval>::iterator itmi1 = miinters1.begin();
  OrdCT<Interval> miinters2 = mi2.inters_();
  OrdCT<Interval>::iterator itmi2 = miinters2.begin();
  // Traverse dimensions
  while (itmi1 != miinters1.end()) {
    if ((*itmi2).size() == 1) {
      NI2 newg = 0;
      NI2 newo = (*itmi2).lo_(); 

      itresg = resg.insert(itresg, newg);
      itreso = reso.insert(itreso, newo);

      ++itresg;
      ++itreso;
    }

    else if ((*itmi1).size() == (*itmi2).size()) {
      NI2 newg = (*itmi2).step_() / (*itmi1).step_();
      NI2 newo = (*itmi2).lo_() - newg * (*itmi1).lo_(); 

      itresg = resg.insert(itresg, newg);
      itreso = reso.insert(itreso, newo);

      ++itresg;
      ++itreso;
    }

    else {
      LOG << "ERROR: Check connect subscripts" << endl;
      LMap emptyLM;
      return emptyLM;
    }

    ++itmi1;
    ++itmi2;
  }

  return LMap(resg, reso);
}

// Returns a multi-interval with the corresponding subscripts
// applied
MultiInterval Connectors::subscriptMI(MultiInterval mi, ExpOptList r) 
{
  OrdCT<Interval> mires;
  OrdCT<Interval>::iterator itmires = mires.begin();
  OrdCT<Interval> miinters = mi.inters_();
  OrdCT<Interval>::iterator itmi = miinters.begin();

  const VarSymbolTable auxsyms = mmoclass_.syms();
  EvalExpFlatter evexp(auxsyms);

  if (r) {
    foreach_ (Expression ri, *r) {
      Interval ndim = Apply(evexp, ri);
      int offset = (*itmi).lo_();
      Interval res(ndim.lo_() + offset - 1, ndim.step_(), ndim.hi_() + offset - 1);
      itmires = mires.insert(itmires, (*itmi).cap(res));

      ++itmires;
      ++itmi;
    }
  }

  else
    mires = miinters;

  //MultiInterval aux(mires);
  //cout << "aux: " << aux << "\n";
  return MultiInterval(mires);
}

// Create left and right maps
PWLMap Connectors::createEdgeMap(Set dom, Set im, ExpOptList r) 
{
  PWLMap res;

  // In the current implementation, dom and im have only one atomic set
  // Check buildSet and createEdgeDom
  if (dom.asets_().size() != 1 || im.asets_().size() != 1) {
    LOG << "COMPILER ERROR: dom and im should have only one atomic set" << endl;
    return res;
  }

  else {
    AtomSet domas = *(dom.asets_().begin());
    AtomSet imas = *(im.asets_().begin());

    MultiInterval dommi = domas.aset_();
    MultiInterval immi = imas.aset_();

    LMap lm = createLM(dommi, subscriptMI(immi, r));
    res.addSetLM(dom, lm);
  }

  return res;
}

// Check if edge is already created
bool Connectors::existsEdge(string nm)
{
  foreach_ (SetEdgeDesc ei, edges(G_)) {
    string nmi = G_[ei].name_();

    if (nmi == nm)
      return true;
  }

  return false;
}

// Create a set-edge if it doesn't exist, or update it
void Connectors::createEdge(ExpOptList r1, ExpOptList r2, SetVertexDesc Vdesc1, SetVertexDesc Vdesc2) 
{
  SetVertex V1 = G_[Vdesc1];
  SetVertex V2 = G_[Vdesc2];

  string nm = "E_" + V1.name_() + "_" + V2.name_();

  if (!existsEdge(nm)) {
    Set dom = createEdgeDom(r1);
    if (dom.empty()) {
      LOG << "ERROR: Check connects" << endl;
      return;
    }
    PWLMap pw1 = createEdgeMap(dom, V1.vs_(), r1);
    PWLMap pw2 = createEdgeMap(dom, V2.vs_(), r2);
    SetEdge E(nm, pw1, pw2);
    cout << V1 << V2 << E << "\n";

    /*
    SetEdgeDesc Edesc;
    bool b;
    boost::tie(Edesc, b) = boost::add_edge(Vdesc1, Vdesc2, G_);
    G_[Edesc] = E;
    */
  }

  else {
    //TODO
  }
}

// Subscripts checks -----------------------------------------------------------------------------

bool Connectors::checkLinearBase(Expression e)
{
  if (is<Integer>(e)) 
    return true;

  else if (is<Reference>(e))
    return true;

  return false;
}

// Check if a given expression is linear
// For the time being only a * x + b is allowed
// e.g. a * x + b - c is rejected
// ENHANCEMENT: generalize to arbitrary linear expressions
bool Connectors::checkLinear(Expression e) 
{
  if (is<UnaryOp>(e)) {
    UnaryOp u = boost::get<UnaryOp>(e);
    return checkLinearBase(u.exp());
  }

  else if (is<BinOp>(e)) {
    BinOp b = boost::get<BinOp>(e);
    Expression l = b.left();
    Expression r = b.right();

    if (b.op() == Add || b.op() == Sub) 
      return checkLinear(l) && checkLinear(r);

    else if (b.op() == Mult) 
      return (is<Integer>(l) && checkLinearBase(r)) || (is<Integer>(r) && checkLinearBase(l)); 

    else if (b.op() == Div)
      return is<Integer>(r) && checkLinearBase(l);

    else if (b.op() == Exp)
      return is<Integer>(l) && is<Integer>(r);

    return false;
  }

  return checkLinearBase(e);
}

// Check that all expressions in an expression list are linear
bool Connectors::checkLinearList(ExpList expl) 
{
  foreach_ (Expression e, expl) {
    if (!checkLinear(e)) {
      LOG << "ERROR: Subscript " << e << " should be linear" << endl;
      return false;
    }
  }

  return true;
}

// Check that arrays in connect use the same subscripts in the
// same order, and also, that en each dimension a single variable
// is used (to ensure it is linear)
bool Connectors::checkCounters(ExpList l1, ExpList l2)
{
  ExpList::iterator itl1 = l1.begin();
  ExpList::iterator itl2 = l2.begin();

  while (itl1 != l1.end() && itl2 != l2.end()) {
    foreach_ (Name nm1, varsNms_) {
      Reference r1(nm1);
      Expression e1(r1);
      ContainsExpression co1(e1);

      bool coNm11 = Apply(co1, *itl1);
      bool coNm12 = Apply(co1, *itl2);

      foreach_ (Name nm2, varsNms_) {
        Reference r2(nm2);
        Expression e2(r2);
        ContainsExpression co2(e2);

        if (nm1 != nm2) {
          bool coNm21 = Apply(co2, *itl1);
          bool coNm22 = Apply(co2, *itl2);

          // l1 is non linear in some dimension 
          if (coNm11 && coNm21) {
            LOG << "ERROR: Subscript " << *itl1 << " should be linear" << endl;
            return false;
          }

          // l2 is non linear in some dimension
          if (coNm12 && coNm22) {
            LOG << "ERROR: Subscript " << *itl2 << " should be linear" << endl;
            return false;
          }

          // Different counters for same dimension
          if (coNm11 && coNm22) {
            LOG << "ERROR: Subscripts " << *itl1 << " and " << *itl2 << "should use the same variable" << endl;
            return false;
          }
        }
      }
    }

    ++itl1; 
    ++itl2;
  }

  return true;
}

// Check that subscripts in the arrays comply with the restrictions
// as described in the paper
bool Connectors::checkSubscripts(ExpOptList range1, ExpOptList range2)
{
  if (range1 && range2) {
    ExpList r1 = range1.get();
    ExpList r2 = range2.get();
 
    if (r1.size() == 0 || r2.size() == 0)
      return true;

    else if (r1.size() != r2.size()) {
      LOG << "MCC ERROR: Unmatched dimensions in connect equation" << endl;
      return false;
    }

    else {
      if (!checkLinearList(r1))
        return false;

      else if (!checkLinearList(r2))
        return false;

      else if (!checkCounters(r1, r2)) 
        return false;
    }
  }

  return true;
}

// Connects --------------------------------------------------------------------------------------

// Returns a pair containing a variable in the first
// component, and its subscripts in the second component
Pair<Name, ExpOptList> Connectors::separate(Expression e)
{
  Reference reference;

  if (is<UnaryOp>(e)) {
    UnaryOp u = boost::get<UnaryOp>(e);
    if (is<Reference>(u.exp()))
      reference = boost::get<Reference>(u.exp());
    else
      LOG << "ERROR: It should be a Reference" << std::endl;
  } 
 
  else if (is<Reference>(e))
    reference = boost::get<Reference>(e);

  Ref refs = reference.ref();
  if (refs.size() > 1) 
    LOG << "ERROR: There shouldn't be calls to members in connectores" << std::endl;
  RefTuple rf = refs.front();
  ExpOptList opti;
  if (get<1>(rf).size() > 0) 
    opti = Option<ExpList>(get<1>(rf));
  Name r = get<0>(rf);

  return Pair<Name, ExpOptList>(r, opti);
}

// Create set-vertices and set-edges for variables in connect equations
bool Connectors::connect(Connect co)
{
  Expression eleft = co.left(), eright = co.right();
  
  Pair<Name, ExpOptList> left = separate(eleft);
  Pair<Name, ExpOptList> right = separate(eright);
  ExpOptList range1 = get<1>(left);
  ExpOptList range2 = get<1>(right);

  if (checkSubscripts(range1, range2)) {
    Name v1 = get<0>(left);
    Name v2 = get<0>(right);

    Option<SetVertexDesc> V1 = createVertex(v1);
    Option<SetVertexDesc> V2 = createVertex(v2);

    if (V1 && V2) {
      createEdge(range1, range2, *V1, *V2);
      return true;
    }

    else
      return false; 
  }

  return false;
}

// Graph creation --------------------------------------------------------------------------------

// Check if expression e uses another variable apart from n
// (If a counter does so in its declaration, they aren't independent)
bool Connectors::checkIndependentCounters(Name n, Expression e)
{
  foreach_ (Name nm, counters_) {
    Reference rnm(nm);
    Expression enm(rnm);
    ContainsExpression co(enm);
    
    if (Apply(co, e) && nm != n) {
      LOG << "ERROR: Counters aren't independent in " << e << endl;
      return false;
    }
  }

  return true;
}

// Construct Connect Graph, treating connects.
// If the model doesn't satisfy the restrictions described in 
// the paper, an empty model will be returned.
Pair<bool, EquationList> Connectors::createGraph(EquationList &eqs)
{
  EquationList emptyEqs;

  EquationList notConnect;
  EquationList::iterator itNC = notConnect.begin();
  bool ok = true;

  foreach_(Equation eq, eqs) {
    // Connect equation
    if (is<Connect>(eq)) {
      ok = connect(get<Connect>(eq));

      if (!ok) {
        LOG << "ERROR: Ill-formed connect equation " << eq << endl;
        break;
      }
    }

    // Traverse ForEqs to find connects
    else if(is<ForEq>(eq)) {
      // Save information of counters
      UnordCT<Name> prevVarsNms = varsNms_;
      UnordCT<Name> auxVarsNms = varsNms_;
      OrdCT<Name> prevCounters = counters_;
      OrdCT<Name> auxCounters = counters_;
      OrdCT<Name>::iterator itaux = auxCounters.begin();
      
      vector<Name> auxvars;
      vector<Name>::iterator itvars = auxvars.begin();
      ForEq feq = boost::get<ForEq>(eq);
      foreach_ (Index ind, feq.range().indexes()) {
        Name n = ind.name();
        OptExp e = ind.exp();
        if (e) {
          VarInfo vi(TypePrefixes(), n, Option<Comment>(), 
                     Option<Modification>(Modification(ModAssign(*e))), 
                     ExpOptList(ExpList(1, *e)), false);
          mmoclass_.addVar(n, vi);

          auxVarsNms.insert(n);
          itaux = auxCounters.insert(itaux, n);
          ++itaux;

          itvars = auxvars.insert(itvars, n);
          ++itvars;
        }

        else { 
          ok = false;
          LOG << "ERROR: Counter " << n << " should have bounds" << endl;
          break;
        }
      }

      set_varsNms(auxVarsNms);
      set_counters(auxCounters);

      // Traverse recursively inside the for
      EquationList eql = feq.elements();
      Pair<bool, EquationList> rec = createGraph(eql);
      ok = get<0>(rec);
      EquationList recNotConnEql = get<1>(rec);

      if (!ok)
        break;

      // Check that for with connect equations have independent counters
      if (eql.size() > 0 && eql.size() != recNotConnEql.size()) {
        foreach_ (Index ind, feq.range().indexes()) {
          Name n = ind.name();
          OptExp e = ind.exp();
       
          if (e) {
            bool indep = checkIndependentCounters(n, *e); 

            if (!indep) {
              ok = false;
              LOG << "ERROR: Counters aren't independent in " << *e << endl;
              break;
            }
          }

          else
            LOG << "COMPILER ERROR" << endl;
        }
      } 

      // If there are "non-connect" equations, put them in a new for
      if (!recNotConnEql.empty()) {
        ForEq forrec(feq.range().indexes(), recNotConnEql);
        itNC = notConnect.insert(itNC, forrec);
        ++itNC;
      }

      // Remove information of counters
      foreach_ (Name auxnm, auxvars)
        mmoclass_.rmVar(auxnm);

      set_varsNms(prevVarsNms);
      set_counters(prevCounters);
    }

    // Other equations
    else {
      itNC = notConnect.insert(itNC, eq);
      ++itNC;
    }
  }

  Pair<bool, EquationList> res(ok, notConnect);
  return res;
}
