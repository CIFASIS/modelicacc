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

using namespace std;
using namespace Modelica;
using namespace Modelica::AST;

#define NameToRef(X) Reference(Reference(), X, Option<ExpList>())
#define RefIndex(X, Y) Reference(Reference(), X, Option<ExpList>(Y))
#define PrintOpt(N) (N ? N.get() : "{}")

Connectors::Connectors(MMO_Class &c) 
 : mmoclass_(c), eCount2_(0){
  SBGraph g;
  G = g;
}

member_imp(Connectors, vector<int>, vCount);
member_imp(Connectors, vector<int>, eCount1);
member_imp(Connectors, int, eCount2);
member_imp(Connectors, MMO_Class, mmoclass);
member_imp(Connectors, VertexNameTable, vnmtable);
member_imp(Connectors, NameVertexTable, nmvtable);
//member_imp(Connectors, EquationList, oldeqs);

/*|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||*/
/*-----------------------------------------------------------------------------------------------*/
// Debugging functions --------------------------------------------------------------------------//
/*-----------------------------------------------------------------------------------------------*/
/*|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||*/

void Connectors::debug(std::string filename){
  VertexIt vi, vi_end;
  boost::tie(vi, vi_end) = boost::vertices(G);
  for(; vi != vi_end; ++vi){
    Name n = G[*vi].name;
    Set vs = G[*vi].vs_();
    cout << n << ": " << vs << "\n";
  }

  EdgeIt ei, ei_end;
  boost::tie(ei, ei_end) = boost::edges(G);
  for(; ei != ei_end; ++ei){
    Name n = G[*ei].name;
    PWLMap es1 = G[*ei].es1_();
    PWLMap es2 = G[*ei].es2_();
    cout << n << ": " << es1 << ", " << es2 << "\n";
  }

  GraphPrinter gp(G, -1);

  gp.printGraph(filename);
  cout << "Generated Connect Graph written to " << filename << endl;
}

/*|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||*/
/*-----------------------------------------------------------------------------------------------*/
// Create graph  --------------------------------------------------------------------------------//
/*-----------------------------------------------------------------------------------------------*/
/*|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||*/

EquationList eqlinit;
EquationList Connectors::oldeqs = eqlinit;
EquationList::iterator Connectors::itold = oldeqs.begin();

void Connectors::solve(){
  int maxdim = 1;
  foreach_(Name n, mmoclass_.variables()){
    Option<VarInfo> ovi = mmoclass_.getVar(n);
    if(ovi){
      VarInfo vi = *ovi;
      Option<ExpList> oinds = vi.indices();
      if(oinds){
        ExpList inds = *oinds;
        NI1 aux = inds.size();
        maxdim = max(maxdim, aux);
      }
    }
  }  

  vector<NI1> aux(maxdim, 1);
  set_vCount(aux);
  set_eCount1(aux);

  EquationList eql;

  //cout << "mmo:\n" << mmoclass_ << "\n\n";
  bool ok = createGraph(mmoclass_.equations_ref().equations_ref());
  if(ok){
    debug("prueba.dot");

    PWLMap res = connectedComponents(G);
    cout << res << "\n\n";

    foreach_(Name nm, mmoclass_.variables()){
      Option<VarInfo> ovi = mmoclass_.getVar(nm);
      if(ovi){
        VarInfo vi = *ovi;
        Name ty = vi.type();
        if(ty != "Real" && ty != "Integer"){
          mmoclass_.rmVar(nm);
        }
      }
    }

    generateCode(res);

    cout << mmoclass_ << "\n";
  }

  else{
    SBGraph g;
    G = g;

    MMO_Class auxmmo;
    set_mmoclass(auxmmo);
  }
}

bool Connectors::createGraph(EquationList &eqs){
  bool ok = true;

  BOOST_FOREACH(Equation eq, eqs){
    if(is<Connect>(eq)){
      ok = connect(get<Connect>(eq));

      if(!ok)
        break;
    }

    else if(is<ForEq>(eq)){
      vector<Name> auxvars;
      vector<Name>::iterator itvars = auxvars.begin();
      ForEq feq = boost::get<ForEq>(eq);
      foreach_(Index ind, feq.range().indexes()){
        Name n = ind.name();
        OptExp e = ind.exp();
        if(e){
          TypePrefixes tp;
          Option<Comment> aux1;
          Expression aux2 = *e;
          ModAssign aux3(aux2);
          Modification aux4(aux3);
          Option<Modification> mod(aux4);
          ExpOptList aux5;
          VarInfo vi(tp, n, aux1, mod, aux5, false);
          mmoclass_.addVar(n, vi);
          itvars = auxvars.insert(itvars, n);
          ++itvars;
        }
        else 
          cerr << "Should be defined\n";
      }

      EquationList el = feq.elements();
      ok = createGraph(el);

      if(!ok)
        break;

      foreach_(Name auxnm, auxvars){
        mmoclass_.rmVar(auxnm);
      }
    }

    else{
      itold = oldeqs.insert(itold, eq);
      ++itold;
    }
  }

  return ok;
}

bool Connectors::connect(Connect co){
  Expression eleft = co.left(), eright = co.right();
  
  Pair<Name, ExpOptList> left = separate(eleft);
  Pair<Name, ExpOptList> right = separate(eright);

  Name v1 = get<0>(left);
  Name v2 = get<0>(right);

  OrdCT<Interval> miv1 = createVertex(v1).inters_();
  OrdCT<Interval>::iterator itmiv1 = miv1.begin(); 
  OrdCT<Interval> miv2 = createVertex(v2).inters_();
  OrdCT<Interval>::iterator itmiv2 = miv2.begin();

  ExpOptList range1 = get<1>(left);
  ExpOptList range2 = get<1>(right);

  if(checkRanges(range1, range2)){
    OrdCT<Interval> mi11;
    OrdCT<Interval>::iterator itmi11 = mi11.begin();
    int dim = 0;
    const VarSymbolTable aux = mmoclass_.syms();
    EvalExpFlatter evexp(aux);
    if(range1){
      foreach_(Expression e1, range1.get()){
        if(is<SubAll>(e1)){
          Option<VarInfo> ovi = mmoclass_.getVar(v1);
          if(ovi){
            VarInfo vi = *ovi;
            ExpOptList oinds = vi.indices();
            if(oinds){
              ExpList inds = *oinds;
              ExpList::iterator itinds = inds.begin();

              for(int i = 0; i < dim; ++i)
                ++itinds;

              e1 = *itinds;
            }
          }

          else
            return false;
        }

        Interval ll = Apply(evexp, e1);
        NI1 auxlo = (*itmiv1).lo_() - 1;
        Interval l(auxlo + ll.lo_(), ll.step_(), auxlo + ll.hi_());
        ++itmiv1;
      
        if(!l.empty_()){
          itmi11 = mi11.insert(itmi11, l);
          ++itmi11;
          ++dim;
        }

        else{
          OrdCT<Interval> aux;
          mi11 = aux;
          break;
        }
      }
    }

    else{
      itmi11 = mi11.begin();
      foreach_(Interval i1, miv1){
        NI1 auxlo = i1.lo_();
        Interval l(auxlo, 1, auxlo);
        itmi11 = mi11.insert(itmi11, l);
        ++itmi11;
      }
    }

    MultiInterval mi1(mi11);

    OrdCT<Interval> mi22;
    OrdCT<Interval>::iterator itmi22 = mi22.begin();
    dim = 0;
    if(range2){
      foreach_(Expression e2, range2.get()){
        if(is<SubAll>(e2)){
          Option<VarInfo> ovi = mmoclass_.getVar(v2);
          if(ovi){
            VarInfo vi = *ovi;
            ExpOptList oinds = vi.indices();
            if(oinds){
              ExpList inds = *oinds;
              ExpList::iterator itinds = inds.begin();

              for(int i = 0; i < dim; ++i)
                ++itinds;

              e2 = *itinds;
            }
          }

          else
            return false;
        }

        Interval rr = Apply(evexp, e2);
        NI1 auxlo = (*itmiv2).lo_() - 1;
        Interval r(auxlo + rr.lo_(), rr.step_(), auxlo + rr.hi_());
        ++itmiv2;

        if(!r.empty_()){
          itmi22 = mi22.insert(itmi22, r);
          ++itmi22;
          ++dim;
        }

        else{
          OrdCT<Interval> aux;
          mi22 = aux;
          break;
        }
      }
    }

    else{
      itmi22 = mi22.begin();
      foreach_(Interval i2, miv2){
        NI1 auxlo = i2.lo_();
        Interval r(auxlo, 1, auxlo);
        itmi22 = mi22.insert(itmi22, r);
        ++itmi22;
      }
    }

    MultiInterval mi2(mi22);

    VertexIt vi1, vi2, vi_end1, vi_end2;
    boost::tie(vi1, vi_end1) = boost::vertices(G);
    SetVertexDesc d1 = *vi1, d2 = *vi2;

    for(; vi1 != vi_end1; ++vi1){
      boost::tie(vi2, vi_end2) = boost::vertices(G);
      Name aux1 = G[*vi1].name;

      for(; vi2 != vi_end2; ++vi2){
        Name aux2 = G[*vi2].name;
        if(aux1 == v1 && aux2 == v2){
          d1 = *vi1;
          d2 = *vi2;
          updateGraph(d1, d2, mi1, mi2);
        }
      }
    }

    return true;
  }

  return false;
}

// Get expression and range
Pair<Name, ExpOptList> Connectors::separate(Expression e){
  Reference reference;

  if(is<UnaryOp>(e)){
    UnaryOp u = boost::get<UnaryOp>(e);
    if(is<Reference>(u.exp()))
      reference = boost::get<Reference>(u.exp());
    else
      std::cerr << "ERROR: Deberia llegar una Reference" << std::endl;
  } 
 
  else if (is<Reference>(e))
    reference = boost::get<Reference>(e);

  Ref refs = reference.ref();
  if(refs.size() > 1) 
    std::cerr << "ERROR: No deberia haber llamadas a miembros en connectors" << std::endl;
  RefTuple rf = refs.front();
  ExpOptList opti;
  if(get<1>(rf).size() > 0) 
    opti = Option<ExpList>(get<1>(rf));
  Name r = get<0>(rf);
  return Pair<Name, ExpOptList>(r, opti);
}

MultiInterval Connectors::createVertex(Name n){
  MultiInterval mires; 

  VertexIt vi, vi_end;
  boost::tie(vi, vi_end) = boost::vertices(G);
  bool exists = false;
  for(; vi != vi_end; ++vi){
    Name aux = G[*vi].name;
    if(aux == n){
      exists = true;
      AtomSet auxas = *(G[*vi].vs_().asets_().begin()); 
      mires = auxas.aset_(); 
    }
  }
  
  if(!exists){
    Option<VarInfo> ovi = mmoclass_.getVar(n);
    if(ovi){
      VarInfo vi = *ovi;
      ExpOptList oinds = vi.indices();
      // Multi dimensional variable
      if(oinds){
        ExpList inds = *oinds;
  
        OrdCT<Interval> mi1;
        OrdCT<Interval>::iterator itmi1 = mi1.begin();
        int dim = 0;
        vector<NI1>::iterator itvc = vCount_.begin();
        vector<NI1> newvc;
        vector<NI1>::iterator itnew = newvc.begin();
   
        const VarSymbolTable auxt = mmoclass_.syms();
        EvalExpression evexp(auxt);
  
        foreach_(Expression e, inds){
          if(is<SubAll>(e) || is<Range>(e))
            ERROR("Ill-defined array");
   
          Real res = Apply(evexp, e);
          Interval i(*itvc, 1, *itvc + res - 1);
          if(!i.empty_()){
            itmi1 = mi1.insert(itmi1, i);
            ++itmi1;
            itnew = newvc.insert(itnew, *itvc + res);
            ++itnew;

            ++dim;
            ++itvc;
          }
          else{
            OrdCT<Interval> aux;
            mi1 = aux;
            break;
          }
        }
  
        MultiInterval mi(mi1);

        if(!mi1.empty()){
          for(; itvc != vCount_.end(); ++itvc){
            itnew = newvc.insert(itnew, *itvc);
            ++itnew;
          }
          set_vCount(newvc);
        }

        AtomSet as(mi);
        Set s;
        s.addAtomSet(as);
  
        SetVertex V(n, s);
        SetVertexDesc v = boost::add_vertex(G);
  
        G[v] = V;
        mires = mi;
      }
  
      // Uni dimensional variable
      else{
        vector<NI1>::iterator itvc = vCount_.begin();
        NI1 auxvc = *itvc;
        Interval i(auxvc, 1, auxvc);
        vector<NI1> newvc;
        vector<NI1>::iterator itnew = newvc.begin();        
        itnew = newvc.insert(itnew, auxvc + 1);
        ++itnew;
        ++itvc;

        while(itvc != vCount_.end()){
          itnew = newvc.insert(itnew, *itvc);
          ++itnew;

          ++itvc;
        }
        set_vCount(newvc);

        MultiInterval mi;
        mi.addInter(i);
        AtomSet as(mi);
        Set s;
        s.addAtomSet(as);
  
        SetVertex V(n, s);
        SetVertexDesc v = boost::add_vertex(G);
 
        G[v] = V;
        mires = mi;
      }
    }
  }

  vnmtable_.insert(mires, n);
  nmvtable_.insert(n, mires);
  return mires;
}

// Check if only one variable is used at each subscript
bool Connectors::checkRanges(ExpOptList range1, ExpOptList range2){
  std::vector<Name> vars;
  std::vector<Name>::iterator itvars = vars.begin();
  std::pair<Name, VarInfo> aux;

  BOOST_FOREACH(aux, mmoclass_.syms()){
    itvars = vars.insert(itvars, get<0>(aux));
    ++itvars;
  }

  if(range1 && range2){
    ExpList r1 = range1.get();
    ExpList r2 = range2.get();
 
    if(r1.size() == 0 || r2.size() == 0)
      return true;

    else if(r1.size() != r2.size()){
      cerr << "Unmatched dimensions in equation connect" << endl;
      return false;
    }

    else{
      ExpList::iterator it1 = r1.begin(), it2 = r2.begin();

      while(it1 != r1.end()){
        foreach_(Name n1, vars){
          Reference r1(n1);
          Expression e1(r1);
          ContainsExpression co1(e1);
 
          bool cn11 = Apply(co1, *it1);
          bool cn21 = Apply(co1, *it2);

          // This loop checks that there is only one variable at each subscript
          foreach_(Name n2, vars){
            //cout << n1 << "; " << n2 << "; " << *it1 << "; " << *it2 << "\n";
            Reference r2(n2);
            Expression e2(r2);
            ContainsExpression co2(e2);

            bool cn12 = Apply(co2, *it1);
            bool cn22 = Apply(co2, *it2);

            if(((cn11 && cn12)) && (n1 != n2)){
              cerr << "Only one variable permitted at subscript\n";
              return false;
            }

            if(((cn21 && cn22)) && (n1 != n2)){
              cerr << "Only one variable permitted at subscript\n";
              return false;
            }

            if((cn11 && cn22) && (n1 != n2)){
              cerr << "Arrays should use the same counter for the i-th dimension\n";
              return false;
            }
          }
        }

        ++it1;
        ++it2;
      }
    }
  }

  return true;
}

Option<SetEdgeDesc> Connectors::existsEdge(SetVertexDesc d1, SetVertexDesc d2){
  EdgeIt ei, ei_end;
  boost::tie(ei, ei_end) = boost::edges(G);

  for(; ei != ei_end; ++ei){
    SetVertexDesc v1 = boost::source(*ei, G);
    SetVertexDesc v2 = boost::target(*ei, G);
    Option<SetEdgeDesc> e(*ei);

    if(v1 == d1 && v2 == d2)
      return e;

    if(v1 == d2 && v2 == d1)
      return e;
  }

  Option<SetEdgeDesc> e;
  return e;
}

void Connectors::updateGraph(SetVertexDesc d1, SetVertexDesc d2, 
                             MultiInterval mi1, MultiInterval mi2){
  OrdCT<Interval> ints1 = mi1.inters_();
  OrdCT<Interval>::iterator itints1 = ints1.begin();
  OrdCT<Interval> ints2 = mi2.inters_();
  OrdCT<Interval>::iterator itints2 = ints2.begin();

  NI1 misz1 = mi1.size();
  NI1 misz2 = mi2.size();

  if((mi1.ndim_() == mi2.ndim_()) || misz1 == 1  || misz2 == 1){
    OrdCT<NI2> g1;
    OrdCT<NI2>::iterator itg1 = g1.begin();
    OrdCT<NI2> o1;
    OrdCT<NI2>::iterator ito1 = o1.begin();
    OrdCT<NI2> g2;
    OrdCT<NI2>::iterator itg2 = g2.begin();
    OrdCT<NI2> o2;
    OrdCT<NI2>::iterator ito2 = o2.begin();
    OrdCT<Interval> mi;
    OrdCT<Interval>::iterator itmi = mi.begin(); 

    vector<NI1>::iterator itec = eCount1_.begin(); 
    vector<NI1> newec;
    vector<NI1>::iterator itnew = newec.begin();

    while(itints1 != ints1.end()){
      NI1 sz1 = (*itints1).size();
      NI1 sz2 = (*itints2).size();

      if(sz1 == sz2 || sz1 == 1 || sz2 == 1){
        NI1 count = max(sz1, sz2);
        NI1 auxec = *itec;
        Interval i(auxec, 1, auxec + count - 1);
        itmi = mi.insert(itmi, i);
        ++itmi;

        NI2 g1i = (*itints1).step_();
        NI2 o1i = (-g1i) * auxec + (*itints1).lo_();

        NI2 g2i = (*itints2).step_();
        NI2 o2i = (-g2i) * auxec + (*itints2).lo_();

        if(sz1 == 1){
          itg1 = g1.insert(itg1, 0);
          ito1 = o1.insert(ito1, (*itints1).lo_());
        }

        else{
          itg1 = g1.insert(itg1, g1i);
          ito1 = o1.insert(ito1, o1i);
        }

        if(sz2 == 1){
          itg2 = g2.insert(itg2, 0);
          ito2 = o2.insert(ito2, (*itints2).lo_());
        }

        else{
          itg2 = g2.insert(itg2, g2i);
          ito2 = o2.insert(ito2, o2i);
        }
 
        itnew = newec.insert(itnew, auxec + count);
        ++itnew;

        ++itg1;
        ++ito1;
        ++itg2;
        ++ito2;
        ++itec;
      }
 
      else{ 
        cerr << "Incompatible connect\n"; 
        newec = eCount1_;
        break;
      }

      ++itints1;
      ++itints2;
    }

    set_eCount1(newec);

    AtomSet as(mi);
    Set s;
    s.addAtomSet(as); 

    LMap lm1(g1, o1);
    LMap lm2(g2, o2);
 
    OrdCT<Set> cts1;
    cts1.insert(cts1.end(), s);
    OrdCT<LMap> ctlm1;
    ctlm1.insert(ctlm1.end(), lm1); 
    PWLMap e1(cts1, ctlm1);

    OrdCT<Set> cts2;
    cts2.insert(cts2.end(), s);
    OrdCT<LMap> ctlm2;
    ctlm2.insert(ctlm2.end(), lm2); 
    PWLMap e2(cts2, ctlm2);

    string enm = "E" + to_string(eCount2_);
    SetEdge E(enm, e1, e2);
    SetEdgeDesc e;
    bool b;
    boost::tie(e, b) = boost::add_edge(d1, d2, G);
    G[e] = E;
    ++eCount2_;
  }

  else
    cerr << "Incompatible connect\n";
}

void Connectors::generateCode(PWLMap pw){
  EquationList res;
  EquationList::iterator itres = res.begin();

  Set vcdom = pw.wholeDom();
  Set vcim = pw.image(vcdom);

  ExpList nms;
  ExpList::iterator itnms = nms.begin();
  int ascii = 0;
  for(unsigned int i = 0; i < vCount_.size(); ++i){
    string s(1, 105 + ascii);
    Expression nm(s);
    itnms = nms.insert(itnms, nm);
    ++itnms;
    ++ascii;
  }

  foreach_(AtomSet as, vcim.asets_()){
    Set auxs;
    auxs.addAtomSet(as);
    Set vcdomi = pw.preImage(auxs);
    Set vcdomiaux = vcdomi.diff(auxs);

    // Variables of equality in the ForEq
    Pair<vector<Name>, vector<Name>> p = separateVars();

    foreach_(AtomSet auxi, vcdomiaux.asets_()){
      MultiInterval mi = auxi.aset_();

      // Effort vars
      IndexList ran1;
      IndexList::iterator itran1 = ran1.begin(); 
      OrdCT<NI1> off1 = getOff(mi);
      MultiInterval mirange1 = applyOff(mi, off1);
      itnms = nms.begin();
      // Range of ForEq
      foreach_(Interval i, mirange1.inters_()){
        Expression elo(i.lo_());
        Expression est(i.step_());
        Expression ehi(i.hi_());
        Range auxr(elo, est, ehi);
        Expression auxer(auxr);
        Option<Expression> r(auxer); 

        Name s = get<Name>(*itnms);
        Index ind(s, r);
        itran1 = ran1.insert(itran1, ind);
        ++itran1;
        ++itnms;
      }
      Indexes range1(ran1);

      Set sauxi;
      sauxi.addAtomSet(auxi);
      vector<Name> effvars = get<0>(p);
      vector<Pair<Name, Name>> vars1 = getVars(effvars, sauxi);
      vector<Pair<Name, Name>>::iterator itv1 = vars1.begin();

      vector<Pair<Name, Name>> vars2 = getVars(effvars, auxs);
      vector<Pair<Name, Name>>::iterator itv2 = vars2.begin();

      itnms = nms.begin();
      OrdCT<NI1> off2 = getOff(as.aset_());
      MultiInterval auxmi1 = applyOff(as.aset_(), off2);
      Pair<ExpList, bool> tm1 = transMulti(mirange1, auxmi1, nms, false);
      ExpList inds1 = get<0>(tm1);

      for(; itv1 != vars1.end(); ++itv1){
        for(; itv2 != vars2.end(); ++itv2){
          if(get<1>(*itv1) == get<1>(*itv2)){
            ExpList auxnms1;

            const Name nm1 = get<0>(*itv1);
            Option<MultiInterval> omivar1 = nmvtable_[nm1];
            MultiInterval mivar1;
            if(omivar1)
              mivar1 = *omivar1;

            else
              ERROR("Should be a vertex");

            if(mivar1.size() != 1)
              auxnms1 = nms;

            Reference ref1(get<0>(*itv1) + get<1>(*itv1), auxnms1); // Left of equality
            Expression l(ref1);

            ExpList auxnms2;

            const Name nm2 = get<0>(*itv2);
            Option<MultiInterval> omivar2 = nmvtable_[nm2];
            MultiInterval mivar2;
            if(omivar2)
              mivar2 = *omivar2;

            else
              ERROR("Should be a vertex");

            if(mivar2.size() != 1)
              auxnms2 = inds1;

            Reference ref2(get<0>(*itv2) + get<1>(*itv2), auxnms2);
            Expression r(ref2);

            Equality eq1(l, r);

            EquationList auxeqlist1;
            auxeqlist1.insert(auxeqlist1.begin(), eq1);

            ForEq feq1(range1, auxeqlist1);

            itres = res.insert(itres, feq1);
            ++itres;
          }
        }
      }
    }
 
    // Flow vars
    IndexList ran2; 
    IndexList::iterator itran2 = ran2.begin();
    itnms = nms.begin();
    OrdCT<NI1> off3 = getOff(as.aset_());
    MultiInterval mirange2 = applyOff(as.aset_(), off3); 
    //Range of ForEq of flow vars
    foreach_(Interval i, mirange2.inters_()){
      Expression elo(i.lo_());
      Expression est(i.step_());
      Expression ehi(i.hi_());
      Range auxr(elo, est, ehi);
      Expression auxer(auxr);
      Option<Expression> r(auxer); 

      string auxvar = boost::get<Name>(*itnms);
      Index ind(auxvar, r);
      itran2 = ran2.insert(itran2, ind);
      ++itran2;
      ++itnms;
    }
    Indexes range2(ran2);

    vector<Name> flowvars = get<1>(p);

    ExpList exps;
    ExpList::iterator itexps = exps.begin();

    foreach_(AtomSet auxi, vcdomi.asets_()){
      OrdCT<NI1> off4 = getOff(auxi.aset_());
      MultiInterval auxmi2 = applyOff(auxi.aset_(), off4); 
      Pair<ExpList, bool> tm2 = transMulti(mirange2, auxmi2, nms, true);
      ExpList inds2 = get<0>(tm2);

      Set sauxi;
      sauxi.addAtomSet(auxi);
      vector<Pair<Name, Name>> vars3 = getVars(flowvars, sauxi);
      vector<Pair<Name, Name>>::iterator itv3 = vars3.begin();

      for(; itv3 != vars3.end(); ++itv3){
        Expression auxexp;

        ExpList auxnms3;

        const Name nm3 = get<0>(*itv3);
        Option<MultiInterval> omivar3 = nmvtable_[nm3];
        MultiInterval mivar3;
        if(omivar3)
          mivar3 = *omivar3;

        else
          ERROR("Should be a vertex");

        if(mivar3.size() != 1)
           auxnms3 = inds2;

        if(get<1>(tm2)){
          RefTuple rt(get<0>(*itv3) + get<1>(*itv3), auxnms3);
          AddAll aa(rt);
          Expression eaa(aa);
          auxexp = eaa; 
        }

        else{
          Reference ref3(get<0>(*itv3) + get<1>(*itv3), auxnms3); 
          Expression eref3(ref3);
          auxexp = eref3;
        }

        itexps = exps.insert(itexps, auxexp); 
        ++itexps;
      }
    }

    Expression flowexp;

    if(exps.size() != 0){
      itexps = exps.begin();
      flowexp = *itexps;
      ++itexps;
      // Add all flow vars
      for(; itexps != exps.end(); ++itexps){
        BinOp bop(flowexp, Add, *itexps);
        Expression ebop(bop);
        flowexp = ebop;
      }
    }

    Expression zero(0);

    Equality eq2(flowexp, zero);

    EquationList auxeqlist2;
    auxeqlist2.insert(auxeqlist2.begin(), eq2);

    ForEq feq2(range2, auxeqlist2);

    itres = res.insert(itres, feq2);
    ++itres;
  }    

  EquationList eql = simplifyCode(res); // Remove repeated equations
  // Add new equations of connects. Keep non-connect equations
  // as they were
  foreach_(Equation eqi, eql){
    itold = oldeqs.insert(itold, eqi);
    ++itold;
  }
  EquationSection eqres(false, oldeqs);
  mmoclass_.set_equations(eqres);
}

OrdCT<NI1> Connectors::getOff(MultiInterval mi){
  OrdCT<NI1> res;
  typename std::map<Name, MultiInterval>::iterator it;

  for(it = nmvtable_.begin(); it != nmvtable_.end(); ++it){
    MultiInterval aux = mi.cap(it->second);
    if(!aux.empty())
      res = (it->second).minElem();
  }

  return res;
}

Pair<vector<Name>, vector<Name>> Connectors::separateVars(){
  vector<Name> vars1;
  vector<Name>::iterator itvars1 = vars1.begin();
  vector<Name> vars2;
  vector<Name>::iterator itvars2 = vars2.begin();

  foreach_(Name n, mmoclass_.variables()){
    if(!isFlowVar(n)){
      itvars1 = vars1.insert(itvars1, n);
      ++itvars1;
    }
 
    else{
      itvars2 = vars2.insert(itvars2, n);
      ++itvars2;
    }
  }

  Pair<vector<Name>, vector<Name>> p(vars1, vars2);
  return p;
} 

bool Connectors::isFlowVar(Name n){
  Option<VarInfo> ovi = mmoclass_.syms_ref()[n];
  if(ovi){
    VarInfo vi = *ovi;
    TypePrefixes::iterator ittp = vi.prefixes_ref().begin(); 
    for(; ittp != vi.prefixes_ref().end(); ++ittp){
      Option<TypePrefix> otp = *ittp;
      if(otp){
        TypePrefix tp = *otp;
        if(tp == flow)
          return true;
      }
    }
  }

  return false;
}

vector<Pair<Name, Name>> Connectors::getVars(vector<Name> vs, Set sauxi){
  VertexIt vi, vi_end;
  boost::tie(vi, vi_end) = boost::vertices(G);
  vector<Pair<Name, Name>> vars;
  vector<Pair<Name, Name>>::iterator itvars = vars.begin();

  for(; vi != vi_end; ++vi){
    Set v = (G[*vi]).vs_();
    Name nm = (G[*vi]).name;

    if(!(v.cap(sauxi)).empty()){
      foreach_(Name n, vs){
        if(n.length() == nm.length()){
          Name suffix;
          Pair<Name, Name> p(nm, suffix);
          itvars = vars.insert(itvars, p);
          ++itvars;
        }

        else if(n.length() > nm.length()){
          if(n.substr(0, nm.length()) == nm && n.substr(nm.length(), 1) == "_"){
            Name suffix = n.substr(nm.length());
            Pair<Name, Name> p(nm, suffix);
            itvars = vars.insert(itvars, p);
            ++itvars;
          }
        }
      }
    }
  }

  return vars;
}

Pair<ExpList, bool> Connectors::transMulti(MultiInterval mi1, MultiInterval mi2, 
                                           ExpList nms, bool forFlow){
  ExpList res;
  ExpList::iterator itres = res.begin();
  bool bres = false;

  ExpList::iterator itnms = nms.begin();
  OrdCT<Interval> mi22 = mi2.inters_();
  OrdCT<Interval>::iterator itmi2 = mi22.begin(); 

  if(mi1.ndim_() == mi2.ndim_()){
    foreach_(Interval i1, mi1.inters_()){
      NI2 m3;
      NI2 h3;

      Expression x;
      if(is<Name>(*itnms))
        x = *itnms;

      else{
        ERROR("Should be a name");
        ExpList auxres;
        Pair<ExpList, bool> pres(auxres, false);
        return pres;
      }

      if(i1.size() == (*itmi2).size()){
        m3 = (NI2) (*itmi2).step_() / i1.step_();
        h3 = (-m3) * i1.lo_() + (*itmi2).lo_();

        BinOp multaux(m3, Mult, x);
        Expression mult(multaux);
        Expression h(h3);
        BinOp linearaux(mult, Add, h);
        Expression linear(linearaux);

        itres = res.insert(itres, linear);
        ++itres;
      }

      else if((*itmi2).size() == 1 && !forFlow){
        m3 = 0;
        h3 = (*itmi2).lo_();

        Expression linear(h3);

        itres = res.insert(itres, linear);
        ++itres;
      }

      else if(i1.size() == 1 && forFlow){
        Expression lo((*itmi2).lo_());
        Expression st((*itmi2).step_());
        Expression hi((*itmi2).hi_());
        Range r(lo, st, hi);
        Expression er(r);

        itres = res.insert(itres, er);
        ++itres;

        bres = true;
      }

      else{
        ERROR("Operation not allowed");
        ExpList auxres;
        Pair<ExpList, bool> pres(auxres, false);
        return pres;
      }

      ++itmi2; 
      ++itnms;
    }
  }

  Pair<ExpList, bool> pres(res, bres);
  return pres;
}

MultiInterval Connectors::applyOff(MultiInterval mi, OrdCT<NI1> off){
  OrdCT<Interval> res;
  OrdCT<Interval>::iterator itres = res.begin();

  OrdCT<NI1>::iterator itoff = off.begin();

  if(mi.ndim_() == (int) off.size()){
    foreach_(Interval i, mi.inters_()){
      Interval iaux(i.lo_() - (*itoff) + 1, i.step_(), i.hi_() - (*itoff) + 1);

      itres = res.insert(itres, iaux);
      ++itres;
 
      ++itoff;
    }
  }

  MultiInterval mires(res);
  return mires;
}

EquationList Connectors::simplifyCode(EquationList &eql){
  EquationList res;
  EquationList::iterator itres = res.begin();

  foreach_(Equation eq1, eql){
    bool found = false;
    foreach_(Equation eq2, res){
      if(eq1 == eq2)
        found = true;
    }
    
    if(!found){
      itres = res.insert(itres, eq1);
      ++itres;
    }
  } 

  return res;
}


