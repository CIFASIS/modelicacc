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

#include <util/graph/sbg/sbg.h>
#include <util/graph/sbg/sbg_printer.h>

using namespace SBG;

int main(){
  float offSp = 0;
  float offSn = 1;
  float offGp = 2;
  float offRp = 10000000;
  float offRn = 2 * offRp;
  float offCp = 3 * offRp;
  float offCn = 4 * offRp;

  Interval i1(1 + offSp, 1, 1 + offSp);
  Interval i2(1 + offSn, 1, 1 + offSn);
  Interval i3(1 + offGp, 1, 1 + offGp);
  Interval i4(1 + offRp, 1, offRp + offRp);
  Interval i5(1 + offRn, 1, offRp + offRn);
  Interval i6(1 + offCp, 1, offRp + offCp);
  Interval i7(1 + offCn, 1, offRp + offCn);

  MultiInterval mi1;
  mi1.addInter(i1);
  AtomSet as1(mi1);
  Set sp;
  sp.addAtomSet(as1);
  SetVertex V1("V1", 1, sp, 0);

  MultiInterval mi2;
  mi2.addInter(i2);
  AtomSet as2(mi2);
  Set sn;
  sn.addAtomSet(as2);
  SetVertex V2("V2", 2, sn, 0);

  MultiInterval mi3;
  mi3.addInter(i3);
  AtomSet as3(mi3);
  Set gp;
  gp.addAtomSet(as3);
  SetVertex V3("V3", 3, gp, 0);

  MultiInterval mi4;
  mi4.addInter(i4);
  AtomSet as4(mi4);
  Set rp;
  rp.addAtomSet(as4);
  SetVertex V4("V4", 4, rp, 0);

  MultiInterval mi5;
  mi5.addInter(i5);
  AtomSet as5(mi5);
  Set rn;
  rn.addAtomSet(as5);
  SetVertex V5("V5", 5, rn, 0);

  MultiInterval mi6;
  mi6.addInter(i6);
  AtomSet as6(mi6);
  Set cp;
  cp.addAtomSet(as6);
  SetVertex V6("V6", 6, cp, 0);

  MultiInterval mi7;
  mi7.addInter(i7);
  AtomSet as7(mi7);
  Set cn;
  cn.addAtomSet(as7);
  SetVertex V7("V7", 7, cn, 0);

  float offE1 = 0;
  float offE2 = 1;
  float offE3 = 2;
  float offE4 = 2 + offRp;
  float offE5 = 2 + 2 * offRp - 1;

  Interval i8(1 + offE1, 1, offE2);
  MultiInterval mi8;
  mi8.addInter(i8);
  AtomSet as8(mi8);
  Set domE1;
  domE1.addAtomSet(as8);
  LMap lm1;
  lm1.addGO(0, 1 + offSp);
  LMap lm2;
  lm2.addGO(0, 1 + offRp);
  PWLMap mapE1sp;
  mapE1sp.addSetLM(domE1, lm1);
  PWLMap mapE1rp;
  mapE1rp.addSetLM(domE1, lm2);
  SetEdge E1("E1", 1, mapE1sp, mapE1rp, 0);
 
  Interval i9(1 + offE2, 1, offE3); 
  MultiInterval mi9;
  mi9.addInter(i9);
  AtomSet as9(mi9);
  Set domE2;
  domE2.addAtomSet(as9);
  LMap lm3;
  lm3.addGO(0, 1 + offSn);
  LMap lm4;
  lm4.addGO(0, 1 + offGp);
  PWLMap mapE2sn;
  mapE2sn.addSetLM(domE2, lm3);
  PWLMap mapE2gp;
  mapE2gp.addSetLM(domE2, lm4);
  SetEdge E2("E2", 2, mapE2sn, mapE2gp, 0);

  Interval i10(1 + offE3, 1, offE4);
  MultiInterval mi10;
  mi10.addInter(i10);
  AtomSet as10(mi10);
  Set domE3;
  domE3.addAtomSet(as10);
  LMap lm5;
  lm5.addGO(1, offRn - offE3);
  LMap lm6;
  lm6.addGO(1, offCp - offE3);
  PWLMap mapE3rn;
  mapE3rn.addSetLM(domE3, lm5);
  PWLMap mapE3cp;
  mapE3cp.addSetLM(domE3, lm6);
  SetEdge E3("E3", 3, mapE3rn, mapE3cp, 0);

  Interval i11(1 + offE4, 1, offE5);
  MultiInterval mi11;
  mi11.addInter(i11);
  AtomSet as11(mi11);
  Set domE4;
  domE4.addAtomSet(as11);
  LMap lm7;
  lm7.addGO(1, offRn - offE4);
  LMap lm8;
  lm8.addGO(1, 1 + offRp - offE4);
  PWLMap mapE4rn;
  mapE4rn.addSetLM(domE4, lm7);
  PWLMap mapE4rp;
  mapE4rp.addSetLM(domE4, lm8);
  SetEdge E4("E4", 4, mapE4rn, mapE4rp, 0);

  Interval i12(1 + offE5, 1, 1 + offE5 + offRp - 1); 
  MultiInterval mi12;
  mi12.addInter(i12);
  AtomSet as12(mi12);
  Set domE5;
  domE5.addAtomSet(as12);
  LMap lm9;
  lm9.addGO(1, offCn - offE5);
  LMap lm10;
  lm10.addGO(0, 1 + offGp);
  PWLMap mapE5cn;
  mapE5cn.addSetLM(domE5, lm9);
  PWLMap mapE5gp;
  mapE5gp.addSetLM(domE5, lm10);
  SetEdge E5("E5", 5, mapE5cn, mapE5gp, 0);

  SBGraph g1;

  SetVertexDesc v1 = boost::add_vertex(g1);
  SetVertexDesc v2 = boost::add_vertex(g1);
  SetVertexDesc v3 = boost::add_vertex(g1);
  SetVertexDesc v4 = boost::add_vertex(g1);
  SetVertexDesc v5 = boost::add_vertex(g1);
  SetVertexDesc v6 = boost::add_vertex(g1);
  SetVertexDesc v7 = boost::add_vertex(g1);

  g1[v1] = V1;
  g1[v2] = V2;
  g1[v3] = V3;
  g1[v4] = V4;
  g1[v5] = V5;
  g1[v6] = V6;
  g1[v7] = V7;

  SetEdgeDesc e1;
  bool b1;
  boost::tie(e1, b1) = boost::add_edge(v1, v4, g1);
  SetEdgeDesc e2; 
  bool b2;
  boost::tie(e2, b2) = boost::add_edge(v2, v3, g1);
  SetEdgeDesc e3;
  bool b3;
  boost::tie(e3, b3) = boost::add_edge(v5, v6, g1);
  SetEdgeDesc e4;
  bool b4;
  boost::tie(e4, b4) = boost::add_edge(v5, v4, g1);
  SetEdgeDesc e5;
  bool b5;
  boost::tie(e5, b5) = boost::add_edge(v7, v3, g1);

  g1[e1] = E1;
  g1[e2] = E2;
  g1[e3] = E3;
  g1[e4] = E4;
  g1[e5] = E5;

  GraphPrinter gp1(g1, -1);
  std::string fn1("graph1.dot");
  gp1.printGraph(fn1);

  /**********************************************************************/
 
  float offsp = 0;
  float offsn = 1;
  float offgp = 2;
  float offrp = 1000;
  float offrn = 2 * offrp;
  float offcp = 3 * offrp;
  float offcn = 4 * offrp;

  Interval ii1(1 + offsp, 1, 1 + offsp);
  MultiInterval mii1;
  mii1.addInter(ii1);
  AtomSet ass1(mii1);
  Set ssp;
  ssp.addAtomSet(ass1);
  SetVertex VV1("V1", 1, ssp, 0);

  Interval ii2(1 + offsn, 1, 1 + offsn);
  MultiInterval mii2;
  mii2.addInter(ii2);
  AtomSet ass2(mii2);
  Set ssn;
  ssn.addAtomSet(ass2);
  SetVertex VV2("V2", 2, ssn, 0);

  Interval ii3(1 + offgp, 1, 1 + offgp);
  MultiInterval mii3;
  mii3.addInter(ii3);
  AtomSet ass3(mii3);
  Set ggp;
  ggp.addAtomSet(as3);
  SetVertex VV3("V3", 3, ggp, 0);

  Interval ii4(1 + offrp, 1, offrp + offrp);
  MultiInterval mii4;
  mii4.addInter(ii4);
  AtomSet ass4(mii4);
  Set rrp;
  rrp.addAtomSet(ass4);
  SetVertex VV4("V4", 4, rrp, 0);

  Interval ii5(1 + offrn, 1, offrp + offrn);
  MultiInterval mii5;
  mii5.addInter(ii5);
  AtomSet ass5(mii5);
  Set rrn;
  rrn.addAtomSet(ass5);
  SetVertex VV5("V5", 5, rrn, 0);

  Interval ii6(1 + offcp, 1, offrp + offcp);
  MultiInterval mii6;
  mii6.addInter(ii6);
  AtomSet ass6(mii6);
  Set ccp;
  ccp.addAtomSet(ass6);
  SetVertex VV6("V6", 6, ccp, 0);

  Interval ii7(1 + offcn, 1, offrp + offcn);
  MultiInterval mii7;
  mii7.addInter(ii7);
  AtomSet ass7(mii7);
  Set ccn;
  ccn.addAtomSet(ass7);
  SetVertex VV7("V7", 7, ccn, 0);

  float offEE1 = 0;
  float offEE2 = 1;
  float offEE3 = 2;
  float offEE4 = 2 + offrp;
  float offEE5 = 2 + 2 * offrp - 1;
  float offEE6 = 2 + 2 * offrp;

  Interval ii8(1 + offEE1, 1, offEE2);
  MultiInterval mii8;
  mii8.addInter(ii8);
  AtomSet ass8(mii8);
  Set domEE1;
  domEE1.addAtomSet(ass8);
  LMap llm1;
  llm1.addGO(0, 1 + offsp);
  LMap llm2;
  llm2.addGO(0, 1 + offrp);
  PWLMap mmapE1sp;
  mmapE1sp.addSetLM(domEE1, llm1);
  PWLMap mmapE1rp;
  mmapE1rp.addSetLM(domEE1, llm2);
  SetEdge EE1("E1", 1, mmapE1sp, mmapE1rp, 0);

  Interval ii9(1 + offEE2, 1, offEE3);
  MultiInterval mii9;
  mii9.addInter(ii9);
  AtomSet ass9(mii9);
  Set domEE2;
  domEE2.addAtomSet(ass9);
  LMap llm3;
  llm3.addGO(0, 1 + offsn);
  LMap llm4;
  llm4.addGO(0, 1 + offgp);
  PWLMap mmapE2sn;
  mmapE2sn.addSetLM(domEE2, llm3);
  PWLMap mmapE2gp;
  mmapE2gp.addSetLM(domEE2, llm4);
  SetEdge EE2("E2", 2, mmapE2sn, mmapE2gp, 0);

  Interval ii10(1 + offEE3, 1, offEE4);
  MultiInterval mii10;
  mii10.addInter(ii10);
  AtomSet ass10(mii10);
  Set domEE3;
  domEE3.addAtomSet(ass10);
  LMap llm5;
  llm5.addGO(1, offrn - offEE3); 
  LMap llm6;
  llm6.addGO(1, offcp - offEE3);
  PWLMap mmapE3rn;
  mmapE3rn.addSetLM(domEE3, llm5);
  PWLMap mmapE3cp;
  mmapE3cp.addSetLM(domEE3, llm6);
  SetEdge EE3("E3", 3, mmapE3rn, mmapE3cp, 0);

  Interval ii11(1 + offEE4, 1, offEE5);
  MultiInterval mii11;
  mii11.addInter(ii11);
  AtomSet ass11(mii11);
  Set domEE4;
  domEE4.addAtomSet(ass11);
  LMap llm7;
  llm7.addGO(1, offrn - offEE4);
  LMap llm8;
  llm8.addGO(1, 1 + offrp - offEE4);
  PWLMap mmapE4rn;
  mmapE4rn.addSetLM(domEE4, llm7);
  PWLMap mmapE4rp;
  mmapE4rp.addSetLM(domEE4, llm8);
  SetEdge EE4("E4", 4, mmapE4rn, mmapE4rp, 0);

  Interval ii12(1 + offEE5, 1, offEE6);
  MultiInterval mii12;
  mii12.addInter(ii12);
  AtomSet ass12(mii12);
  Set domEE5;
  domEE5.addAtomSet(ass12);
  LMap llm9;
  llm9.addGO(1, offcn - offEE5);
  LMap llm10;
  llm10.addGO(0, 1 + offgp);
  PWLMap mmapE5cn;
  mmapE5cn.addSetLM(domEE5, llm9);
  PWLMap mmapE5gp;
  mmapE5gp.addSetLM(domEE5, llm10);
  SetEdge EE5("E5", 1, mmapE5cn, mmapE5gp, 0);

  Interval ii13(1 + offEE6, 1, 1 + offEE6 + offrp - 2);
  MultiInterval mii13;
  mii13.addInter(ii13);
  AtomSet ass13(mii13);
  Set domEE6;
  domEE6.addAtomSet(ass13);
  LMap llm11;
  llm11.addGO(1, offcn - offEE6);
  LMap llm12;
  llm12.addGO(1, offcn - offEE6 + 1);
  PWLMap mmapE6cn;
  mmapE6cn.addSetLM(domEE6, llm11);
  PWLMap mmapE6cn2;
  mmapE6cn2.addSetLM(domEE6, llm12); 
  SetEdge EE6("E6", 1, mmapE6cn, mmapE6cn2, 0);

  SBGraph g2;

  SetVertexDesc vv1 = boost::add_vertex(g2);
  SetVertexDesc vv2 = boost::add_vertex(g2);
  SetVertexDesc vv3 = boost::add_vertex(g2);
  SetVertexDesc vv4 = boost::add_vertex(g2);
  SetVertexDesc vv5 = boost::add_vertex(g2);
  SetVertexDesc vv6 = boost::add_vertex(g2);
  SetVertexDesc vv7 = boost::add_vertex(g2);

  g2[vv1] = VV1;
  g2[vv2] = VV2;
  g2[vv3] = VV3;
  g2[vv4] = VV4;
  g2[vv5] = VV5;
  g2[vv6] = VV6;
  g2[vv7] = VV7;

  SetEdgeDesc ee1;
  bool bb1;
  boost::tie(ee1, bb1) = boost::add_edge(vv1, vv4, g2);
  SetEdgeDesc ee2; 
  bool bb2;
  boost::tie(ee2, bb2) = boost::add_edge(vv2, vv3, g2);
  SetEdgeDesc ee3;
  bool bb3;
  boost::tie(ee3, bb3) = boost::add_edge(vv5, vv6, g2);
  SetEdgeDesc ee4;
  bool bb4;
  boost::tie(ee4, bb4) = boost::add_edge(vv5, vv4, g2);
  SetEdgeDesc ee5;
  bool bb5;
  boost::tie(ee5, bb5) = boost::add_edge(vv7, vv3, g2);
  SetEdgeDesc ee6;
  bool bb6;
  boost::tie(ee6, bb6) = boost::add_edge(vv7, vv7, g2);

  g2[ee1] = EE1;
  g2[ee2] = EE2;
  g2[ee3] = EE3;
  g2[ee4] = EE4;
  g2[ee5] = EE5;
  g2[ee6] = EE6;

  GraphPrinter gp2(g2, -1);
  std::string fn2("graph2.dot");
  gp2.printGraph(fn2);

  return 0;
}
