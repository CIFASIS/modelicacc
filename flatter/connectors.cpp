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
#include <util/ast_visitors/contains_expression_flatter.h>
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

member_imp(Connectors, set<Name>, varsNms);
member_imp(Connectors, set<Name>, flowVars);
member_imp(Connectors, set<Name>, effVars);

member_imp(Connectors, vector<Name>, counters);
member_imp(Connectors, ExpList, countersCG);

member_imp(Connectors, VarsDimsTable, varsDims);

Connectors::Connectors(MMO_Class &c) : mmoclass_(c), ECount_(0), maxdim_(1) {}

// Determine if variable is a flow variable
bool Connectors::isFlowVar(Name v) 
{
  if (v[0] == '-')
    v = v.substr(1, v.length() - 1);

  Option<VarInfo> vi = mmoclass_.syms()[v];
  if (vi) {
    TypePrefixes tps = (*vi).prefixes_;

    foreach_ (Option<TypePrefix> tp, tps) {
      if (tp) {
        if (*tp == flow)
          return true;
      }
    }
  }

  return false;
}

// Add var to different containers of Connectors
void Connectors::addVar(Name n, VarInfo vi) {
  mmoclass_.addVar(n, vi);

  // Fill variables, effort and flow names
  varsNms_.insert(n);

  if (!isFlowVar(n))
    effVars_.insert(n);

  else
    flowVars_.insert(n);

  // Fill varsDims
  int dims = 0;
  Option<ExpList> vinds = vi.indices_;
  if (vinds) 
    dims = (*vinds).size();

  Option<int> odims(dims);
  varsDims_.insert(n, dims); 
}


// Initialization
bool Connectors::init() 
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

    else {
      LOG << "Variable " << n << " without information" << endl;
      return false;
    }
  }  

  vector<NI1> maxdimVector(maxdim_, 1);
  set_vCount(maxdimVector);
  set_eCount(maxdimVector);

  // Fill all variable related info
  foreach_ (Name n, mmoclass_.variables()) {
    Option<VarInfo> vinfo = mmoclass_.syms()[n];

    if (vinfo)
      addVar(n, *vinfo);

    else {
      LOG << "Variable " << n << " without information" << endl;
      return false;
    }
  }

  // Fill countersCG
  ExpList newCG;
  ExpList::iterator itNewCG = newCG.begin();
  for(unsigned int i = 0; (int) i < maxdim_; ++i){
    string nm(1, 105 + i);
    itNewCG = newCG.insert(itNewCG, Expression(nm));
    ++itNewCG;
  }

  set_countersCG(newCG);

  return true;
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
  bool okinit = init();

  if (okinit) {
    Pair<bool, EquationList> gr = buildGraph(mmoclass_.equations_ref().equations_ref());
    if (get<0>(gr)) {
      EquationList allEqs;
      EquationList::iterator itAll = allEqs.begin();

      notConnectEqs_ = get<1>(gr);

      foreach_(Equation e, notConnectEqs_) {
        itAll = allEqs.insert(itAll, e);
        ++itAll;
      }

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

      EquationList connEqs = generateCode();
      foreach_(Equation e, connEqs) {
        itAll = allEqs.insert(itAll, e);
        ++itAll;
      }

      mmoclass_.set_equations(EquationSection(false, allEqs));

      cout << mmoclass_ << "\n";
    }
  }

  else {
    SBGraph emptyG;
    set_G(emptyG);

    MMO_Class emptyMmo;
    set_mmoclass(emptyMmo);
  }
}

// Set-vertex creation ---------------------------------------------------------------------------

// Given a connector name, get variables of the connector
// Is used in the case in which the operator '-' is applied
// to a connector, to add new negative variables of the connector
set<Name> Connectors::getByPrefix(Name n) 
{
  set<Name> res;

  if (n[0] == '-')
    n = n.substr(1, n.length() - 1);

  foreach_ (Name nm, varsNms_) {
    if (nm.length() > n.length()) {
      Name auxnm = nm.substr(0, n.length());
      Name end = nm.substr(n.length(), 1);

      if (n == auxnm && end[0] == '_') {
        if (n[0] == '-')
          nm = '-' + nm;

        res.insert(nm);
      }
    }
  }

  return res;
}

// Add "negative" connectors vars to different containers of Connectors
// The name n should be the name of a connector
// This function should be only called by buildVertex
Option<VarInfo> Connectors::addConnectorVars(Name n) 
{
  Option<VarInfo> ovi = mmoclass_.getVar(n);

  if (n[0] == '-') {
    ovi = mmoclass_.getVar(n.substr(1, n.length() - 1));

    if (ovi) {
      foreach_ (Name nm, getByPrefix(n))
        addVar('-' + nm, *ovi);
    }

    else {
      LOG << "ERROR: Variable " << n << " should be defined" << endl;
      return Option<VarInfo>();
    }
  }

  return ovi;
}

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
// To be used in buildVertex
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
      i = Interval(offset, auxi.step(), offset + auxi.lo() - 1);

      ++itinds;
    }

    // Fill other dimensions
    else
      i = Interval(offset, 1, offset);

    v_intervals.addInter(i);

    itNewVC = newVCount.insert(itNewVC, i.hi() + 1);
    ++itNewVC;
  }

  set_vCount(newVCount);
  return buildSet(v_intervals);
}

// Create a set vertex for each variable in a connect,
// if it isn't already created. Variables which aren't
// part of a connect are ignored.
Option<SetVertexDesc> Connectors::buildVertex(Name n)
{
  Name auxn = n;
  Option<VarInfo> ovi = addConnectorVars(n);

  // Vertex already created
  foreach_ (SetVertexDesc Vdesc, vertices(G_)) {
    if (G_[Vdesc].name == n)
      return Option<SetVertexDesc>(Vdesc);
  }
 
  // Vertex doesn't exist 
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
// To be used in buildEdgeDom
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
    i = Interval(*itEC + auxi.lo() - 1, auxi.step(), *itEC + auxi.hi() - 1);

    v_intervals.addInter(i);

    itNewEC = newECount.insert(itNewEC, i.hi() + 1);
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
Set Connectors::buildEdgeDom(ExpOptList r) 
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

  // Connect is inside some loop
  else if (r) {
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
LMap Connectors::buildLM(MultiInterval mi1, MultiInterval mi2)
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
    if ((*itmi2).card() == 1) {
      NI2 newg = 0;
      NI2 newo = (*itmi2).lo(); 

      itresg = resg.insert(itresg, newg);
      itreso = reso.insert(itreso, newo);

      ++itresg;
      ++itreso;
    }

    else if ((*itmi1).card() == (*itmi2).card()) {
      NI2 newg = (*itmi2).step() / (*itmi1).step();
      NI2 newo = (*itmi2).lo() - newg * (*itmi1).lo(); 

      itresg = resg.insert(itresg, newg);
      itreso = reso.insert(itreso, newo);

      ++itresg;
      ++itreso;
    }

    else {
      LOG << "ERROR: Check connect subscripts " << mi1 << " | " << mi2 << endl;
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
      int offset = (*itmi).lo();
      Interval res(ndim.lo() + offset - 1, ndim.step(), ndim.hi() + offset - 1);
      itmires = mires.insert(itmires, (*itmi).cap(res));

      ++itmires;
      ++itmi;
    }
  }

  else
    mires = miinters;

  return MultiInterval(mires);
}

// Create left and right maps
PWLMap Connectors::buildEdgeMap(Set dom, Set im, ExpOptList r) 
{
  PWLMap res;

  // In the current implementation, dom and im have only one atomic set
  // Check buildSet and buildEdgeDom
  if (dom.asets_().size() != 1 || im.asets_().size() != 1) {
    LOG << "COMPILER ERROR: dom and im should have only one atomic set" << endl;
    return res;
  }

  else {
    AtomSet domas = *(dom.asets_().begin());
    AtomSet imas = *(im.asets_().begin());

    MultiInterval dommi = domas.aset_();
    MultiInterval immi = imas.aset_();

    LMap lm = buildLM(dommi, subscriptMI(immi, r));
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

// Create a set-edges. Current implementation might create several edges to connect
// the same vertices. This doesn't impact on the cost of the algorithm
// ENHANCEMENT: update edges insted of creeating, if set-edge exists
void Connectors::buildEdge(ExpOptList r1, ExpOptList r2, SetVertexDesc Vdesc1, SetVertexDesc Vdesc2) 
{
  SetVertex V1 = G_[Vdesc1];
  SetVertex V2 = G_[Vdesc2];

  string nm = "E_";

  if (V1.name_() < V2.name_())
    nm = nm + V1.name_() + "_" + V2.name_();

  else
    nm = nm + V2.name_() + "_" + V1.name_();

  // Create new set-edge
  Set dom1 = buildEdgeDom(r1);
  Set dom2 = buildEdgeDom(r2);
  if (dom1.empty() && dom2.empty()) {
    LOG << "ERROR: Check connects " << V1.name_() << ", " << V2.name_() << endl;
    return;
  }
  Set dom = dom1;
  if (dom2.size() > dom1.size())
    dom = dom2;
  PWLMap pw1 = buildEdgeMap(dom, V1.vs_(), r1);
  PWLMap pw2 = buildEdgeMap(dom, V2.vs_(), r2);
  SetEdge E(nm, pw1, pw2);

  SetEdgeDesc Edesc;
  bool b;
  boost::tie(Edesc, b) = boost::add_edge(Vdesc1, Vdesc2, G_);
  G_[Edesc] = E;
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

    if (is<UnaryOp>(eleft)) {
      UnaryOp uop = boost::get<UnaryOp>(eleft);
      if (uop.op_ == Minus) 
        v1 = "-" + v1;
    }

    if (is<UnaryOp>(eright)) {
      UnaryOp uop = boost::get<UnaryOp>(eright);
      if (uop.op_ == Minus)
        v2 = "-" + v2;
    }

    Option<SetVertexDesc> V1 = buildVertex(v1);
    Option<SetVertexDesc> V2 = buildVertex(v2);

    if (V1 && V2) {
      buildEdge(range1, range2, *V1, *V2);
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
bool Connectors::checkIndependentCounters(ForEq feq)
{
  foreach_ (Index ind, feq.range().indexes()) {
    Name n = ind.name();
    OptExp e = ind.exp();

    if (e) {
      foreach_ (Name nm, counters_) {
        Reference rnm(nm);
        Expression enm(rnm);
        ContainsExpression co(enm);
      
        if (Apply(co, *e) && nm != n) {
          LOG << "ERROR: Counters aren't independent in " << e << endl;
          return false;
        }
      }
    }

    else
      LOG << "ERROR: Empty counter range" << endl;
  }

  return true;
}

// Create vertices for non connected connectors (for flow equations)
void Connectors::buildDisconnected()
{
  foreach_ (Name n, flowVars_) { 
    int i = n.length();
    while (n[i] != '_')
      i--;

    buildVertex(n.substr(0, i));
  }
}

// Construct Connect Graph, treating connects.
// If the model doesn't satisfy the restrictions described in 
// the paper, an empty model will be returned.
Pair<bool, EquationList> Connectors::buildConnects(EquationList &eqs)
{
  EquationList emptyEqs;

  EquationList notConnect;
  EquationList::iterator itNC = notConnect.begin();
  bool ok = true;

  const VarSymbolTable auxsyms = mmoclass_.syms();
  EvalExpFlatter evexp(auxsyms);

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
      set<Name> prevVarsNms = varsNms_;
      set<Name> auxVarsNms = varsNms_;
      vector<Name> prevCounters = counters_;
      vector<Name> auxCounters = counters_;
      vector<Name>::iterator itaux = auxCounters.begin();
      
      vector<Name> auxvars;
      vector<Name>::iterator itvars = auxvars.begin();
      ForEq feq = boost::get<ForEq>(eq);
      foreach_ (Index ind, feq.range().indexes()) {
        Name n = ind.name();
        OptExp e = ind.exp();
        if (e) {
          Interval i = Apply(evexp, *e);
          if (!i.empty()) {
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
      Pair<bool, EquationList> rec = buildConnects(eql);
      ok = get<0>(rec);
      EquationList recNotConnEql = get<1>(rec);

      if (!ok)
        break;

      // Check that for with connect equations have independent counters
      if (eql.size() > 0 && eql.size() != recNotConnEql.size()) 
        ok = checkIndependentCounters(feq);

      if (!ok)
        break;

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

Pair<bool, EquationList> Connectors::buildGraph(EquationList &eqs)
{
  buildDisconnected();
  return buildConnects(eqs); 
}

// Code generation helpers -----------------------------------------------------------------------

// Given a set-vertex, returns its name
Name Connectors::getName(AtomSet as)
{
  Name nm;

  Set auxas;
  auxas.addAtomSet(as);

  foreach_ (SetVertexDesc vi, vertices(G_)) {
    Set vs = G_[vi].vs_();

    if (!auxas.cap(vs).empty()) 
      nm = G_[vi].name_();
  }

  return nm;
}

// Given a subset of a set-vertex, returns the set which contains
// all the vertices in the set-vertex
AtomSet Connectors::getAtomSet(AtomSet as) 
{
  AtomSet res;

  Set auxas;
  auxas.addAtomSet(as);

  foreach_ (SetVertexDesc vi, vertices(G_)) {
    Set vs = G_[vi].vs_();

    if (!auxas.cap(vs).empty()) 
      res = *(vs.asets_().begin());
  }

  return res;
}

// Determine if a map is the identity
bool Connectors::isIdMap(LMap lm) 
{
  bool cond = true;

  foreach_ (NI2 g, lm.gain_())
    if (g != 1)
      cond = false;

  foreach_ (NI2 o, lm.off_())
    if (o != 0)
      cond = false;

  return cond;
}

// Given a set that represents a connector,
// get variables names of the connector, that are effort variables
vector<Name> Connectors::getEffVars(Set connector)
{
  vector<Name> res;
  vector<Name>::iterator itres = res.begin();

  // Get connector name
  foreach_ (SetVertexDesc vi, vertices(G_)) {
    Set vs = G_[vi].vs_();
    Name vinm = G_[vi].name_();

    if (!connector.cap(vs).empty()) {
      // Search effort vars in the connector
      foreach_ (Name e, effVars_) {
        Name aux = e.substr(0, vinm.length()); 

        if (e.size() > vinm.size()) {
          Name end = e.substr(vinm.length(), 1);

          if (aux == vinm && end == "_") {
            itres = res.insert(itres, e);
            ++itres;
          }
        }
      }
    }
  }

  sort(res.begin(), res.end());

  return res;
}

// Given a set that represents a connector,
// get variables names of the connector, that are flow variables
vector<Name> Connectors::getFlowVars(Set connector)
{
  vector<Name> res;
  vector<Name>::iterator itres = res.begin();

  // Get connector name
  foreach_ (SetVertexDesc vi, vertices(G_)) {
    Set vs = G_[vi].vs_();
    Name vinm = G_[vi].name_();

    if (!connector.cap(vs).empty()) {
      // Search flow vars in the connector
      foreach_ (Name e, flowVars_) {
        Name aux = e.substr(0, vinm.length()); 
        if (e.size() > vinm.size()) {
          Name end = e.substr(vinm.length(), 1);
          if (aux == vinm && end == "_") {
            itres = res.insert(itres, e);
            ++itres;
          }
        }
      }
    }
  }

  sort(res.begin(), res.end());

  return res;
}

// Given a representant of a connected component, build the loop indexes 
// that will be used to write the flattened equations.
// In each dimension the counter will set its bounds according to
// the maximum number of elements in that dimension in all subsets of set-vertices.
Indexes Connectors::buildIndex(Set connected)
{
  IndexList indexes;
  IndexList::iterator itinds = indexes.begin();

  ExpList::iterator itCG = countersCG_.begin();

  OrdCT<NI1> nElems; // Maximum number of elements in each dimension
  foreach_ (AtomSet c, connected.asets_()) {
    // Traverse dimensions
    OrdCT<NI1>::iterator itElems = nElems.begin();
    OrdCT<NI1> nElemsAux;
    OrdCT<NI1>::iterator itAux = nElemsAux.begin();
    foreach_ (Interval i, c.aset_().inters_()) {
      int elems = i.card();
      if (*itElems)
        elems = max(*itElems, elems);

      itAux = nElemsAux.insert(itAux, elems);
      ++itAux;
      ++itElems;
    }

    nElems = nElemsAux;
  }

  // Traverse dimensions
  foreach_ (NI1 n, nElems) {
    Range r(Expression(1), Expression(1), Expression(n));
    Expression er(r);
    Option<Expression> oer(er);

    Name nm = get<Name>(*itCG);
    Index index(nm, oer);

    itinds = indexes.insert(itinds, index);
    ++itinds;
    ++itCG;
  }

  Indexes res(indexes);
  return res;
}

// Get atom sets that are in the dom of ccG_, and whose image
// is equal to atomRept (represented vertices)
Set Connectors::getRepd(AtomSet atomRept) 
{
  UnordCT<AtomSet> atomRes;

  Set rept(atomRept);
  Set pre = ccG_.preImage(rept);
  Set diff = pre.diff(rept);

  if (diff.empty())
    return rept;

  OrdCT<LMap> lm = ccG_.lmap_();
  OrdCT<LMap>::iterator itlm = lm.begin();

  foreach_ (Set d, ccG_.dom_()) {
    AtomSet atomD = *(d.asets_().begin()); // ccG_ is atomized 
    PWAtomLMap atomPW(atomD, *itlm);

    diff = d.diff(rept);
    if (atomPW.image(atomD) == atomRept && !diff.empty() && atomD != atomRept)
      atomRes.insert(atomD);

    ++itlm;
  }

  return Set(atomRes);
}

// The sub-set "as" of a connector will be part of some equations in a loop
// with counters described by indexes. Therefore, this function creates the
// necessary subscripts
// e.g.: as describes the array v[2:N], but the counter used is i in 1:N-1
// Therefore, buildSubscripts will return [i+1], and so the loop will be:
// for i in 1:N-1
// ...
// v[i+1] = ...
// ...
// end for;
ExpList Connectors::buildSubscripts(Indexes indexes, AtomSet original, AtomSet as, int dims)
{
  ExpList res;
  ExpList::iterator itres = res.begin();

  OrdCT<Interval> miori = original.aset_().inters_();
  OrdCT<Interval>::iterator itmiori = miori.begin();
  OrdCT<Interval> mias = as.aset_().inters_();
  OrdCT<Interval>::iterator itmias = mias.begin();

  const VarSymbolTable auxsyms = mmoclass_.syms();
  EvalExpFlatter evexp(auxsyms);

  ExpList::iterator itCG = countersCG_.begin();

  // Not an array, doesn't need subscripts
  if (dims == 0)
    return res;

  // Traverse counters, creating subscripts
  int dim = 0;
  foreach_ (Index ind, indexes.indexes_) {
    // Complete subscripts up to variable dimension
    if (dim < dims) {
      OptExp oe = ind.exp_;
      if (oe) {
        int loInd = Apply(evexp, *oe).lo();

        // Constant value in dimension, don't use counter
        if ((*itmias).card() == 1) {
          int off = (*itmias).lo() - (*itmiori).lo() + 1; 

          Expression eoff(off);
          itres = res.insert(itres, eoff);
        }

        // Use counter
        else {
          Real step = (*itmias).step(); 
          Real off = (((*itmias).lo() - (*itmiori).lo() + 1)) - (step * loInd); 

          if (step == 0)
            itres = res.insert(itres, Expression(off));

          else if (step == 1) {
            if (off == 0) 
              itres = res.insert(itres, *itCG);

            else if (off > 0) {
              Expression eoff(off);
              BinOp bop(*itCG, Add, eoff);
              itres = res.insert(itres, bop);
            }

            else {
              Expression eoff(-off);
              BinOp bop(*itCG, Sub, eoff);
              itres = res.insert(itres, bop);
            }
          }

          else {
            Expression estep(step);
            BinOp bop(estep, Mult, *itCG);

            if (off == 0) 
              itres = res.insert(itres, bop);

            else if (off > 0) {
              Expression eoff(off);
              BinOp linearbop(bop, Add, eoff);
              itres = res.insert(itres, linearbop);
            }

            else {
              Expression eoff(-off);
              BinOp linearbop(bop, Sub, eoff);
              itres = res.insert(itres, linearbop);
            }
          }
        }

        ++itres;
      }

      ++itmiori;
      ++itmias;
    }

    ++itCG;
    ++dim;
  }

  return res;
}

// Build expressions for variables used in equations in a loop
// The third argument is to restrict use to either effort or flow variables
ExpList Connectors::buildLoopExpr(Indexes indexes, AtomSet as, vector<Name> vars) 
{
  ExpList res;
  ExpList::iterator itres = res.begin();

  Set auxas;
  auxas.addAtomSet(as);

  // Traverse variables in the loop
  foreach_ (Name nmas, vars) {
    Option<int> dims = varsDims_[nmas];
    ExpList subs = buildSubscripts(indexes, getAtomSet(as), as, *dims);
    Reference rrept(nmas, subs);
    Expression e = rrept;

    if (nmas[0] == '-') {
      rrept = Reference(nmas.substr(1, nmas.length() - 1), subs);

      if (isFlowVar(nmas)) 
        e = UnaryOp(rrept, Minus);

      else 
        e = rrept;
    }

    itres = res.insert(itres, e);
    ++itres;
  }

  return res;
}

// Build ranges for sums in flow equations
ExpList Connectors::buildRanges(AtomSet original, AtomSet as)
{
  ExpList res;
  ExpList::iterator itres = res.begin();

  OrdCT<Interval> intersas = as.aset_().inters_();
  OrdCT<Interval>::iterator itas = intersas.begin();

  // A range is needed in the sum
  if (as.size() != 1) {
    foreach_(Interval iori, original.aset_().inters_()) {
      NI1 lo = (*itas).lo() - iori.lo() + 1;
      NI1 st = (*itas).step();
      NI1 hi = (*itas).hi() - iori.lo() + 1;
      Expression elo(lo);
      Expression est(st);
      Expression ehi(hi);
      Range r(elo, est, ehi);
      Expression expr(r);

      itres = res.insert(itres, expr);
      ++itres;
              
      ++itas;
    }
  }

  // No need of range, just a constant
  else {
    foreach_(Interval iori, original.aset_().inters_()) {
      NI1 lo = (*itas).lo() - iori.lo() + 1;
      Expression expr(lo);

      itres = res.insert(itres, expr);
      ++itres;
              
      ++itas;
    }
  }

  return res;
}

// Build the sums needed for a flow equation
ExpList Connectors::buildAddExpr(AtomSet atomRept, AtomSet as) 
{
  ExpList res;
  ExpList::iterator itres = res.begin();

  Set auxas;
  auxas.addAtomSet(as);

  foreach_ (Name nmas, getFlowVars(auxas)) {
    // A sum is needed
    if (atomRept.size() != as.size()) {
      ExpList range = buildRanges(getAtomSet(as), as);
      RefTuple rrept(nmas, range);
      AddAll add(rrept);
      itres = res.insert(itres, add);
    }

    else {
      AtomSet original = getAtomSet(as);

      // No need of subscript
      if (varsDims_[nmas] == 0) {
        Reference ref(nmas);
        itres = res.insert(itres, ref);
      }

      // A subscript is needed
      else {
        OrdCT<NI1> minOriginal = original.minElem();
        OrdCT<NI1> minAs = as.minElem();
        OrdCT<NI1>::iterator itMinAs = minAs.begin();

        ExpList subs;
        ExpList::iterator itsubs = subs.begin();

        foreach_ (NI1 lo, minOriginal) {
          itsubs = subs.insert(itsubs, *itMinAs - lo + 1);
          ++itsubs;
          ++itMinAs;
        }

        Reference ref(nmas, subs);
        itres = res.insert(itres, ref);
      }
    }

    ++itres;
  }

  return res;
}

// Given the indexes and list of equations, create a loop if it's necessary
EquationList Connectors::buildLoop(Indexes indexes, EquationList eqs)
{
  EquationList res;
  EquationList::iterator itres = res.begin();
  EquationList loopeqs;
  EquationList::iterator itloop = loopeqs.begin();

  set<Name> indexesNms;

  foreach_ (Index i, indexes.indexes_) 
    indexesNms.insert(i.name_);

  // Traverse equations
  foreach_ (Equation eq, eqs) {
    if (is<Equality>(eq)) {
      Equality eqty = boost::get<Equality>(eq);
      Expression left = eqty.left_, right = eqty.right_;

      bool usesCounters = false;

      // Traverse dimensions
      foreach_ (Name i, indexesNms) {
        Reference refi(i);
        Expression ei(refi);
        ContainsExpressionFlatter co(ei);

        // Equation uses counter, it should go in a loop
        if (Apply(co, left) || Apply(co, right)) 
          usesCounters = true;
      }

      if (usesCounters) {
        itloop = loopeqs.insert(itloop, eq);
        ++itloop;
      }

      else {
        itres = res.insert(itres, eq);
        ++itres;
      }
    }
  }

  // Create loops, and add the to result
  if (loopeqs.size() != 0) { 
    ForEq feq(indexes, loopeqs);
    res.insert(itres, feq);
  }

  return res;
}

// Effort ----------------------------------------------------------------------------------------

// Create effort equations for a given representant
EquationList Connectors::buildEffEquations(Indexes indexes, AtomSet atomRept, Set repd) 
{
  EquationList effEqs;
  EquationList::iterator itEffEqs = effEqs.begin();

  Set rept(atomRept);
  Set diff1 = rept.diff(repd);
  Set diff2 = repd.diff(rept);

  if (diff1.empty() && diff2.empty())
    return effEqs;
   
  // Representant subscripts 
  ExpList effRept = buildLoopExpr(indexes, atomRept, getEffVars(Set(atomRept)));
  Expression left = *(effRept.begin());

  // Other effort variables in the representant
  foreach_ (Expression expRept, effRept) {
    if (left != expRept) {
      Equality eq(left, expRept);
      itEffEqs = effEqs.insert(itEffEqs, eq);
      ++itEffEqs;
    }
  }

  // Traverse represented connectors
  foreach_ (AtomSet atomRepd, repd.asets_()) {
    ExpList effRepd = buildLoopExpr(indexes, atomRepd, getEffVars(Set(atomRepd)));
    // Represented variables in connector
    foreach_ (Expression eRepd, effRepd) {
      Equality eq(left, eRepd);
      itEffEqs = effEqs.insert(itEffEqs, eq);
      ++itEffEqs;
    }
  }

  if (!effEqs.size() == 0)
    effEqs = buildLoop(indexes, effEqs);

  return effEqs;
}

// Flow ------------------------------------------------------------------------------------------

// Create effort equations for a given representant
EquationList Connectors::buildFlowEquations(Indexes indexes, AtomSet atomRept, Set repd) 
{
  EquationList res;

  Expression right(0);

  Set rept(atomRept);
  Set diff = repd.diff(rept);

  if(repd.empty())
    return res;

  if (diff.empty()) {
    Set emptySet;
    repd = emptySet;
  }

  // A loop is needed
  if (atomRept.size() > 1) {
    EquationList eqsLoop;

    // Representant subscripts 
    ExpList flowRept = buildLoopExpr(indexes, atomRept, getFlowVars(Set(atomRept)));
    Expression left = *(flowRept.begin());

    // Other flow variables in the representant
    foreach_ (Expression expRept, flowRept) {
      if (left != expRept) 
        left = BinOp(left, Add, expRept);
    }

    // Traverse represented connectors
    foreach_ (AtomSet atomRepd, repd.asets_()) {
      ExpList flowRepd = buildLoopExpr(indexes, atomRepd, getFlowVars(Set(atomRepd)));
      // Represented variables in connector
      foreach_ (Expression expRepd, flowRepd) {
        left = BinOp(left, Add, expRepd);
      }
    }

    Equality eq(left, right);
    eqsLoop.insert(eqsLoop.begin(), eq);
    if (!eqsLoop.size() == 0)
      res = buildLoop(indexes, eqsLoop);
  }

  // No need for a loop, sums will be used if needed
  else {
    ExpList rept = buildAddExpr(atomRept, atomRept);
    Expression left = *(rept.begin());

    foreach_ (Expression e, rept) {
      if (e != *(rept.begin()))
        left = BinOp(left, Add, e);
    }

    // Traverse represented connectors
    foreach_ (AtomSet atomRepd, repd.asets_()) {
      ExpList add = buildAddExpr(atomRept, atomRepd);

      foreach_ (Expression e, add) 
        left = BinOp(left, Add, e);
    }

    Equality eq(left, right);
    res.insert(res.begin(), eq);
  }

  return res;
}

// Code generation -------------------------------------------------------------------------------

// Update mmoclass_ with connect's equations
EquationList Connectors::generateCode() 
{
  EquationList res;
  EquationList::iterator itres = res.begin();

  set_ccG(ccG_.atomize());
  OrdCT<LMap> lmapccG = ccG_.lmap_();
  OrdCT<LMap>::iterator itLMapccG = lmapccG.begin();

  Set wDom = ccG_.wholeDom();
  Set im = ccG_.image(wDom);

  Set flowVertices; // Flow variables already in an equation
  Set effReps;

  // Traverse connected components
  foreach_ (Set repd, ccG_.dom_()) {
    AtomSet atomRepd = *(repd.asets_().begin());
    PWAtomLMap atomMap(atomRepd, *itLMapccG);
    AtomSet atomRept = atomMap.image(atomRepd);
    Set rept(atomRept);
    repd = ccG_.preImage(rept);

    Indexes indexes = buildIndex(repd);

    repd = getRepd(atomRept);
    if (!isIdMap(*itLMapccG)) {
      // Effort equations
      EquationList effEqs = buildEffEquations(indexes, atomRept, repd);

      foreach_ (Equation e, effEqs) {
        itres = res.insert(itres, e);
        ++itres;
      }
    }

    // Flow equations
    //repd = getRepd(atomRept).diff(flowVertices);
    EquationList flowEqs = buildFlowEquations(indexes, atomRept, repd);

    foreach_ (Equation e, flowEqs) {
      itres = res.insert(itres, e);
      ++itres;
    }

    flowVertices = flowVertices.cup(repd);

    ++itLMapccG;
  }

  // Update flow variables and remove the prefix before generating code.
  foreach_(Name nm, mmoclass_.variables()){
    Option<VarInfo> ovi = mmoclass_.getVar(nm);
    if(ovi){
      VarInfo vi = *ovi;
      Name ty = vi.type();
      if(ty == "Real"){
        vi.removePrefix(flow);
        mmoclass_.addVar(nm, vi);
      }
    }
  }

  res = simplifyCode(res);

  return res;
}

// Delete duplicate, trivial or equations
EquationList Connectors::simplifyCode(EquationList eql)
{
  EquationList res;
  EquationList::iterator itres = res.begin();

  foreach_ (Equation eq1, eql) {
    bool trivial = false;
    bool found = false;
   
    if (is<ForEq>(eq1)) {
      EquationList newl;
      EquationList::iterator itnewl = newl.begin();

      ForEq feq = boost::get<ForEq>(eq1);
      foreach_ (Equation eq, feq.elements()) {
        if (is<Equality>(eq)) {
          Equality eqty = boost::get<Equality>(eq);
          if (eqty.left() != eqty.right()) {
            itnewl = newl.insert(itnewl, eqty);
            ++itnewl;
          }
        }

        else {
          itnewl = newl.insert(itnewl, eq);
          ++itnewl;
        }
      }

      ForEq newFeq(feq.range().indexes(), newl);
      eq1 = newFeq;
    }

    else if (is<Equality>(eq1)) { 
      Equality eqty = boost::get<Equality>(eq1);
      if (eqty.left() == eqty.right())
        trivial = true;
    }

    foreach_ (Equation eq2, res) {
      if (eq1 == eq2)
        found = true;
    }

    if (!found && !trivial) {
      itres = res.insert(itres, eq1);
      ++itres;
    }
  }

  return res;
}
