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

  Set sp = createSet(i1);
  SetVertex V1("V1", 1, sp, 0);

  Set sn = createSet(i2);
  SetVertex V2("V2", 2, sn, 0);

  Set gp = createSet(i3);
  SetVertex V3("V3", 3, gp, 0);

  Set rp = createSet(i4);
  SetVertex V4("V4", 4, rp, 0);

  Set rn = createSet(i5);
  SetVertex V5("V5", 5, rn, 0);

  Set cp = createSet(i6);
  SetVertex V6("V6", 6, cp, 0);

  Set cn = createSet(i7);
  SetVertex V7("V7", 7, cn, 0);

  float offE1 = 0;
  float offE2 = 1;
  float offE3 = 2;
  float offE4 = 2 + offRp;
  float offE5 = 2 + 2 * offRp - 1;

  Interval i8(1 + offE1, 1, offE2);
  Set domE1 = createSet(i8);
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
  Set domE2 = createSet(i9);
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
  Set domE3 = createSet(i10);
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
  Set domE4 = createSet(i11);
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
  Set domE5 = createSet(i12);
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
  Set ssp = createSet(ii1);
  SetVertex VV1("V1", 1, ssp, 0);

  Interval ii2(1 + offsn, 1, 1 + offsn);
  Set ssn = createSet(ii2);
  SetVertex VV2("V2", 2, ssn, 0);

  Interval ii3(1 + offgp, 1, 1 + offgp);
  Set ggp = createSet(ii3);
  SetVertex VV3("V3", 3, ggp, 0);

  Interval ii4(1 + offrp, 1, offrp + offrp);
  Set rrp = createSet(ii4);
  SetVertex VV4("V4", 4, rrp, 0);

  Interval ii5(1 + offrn, 1, offrp + offrn);
  Set rrn = createSet(ii5);
  SetVertex VV5("V5", 5, rrn, 0);

  Interval ii6(1 + offcp, 1, offrp + offcp);
  Set ccp = createSet(ii6);
  SetVertex VV6("V6", 6, ccp, 0);

  Interval ii7(1 + offcn, 1, offrp + offcn);
  Set ccn = createSet(ii7);
  SetVertex VV7("V7", 7, ccn, 0);

  float offEE1 = 0;
  float offEE2 = 1;
  float offEE3 = 2;
  float offEE4 = 2 + offrp;
  float offEE5 = 2 + 2 * offrp - 1;
  float offEE6 = 2 + 2 * offrp;

  Interval ii8(1 + offEE1, 1, offEE2);
  Set domEE1 = createSet(ii8);
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
  Set domEE2 = createSet(ii9);
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
  Set domEE3 = createSet(ii10);
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
  Set domEE4 = createSet(ii11);
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
  Set domEE5 = createSet(ii12);
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
  Set domEE6 = createSet(ii13);
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
