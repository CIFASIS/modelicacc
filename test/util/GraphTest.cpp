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

#include <boost/test/unit_test.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/unordered_set.hpp>

#include <util/graph/graph_definition.h>
#include <util/graph/graph_printer.h>

using namespace boost::unit_test;

/// @brief If sb-graphs containers implementation changes (uses vector, for example)
/// this typedef should also change.
typedef UnordCT<Interval> contInt1;
typedef OrdCT<Interval> contInt2;
typedef UnordCT<MultiInterval> contMulti;
typedef UnordCT<AtomSet> contAS;

typedef OrdCT<NI1> contNI1;
typedef OrdCT<NI2> contNI2;

typedef OrdCT<Set> contSet1;
typedef OrdCT<LMap> contLM1;

//____________________________________________________________________________//

// -- Intervals --------------------------------------------------------------//

void TestIntCreation1(){
  Interval i(10, 3, 3);

  BOOST_CHECK(i.empty_() == true);
}

void TestIntCreation2(){
  Interval i(10, 20, 15);

  BOOST_CHECK(i.hi_() == 10);
}

void TestIntCreation3(){
  Interval i(10, 5, 23);

  BOOST_CHECK(i.hi_() == 20);
}

void TestIntCreation4(){
  Interval i(10, 1, Inf);

  BOOST_CHECK(i.hi_() == Inf);
}

void TestIntQuery1(){
  Interval i(10, 2, 20);

  BOOST_CHECK(!i.isIn(13));
}

void TestIntQuery2(){
  Interval i(10, 2, 20);

  BOOST_CHECK(i.isIn(18));
}

void TestIntQuery3(){
  Interval i(10, 2, 20);

  BOOST_CHECK(!i.isIn(100));
}

void TestIntQuery4(){
  Interval i1(10, 2, 20);
  Interval i2(0, 3, 25);

  bool b1 = i1.isIn(12);
  bool b2 = i2.isIn(12);

  BOOST_CHECK(b1 && b2);
}

void TestIntQuery5(){
  Interval i(true);

  BOOST_CHECK(!i.isIn(10));
}

// Cap should be commutative
void TestIntCap1(){
  Interval i1(10, 2, 20);
  Interval i2(0, 3, 25);

  Interval i3(i1.cap(i2));
  Interval i4(i2.cap(i1));

  BOOST_CHECK(i3 == i4); 
}

void TestIntCap2(){
  Interval i1(10, 2, 20);
  Interval i2(0, 3, 25);

  Interval i3 = i1.cap(i2);

  Interval i4(12, 6, 18);

  BOOST_CHECK(i3 == i4);
}

void TestIntCap3(){
  Interval i1(14, 2, 16);
  Interval i2(12, 3, 15);

  Interval i3 = i1.cap(i2);

  Interval i4(true);

  BOOST_CHECK(i3 == i4);
}

void TestIntCap4(){
  Interval i1(14, 2, 28);
  Interval i2(0, 1, Inf);

  Interval i3 = i1.cap(i2);
 
  Interval i4(14, 2, 28);

  BOOST_CHECK(i3 == i4);
}

void TestIntCap5(){
  Interval i1(1, 1, 10);
  
  Interval i2 = i1.cap(i1);

  BOOST_CHECK(i1 == i2);
}

void TestIntDiff1(){
  Interval i1(0, 2, 30);
  Interval i2(true);
  
  contInt1 res1 = i1.diff(i2);

  contInt1 res2;
  res2.insert(i1);

  BOOST_CHECK(res1 == res2);
}

void TestIntDiff2(){
  Interval i1(0, 2, 30);
  Interval i2(10, 3, 40);

  contInt1 res1 = i1.diff(i2);

  Interval i3(0, 2, 8);
  Interval i4(12, 6, 24);
  Interval i5(14, 6, 26);
  Interval i6(30, 2, 30);

  contInt1 res2; 
  res2.insert(i3);
  res2.insert(i4);
  res2.insert(i5);
  res2.insert(i6);

  BOOST_CHECK(res1 == res2);
}

void TestIntDiff3(){
  Interval i1(0, 2, Inf);
  Interval i2(10, 3, 40);

  contInt1 res1 = i1.diff(i2);

  Interval i3(0, 2, 8);
  Interval i4(12, 6, 36);
  Interval i5(14, 6, 38);
  Interval i6(42, 2, Inf);

  contInt1 res2; 
  res2.insert(i3);
  res2.insert(i4);
  res2.insert(i5);
  res2.insert(i6);

  BOOST_CHECK(res1 == res2);
}

void TestIntDiff4(){
  Interval i1(0, 1, 10);
  Interval i2(true);

  contInt1 res1 = i1.diff(i1);

  contInt1 res2;

  BOOST_CHECK(res1 == res2);
}

void TestIntMin1(){
  Interval i(10, 3, 40);

  NI1 res1 = i.minElem();

  BOOST_CHECK(res1 == 10);
}

// -- MultiIntervals --------------------------------------------------------------//
void TestMultiCreation1(){
  Interval i1(1, 1, 10);
  Interval i2(true);

  MultiInterval res1;
  res1.addInter(i1);
  res1.addInter(i2);
  res1.addInter(i2);

  BOOST_CHECK(!res1.empty());
}

void TestMultiCreation2(){
  Interval i1(1, 1, 10);
  Interval i2(0, 2, 50);
  Interval i3(3, 1, 5);
  Interval i4(3, 8, 24);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);
  mi1.addInter(i3);
  mi1.addInter(i4);

  contInt2 res;
  contInt2::iterator it = res.begin();
  it = res.insert(it, i1);
  ++it;
  it = res.insert(it, i2);
  ++it;
  it = res.insert(it, i3);
  ++it;
  res.insert(it, i4);

  MultiInterval mi2(res);

  BOOST_CHECK(mi1 == mi2);
}

void TestMultiEmpty1(){
  MultiInterval mi;

  BOOST_CHECK(mi.empty());
}

void TestMultiEmpty2(){
  Interval i1(true);
  Interval i2(true);
  Interval i3(true);

  MultiInterval mi;
  mi.addInter(i1);
  mi.addInter(i2);
  mi.addInter(i3);

  BOOST_CHECK(mi.empty());
}

void TestMultiEmpty3(){
  Interval i1(true);
  Interval i2(0, 1, 10);
  Interval i3(true);

  MultiInterval mi;
  mi.addInter(i1);
  mi.addInter(i2);
  mi.addInter(i3);

  BOOST_CHECK(!mi.empty());
}

void TestMultiQuery1(){
  Interval i1(1, 1, 10);
  Interval i2(true);
  Interval i3(10, 2, 21);

  MultiInterval mi;
  mi.addInter(i1);
  mi.addInter(i2);
  mi.addInter(i3);

  contNI1 elem1;
  contNI1::iterator it1 = elem1.begin();
  it1 = elem1.insert(it1, 5);
  ++it1;
  it1 = elem1.insert(it1, 10);
  ++it1;
  elem1.insert(it1, 21);

  BOOST_CHECK(!mi.isIn(elem1));
}

void TestMultiQuery2(){
  Interval i1(1, 1, 10);
  Interval i2(10, 20, 10);
  Interval i3(10, 2, 21);

  MultiInterval mi;
  mi.addInter(i1);
  mi.addInter(i2);
  mi.addInter(i3);

  contNI1 elem1;
  contNI1::iterator it1 = elem1.begin();
  it1 = elem1.insert(it1, 5);
  ++it1;
  it1 = elem1.insert(it1, 10);
  ++it1;
  elem1.insert(it1, 21);

  BOOST_CHECK(!mi.isIn(elem1));
}

void TestMultiQuery3(){
  Interval i1(1, 1, 10);
  Interval i2(10, 20, 10);
  Interval i3(10, 2, 21);

  MultiInterval mi;
  mi.addInter(i1);
  mi.addInter(i2);
  mi.addInter(i3);

  contNI1 elem1;
  contNI1::iterator it1 = elem1.begin();
  it1 = elem1.insert(it1, 5);
  ++it1;
  it1 = elem1.insert(it1, 10);
  ++it1;
  elem1.insert(it1, 20);

  BOOST_CHECK(mi.isIn(elem1));
}

void TestMultiAddInter1(){
  Interval i1(0, 2, 10);

  MultiInterval mi1;
  mi1.addInter(i1);  

  contInt2 ints2;
  contInt2::iterator it2 = ints2.begin();
  ints2.insert(it2, i1);

  MultiInterval mi2(ints2); 

  BOOST_CHECK(mi1 == mi2);
}

void TestMultiAddInter2(){
  Interval i1(0, 2, 10);
  Interval i2(30, 2, 40);
  Interval i3(25, 1, 30);

  MultiInterval mi1;
  mi1.addInter(i1);  
  mi1.addInter(i2);
  mi1.addInter(i3);

  contInt2 ints2;
  contInt2::iterator it2 = ints2.end();
  it2 = ints2.insert(it2, i1);
  ++it2;
  it2 = ints2.insert(it2, i2);
  ++it2;
  ints2.insert(it2, i3);

  MultiInterval mi2(ints2); 

  BOOST_CHECK(mi1 == mi2);
}

void TestMultiCap1(){
  Interval i1(0, 2, 20);
  Interval i2(30, 2, 40);
  Interval i3(25, 1, 30);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);
  mi1.addInter(i3);

  MultiInterval mi2;

  MultiInterval mi3 = mi1.cap(mi2);
  MultiInterval mi4 = mi2.cap(mi1);

  BOOST_CHECK(mi2 == mi3 && mi3 == mi4);
}

void TestMultiCap2(){
  Interval i1(0, 2, 20);
  Interval i2(30, 2, 40);
  Interval i3(25, 1, 30);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);
  mi1.addInter(i3);

  Interval i4(5, 3, 15);
  Interval i5(true);
  Interval i6(27, 1, 35);

  MultiInterval mi2;
  mi2.addInter(i4);
  mi2.addInter(i5);
  mi2.addInter(i6);

  contInt2 aux;

  MultiInterval res1(aux);

  MultiInterval res2 = mi1.cap(mi2);
  MultiInterval res3 = mi2.cap(mi1);
  
  BOOST_CHECK(res1 == res2 && res2 == res3);
}

void TestMultiCap3(){
  Interval i1(1, 1, 10);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i1);
  mi1.addInter(i1);

  Interval i2(30, 1, 40);

  MultiInterval mi2;
  mi2.addInter(i1);
  mi2.addInter(i1);
  mi2.addInter(i2);

  MultiInterval mi3 = mi1.cap(mi2);

  BOOST_CHECK(mi3.empty());
}

void TestMultiCap4(){
  Interval i1(1, 1, 10);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i1);
  mi1.addInter(i1);

  MultiInterval mi2 = mi1.cap(mi1);

  BOOST_CHECK(mi1 == mi2);
}

void TestMultiDiff1(){
  Interval i1(0, 2, 20);
  Interval i2(30, 2, 40);
  Interval i3(25, 1, 30);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);
  mi1.addInter(i3);

  Interval i4(5, 3, 15);
  Interval i5(30, 2, 30);
  Interval i6(27, 1, 35);

  MultiInterval mi2;
  mi2.addInter(i4);
  mi2.addInter(i5);
  mi2.addInter(i6);

  contMulti res1 = mi1.diff(mi2);
  contMulti::iterator it1 = res1.begin();

  Interval i7(0, 2, 6);
 
  MultiInterval mi3;
  mi3.addInter(i7);
  mi3.addInter(i2);
  mi3.addInter(i3);

  Interval i8(10, 6, 10);

  MultiInterval mi4;
  mi4.addInter(i8);
  mi4.addInter(i2);
  mi4.addInter(i3);

  Interval i9(12, 6, 12);

  MultiInterval mi5;
  mi5.addInter(i9);
  mi5.addInter(i2);
  mi5.addInter(i3);

  Interval i10(16, 2, 20);

  MultiInterval mi6;
  mi6.addInter(i10);
  mi6.addInter(i2);
  mi6.addInter(i3);

  Interval i11(8, 6, 14);
  Interval i12(32, 2, 40);

  MultiInterval mi7;
  mi7.addInter(i11);
  mi7.addInter(i12);
  mi7.addInter(i3);

  Interval i13(30, 2, 30);
  Interval i14(25, 1, 26);

  MultiInterval mi8;
  mi8.addInter(i11);
  mi8.addInter(i13);
  mi8.addInter(i14);

  contMulti res2;
  res2.insert(mi3);
  res2.insert(mi4);
  res2.insert(mi5);
  res2.insert(mi6);
  res2.insert(mi7);
  res2.insert(mi8);

  BOOST_CHECK(res1 == res2);
}

void TestMultiDiff2(){
  Interval i1(0, 2, 20);
  Interval i2(30, 2, 40);
  Interval i3(25, 1, 30);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);
  mi1.addInter(i3);

  Interval i4(5, 3, 15);
  Interval i5(30, 2, 30);
  Interval i6(25, 1, 35);

  MultiInterval mi2;
  mi2.addInter(i4);
  mi2.addInter(i5);
  mi2.addInter(i6);

  contMulti res1 = mi1.diff(mi2);
  contMulti::iterator it1 = res1.begin();

  Interval i7(0, 2, 6);
 
  MultiInterval mi3;
  mi3.addInter(i7);
  mi3.addInter(i2);
  mi3.addInter(i3);

  Interval i8(10, 6, 10);

  MultiInterval mi4;
  mi4.addInter(i8);
  mi4.addInter(i2);
  mi4.addInter(i3);

  Interval i9(12, 6, 12);

  MultiInterval mi5;
  mi5.addInter(i9);
  mi5.addInter(i2);
  mi5.addInter(i3);

  Interval i10(16, 2, 20);

  MultiInterval mi6;
  mi6.addInter(i10);
  mi6.addInter(i2);
  mi6.addInter(i3);

  Interval i11(8, 6, 14);
  Interval i12(32, 2, 40);

  MultiInterval mi7;
  mi7.addInter(i11);
  mi7.addInter(i12);
  mi7.addInter(i3);

  contMulti res2;
  res2.insert(mi3);
  res2.insert(mi4);
  res2.insert(mi5);
  res2.insert(mi6);
  res2.insert(mi7);

  BOOST_CHECK(res1 == res2);
}

void TestMultiDiff3(){
  Interval i1(true);
  Interval i2(30, 2, 40);
  Interval i3(25, 1, 30);
  
  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);
  mi1.addInter(i3);

  Interval i4(5, 3, 14);
  Interval i5(true);
  Interval i6(true);

  MultiInterval mi2;
  mi2.addInter(i4);
  mi2.addInter(i5);
  mi2.addInter(i6);

  contMulti res1 = mi1.diff(mi2);

  BOOST_CHECK(res1.empty());
}

void TestMultiDiff4(){
  Interval i1(1, 1, 10);
  Interval i2(20, 3, 33);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);

  contMulti res1 = mi1.diff(mi1);

  BOOST_CHECK(res1.empty());
}

void TestMultiDiff5(){
  Interval i1(1, 1, 10);
  Interval i2(2, 2, 20);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);

  Interval i3(1, 1, 15);
  Interval i4(2, 2, 40);

  MultiInterval mi2;
  mi2.addInter(i3);
  mi2.addInter(i4);

  contMulti res1 = mi1.diff(mi2);

  BOOST_CHECK(res1.empty());
}

void TestMultiCrossProd1(){
  Interval i1(1, 1, 10);
  Interval i2(2, 2, 40);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);

  Interval i3(3, 3, 20);
  Interval i4(1, 50, Inf);

  MultiInterval mi2;
  mi2.addInter(i3);
  mi2.addInter(i4);

  MultiInterval res1 = mi1.crossProd(mi2);

  MultiInterval res2;
  res2.addInter(i1);
  res2.addInter(i2);
  res2.addInter(i3);
  res2.addInter(i4);

  BOOST_CHECK(res1 == res2);
}

void TestMultiMin1(){
  Interval i1(0, 1, 40);
  Interval i2(15, 3, 18);
  Interval i3(50, 2, 70);

  MultiInterval mi;

  mi.addInter(i1);
  mi.addInter(i2);
  mi.addInter(i3);

  contNI1 res1 = mi.minElem();

  contNI1 res2;
  contNI1::iterator it2 = res2.begin();
 
  it2 = res2.insert(it2, 0);
  ++it2;
  it2 = res2.insert(it2, 15);
  ++it2;
  res2.insert(it2, 50);

  BOOST_CHECK(res1 == res2);
}

void TestMultiReplace1(){
  Interval i1(1, 1, 10);
  Interval i2(1, 2, 10);
  Interval i3(1, 3, 10);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);
  mi1.addInter(i3);

  Interval i4(1, 4, 10);

  MultiInterval res1 = mi1.replace(i4, 1);

  MultiInterval res2;
  res2.addInter(i4);
  res2.addInter(i2);
  res2.addInter(i3);

  BOOST_CHECK(res1 == res2);
}

void TestMultiReplace2(){
  Interval i1(1, 1, 10);
  Interval i2(1, 2, 10);
  Interval i3(1, 3, 10);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);
  mi1.addInter(i3);

  Interval i4(1, 4, 10);

  MultiInterval res1 = mi1.replace(i4, 4);

  MultiInterval res2;
  res2.addInter(i1);
  res2.addInter(i2);
  res2.addInter(i3);

  BOOST_CHECK(res1 == res2);
}

// -- AtomicSets --------------------------------------------------------------//

void TestASetCreation1(){
  Interval i1(true);
  Interval i2(0, 2, 50);
  Interval i3(3, 1, 5);
  Interval i4(3, 8, 24);

  MultiInterval mi;

  mi.addInter(i1);
  mi.addInter(i2);
  mi.addInter(i3);
  mi.addInter(i4);

  AtomSet as(mi);

  BOOST_CHECK(mi == as.aset_());
}

void TestASetEmpty1(){
  MultiInterval mi;

  BOOST_CHECK(mi.empty());
}

void TestASetEmpty2(){
  Interval i1(true);
  Interval i2(0, 2, 50);
  Interval i3(3, 1, 5);
  Interval i4(3, 8, 24);

  MultiInterval mi;

  mi.addInter(i1);
  mi.addInter(i2);
  mi.addInter(i3);
  mi.addInter(i4);

  AtomSet as(mi);

  BOOST_CHECK(!as.empty());
}

void TestASetEmpty3(){
  Interval i1(true);
  Interval i2(true);
  Interval i3(true);

  MultiInterval mi;
  mi.addInter(i1);
  mi.addInter(i2);
  mi.addInter(i3);

  AtomSet as(mi);

  BOOST_CHECK(mi.empty());
}

void TestASetEmpty4(){
  Interval i1(true);
  Interval i2(true);
  Interval i3(1, 1, 10);

  MultiInterval mi;
  mi.addInter(i1);
  mi.addInter(i2);
  mi.addInter(i3);

  AtomSet as(mi);

  BOOST_CHECK(!mi.empty());
}

void TestASetCap1(){
  Interval i1(0, 2, 20);
  Interval i2(30, 2, 40);
  Interval i3(25, 1, 30);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);
  mi1.addInter(i3);

  AtomSet as1(mi1);

  Interval i4(5, 3, 15);
  Interval i5(true);
  Interval i6(27, 1, 35);

  MultiInterval mi2;
  mi2.addInter(i4);
  mi2.addInter(i5);
  mi2.addInter(i6);

  AtomSet as2(mi2);

  AtomSet res1 = as1.cap(as2);
  AtomSet res2 = as2.cap(as1);

  AtomSet res3;

  BOOST_CHECK(res1 == res2 && res2 == res3);
}

void TestASetCap2(){
  Interval i1(1, 1, 10);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i1);
  mi1.addInter(i1);

  AtomSet as1(mi1);

  AtomSet as2 = as1.cap(as1);

  BOOST_CHECK(as1 == as2);
}

void TestASetDiff1(){
  Interval i1(0, 2, 20);
  Interval i2(30, 2, 40);
  Interval i3(25, 1, 30);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);
  mi1.addInter(i3);

  AtomSet as1(mi1);

  Interval i4(5, 3, 15);
  Interval i5(30, 2, 30);
  Interval i6(27, 1, 35);

  MultiInterval mi2;
  mi2.addInter(i4);
  mi2.addInter(i5);
  mi2.addInter(i6);

  AtomSet as2(mi2);

  contAS res1 = as1.diff(as2);

  Interval i7(0, 2, 6);

  MultiInterval mi3;
  mi3.addInter(i7);
  mi3.addInter(i2);
  mi3.addInter(i3);

  AtomSet as3(mi3);

  Interval i8(10, 6, 10);

  MultiInterval mi4;
  mi4.addInter(i8);
  mi4.addInter(i2);
  mi4.addInter(i3);

  AtomSet as4(mi4);

  Interval i9(12, 6, 12);

  MultiInterval mi5;
  mi5.addInter(i9);
  mi5.addInter(i2);
  mi5.addInter(i3);

  AtomSet as5(mi5);

  Interval i10(16, 2, 20);

  MultiInterval mi6;
  mi6.addInter(i10);
  mi6.addInter(i2);
  mi6.addInter(i3);

  AtomSet as6(mi6);

  Interval i11(8, 6, 14);
  Interval i12(32, 2, 40);

  MultiInterval mi7;
  mi7.addInter(i11);
  mi7.addInter(i12);
  mi7.addInter(i3);

  AtomSet as7(mi7);

  Interval i13(30, 2, 30);
  Interval i14(25, 1, 26);

  MultiInterval mi8;
  mi8.addInter(i11);
  mi8.addInter(i13);
  mi8.addInter(i14);

  AtomSet as8(mi8);

  contAS res2;
  res2.insert(as3);
  res2.insert(as4);
  res2.insert(as5);
  res2.insert(as6);
  res2.insert(as7);
  res2.insert(as8);

  BOOST_CHECK(res1 == res2);
}

void TestASetMin1(){
  Interval i1(0, 1, 40);
  Interval i2(15, 3, 18);
  Interval i3(50, 2, 70);

  MultiInterval mi;

  mi.addInter(i1);
  mi.addInter(i2);
  mi.addInter(i3);

  AtomSet as1(mi);

  contNI1 res1 = mi.minElem();

  contNI1 res2;
  contNI1::iterator it2 = res2.begin();
 
  it2 = res2.insert(it2, 0);
  ++it2;
  it2 = res2.insert(it2, 15);
  ++it2;
  res2.insert(it2, 50);

  BOOST_CHECK(res1 == res2);
}

// -- Sets -------------------------------------------------------------------//

void TestSetCreation1(){
  Interval i1(0, 2, 20);
  Interval i2(30, 2, 40);
  Interval i3(25, 1, 30);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);
  mi1.addInter(i3);

  AtomSet as1(mi1);

  Interval i4(0, 1, 10);

  MultiInterval mi2;
  mi2.addInter(i4);
  mi2.addInter(i4);
  mi2.addInter(i4);

  AtomSet as2(mi2);

  Set s1;
  s1.addAtomSet(as1);
  s1.addAtomSet(as2);

  contAS res2;
  res2.insert(as1);
  res2.insert(as2);

  Set s2(res2);

  BOOST_CHECK(s1 == s2);
}

void TestCompSets1(){
  Interval i1(0, 1, 10);
  
  MultiInterval mi1;
  mi1.addInter(i1);

  AtomSet as1(mi1);

  Set s1;
  s1.addAtomSet(as1);

  Interval i2(0, 1, 20);

  MultiInterval mi2;
  mi2.addInter(i2);

  AtomSet as2(mi2);

  Set s2;
  s2.addAtomSet(as2);
  
  BOOST_CHECK(!(s1 == s2));
}

void TestSetEmpty1(){
  Interval i7(0, 1, Inf);
  Interval i8(20, 3, 50);
  Interval i9(true);

  MultiInterval mi3;
  mi3.addInter(i7);
  mi3.addInter(i8);
  mi3.addInter(i9);

  AtomSet as3(mi3);

  Set s2;
  s2.addAtomSet(as3);

  BOOST_CHECK(!s2.empty());
}

void TestAddASets1(){
  Interval i1(0, 2, 20);
  Interval i2(30, 2, 40);
  Interval i3(25, 1, 30);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);
  mi1.addInter(i3);

  Interval i4(5, 3, 15);
  Interval i5(true);
  Interval i6(27, 1, 35);

  MultiInterval mi2;
  mi2.addInter(i4);
  mi2.addInter(i5);
  mi2.addInter(i6);

  AtomSet as1(mi1);
  AtomSet as2(mi2);

  Set s1;

  s1.addAtomSet(as1);
  s1.addAtomSet(as2);

  contAS aux;
  contAS::iterator itaux = aux.begin();
  aux.insert(itaux, as1);

  Set s2(aux);

  BOOST_CHECK(s1 == s2);
}

void TestSetCap1(){
  Set s1;
  Set s2;

  Set res1 = s1.cap(s2);
  Set res2 = s2.cap(s1);

  BOOST_CHECK(res1 == res2 && res1.empty() && res2.empty());
}

void TestSetCap2(){
  Set s1;

  Interval i1(0, 2, 20);
  Interval i2(30, 2, 40);
  Interval i3(25, 1, 30);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);
  mi1.addInter(i3);

  AtomSet as1(mi1);

  Set s2;

  s2.addAtomSet(as1);

  Set res1 = s1.cap(s2);
  Set res2 = s2.cap(s1);

  BOOST_CHECK(res1 == res2 && res1.empty() && res2.empty());
}

void TestSetCap3(){
  Interval i1(0, 2, 20);
  Interval i2(30, 2, 40);
  Interval i3(25, 1, 30);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);
  mi1.addInter(i3);

  Interval i4(5, 3, 15);
  Interval i5(35, 3, 40);
  Interval i6(27, 1, 35);

  MultiInterval mi2;
  mi2.addInter(i4);
  mi2.addInter(i5);
  mi2.addInter(i6);

  AtomSet as1(mi1);
  AtomSet as2(mi2);

  Set s1;
  s1.addAtomSet(as1);
  s1.addAtomSet(as2);

  Interval i7(0, 1, Inf);
  Interval i8(20, 3, 50);
  Interval i9(28, 1, 28);

  MultiInterval mi3;
  mi3.addInter(i7);
  mi3.addInter(i8);
  mi3.addInter(i9);

  AtomSet as3(mi3);

  Set s2;
  s2.addAtomSet(as3);

  Set res1 = s1.cap(s2);
  Set res2 = s2.cap(s1);

  Interval i10(0, 2, 20);
  Interval i11(32, 6, 38);

  MultiInterval mi4;
  mi4.addInter(i10);
  mi4.addInter(i11);
  mi4.addInter(i9);

  MultiInterval mi5;
  mi5.addInter(i4);
  mi5.addInter(i5);
  mi5.addInter(i9);

  AtomSet as4(mi4);
  AtomSet as5(mi5);

  Set res3;

  res3.addAtomSet(as4);
  res3.addAtomSet(as5);

  BOOST_CHECK(res1 == res2 && res2 == res3);
}

void TestSetCap4(){
  Interval i1(1, 1, 10);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i1);
  mi1.addInter(i1);

  AtomSet as1(mi1);

  Interval i2(15, 1, 20);

  MultiInterval mi2;
  mi2.addInter(i2);
  mi2.addInter(i2);
  mi2.addInter(i2);

  AtomSet as2(mi2);

  Set s1;
  s1.addAtomSet(as1);
  s1.addAtomSet(as2);

  Set s2 = s1.cap(s1);

  BOOST_CHECK(s1 == s2);
}

void TestSetDiff1(){
  Interval i1(0, 1, 10);
  Interval i2(0, 3, 9);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);

  AtomSet as1(mi1);

  Set s1;
  s1.addAtomSet(as1);

  Interval i3(0, 1, 10);
  Interval i4(0, 3, 9);

  MultiInterval mi2;
  mi2.addInter(i3);
  mi2.addInter(i4);

  AtomSet as2(mi2);

  Set s2;
  s2.addAtomSet(as2);
 
  Set res1 = s1.diff(s2); 
  Set res2;

  BOOST_CHECK(res1 == res2);
}

void TestSetMin1(){
  Interval i1(true);
  Interval i2(5, 1, 10);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);

  AtomSet as1(mi1);

  Interval i3(20, 20, 80);
  Interval i4(1, 1, 500);

  MultiInterval mi2;
  mi2.addInter(i3);
  mi2.addInter(i4);

  AtomSet as2(mi2);

  Interval i5(30, 5, 36);
  Interval i6(42, 3, 57);

  MultiInterval mi3;
  mi3.addInter(i5);
  mi3.addInter(i6);  

  AtomSet as3(mi3);

  Set s;
  s.addAtomSet(as1);
  s.addAtomSet(as2);
  s.addAtomSet(as3);

  contNI1 res1 = s.minElem();

  contNI1 res2;
  res2.insert(res2.end(), 5);

  BOOST_CHECK(res1 == res2);   
}

void TestSetMin2(){
  Interval i1(30, 1, 35);
  Interval i2(5, 1, 10);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);

  AtomSet as1(mi1);

  Interval i3(20, 20, 80);
  Interval i4(1, 1, 500);

  MultiInterval mi2;
  mi2.addInter(i3);
  mi2.addInter(i4);

  AtomSet as2(mi2);

  Interval i5(30, 5, 36);
  Interval i6(42, 3, 57);

  MultiInterval mi3;
  mi3.addInter(i5);
  mi3.addInter(i6);  

  AtomSet as3(mi3);

  Set s;
  s.addAtomSet(as1);
  s.addAtomSet(as2);
  s.addAtomSet(as3);

  contNI1 res1 = s.minElem();

  contNI1 res2;
  res2.insert(res2.end(), 20);
  res2.insert(res2.end(), 1);

  BOOST_CHECK(res1 == res2);   
}

// -- LinearMaps -------------------------------------------------------------------//

void TestLMCreation1(){
  LMap res;

  BOOST_CHECK(res.empty());
}

void TestLMCompose1(){
  LMap lm1;

  lm1.addGO(5.0, 1.0);
  lm1.addGO(10.0, 2.0);
  lm1.addGO(3.0, 3.0);

  LMap lm2;

  lm2.addGO(2.0, 3.0);
  lm2.addGO(2.0, 3.0);
  lm2.addGO(2.0, 3.0);

  LMap res1 = lm1.compose(lm2);

  LMap res2;

  res2.addGO(10.0, 16.0);
  res2.addGO(20.0, 32.0);
  res2.addGO(6.0, 12.0);

  BOOST_CHECK(res1 == res2);
}

void TestLMCompose2(){
  LMap lm1;

  lm1.addGO(5.0, 1.0);
  lm1.addGO(10.0, 2.0);
  lm1.addGO(3.0, 3.0);

  LMap lm2;

  lm2.addGO(2.0, 3.0);
  lm2.addGO(2.0, 3.0);

  LMap res1 = lm1.compose(lm2);

  BOOST_CHECK(res1.empty());
}

void TestInvLMap1(){
  LMap lm1;

  lm1.addGO(5.0, 1.0);
  lm1.addGO(10.0, 2.0);
  lm1.addGO(3.0, 3.0);

  LMap res1 = lm1.invLMap();

  LMap res2; 

  float v1 = 1.0 / 5.0;
  float v2 = 1.0 / 10.0;
  float v3 = 1.0 / 3.0; 

  res2.addGO(v1, -v1);
  res2.addGO(v2, -v1);
  res2.addGO(v3, -1); 

  BOOST_CHECK(res1 == res2);
}

// -- Piece wise atomic linear maps ------------------------------------------//

void TestPWAtomCreation1(){
  Interval i1(1, 1, 10);
  Interval i2(1, 1, 10);
  Interval i3(1, 1, 10);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);
  mi1.addInter(i3);

  AtomSet as1(mi1);

  LMap lm1;

  lm1.addGO(1.0, 1.0);
  lm1.addGO(1.0, 1.0);

  PWAtomLMap pwatom1(as1, lm1);

  BOOST_CHECK(pwatom1.empty());
}

void TestPWAtomCreation2(){
  Interval i1(1, 1, 10);
  Interval i2(1, 1, 10);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);
 
  AtomSet as1(mi1);

  LMap lm1;
  lm1.addGO(0.5, 0.0);
  lm1.addGO(0.5, 0.0);

  PWAtomLMap pwatom1(as1, lm1);

  BOOST_CHECK(pwatom1.empty());
}

void TestPWAtomCreation3(){
  Interval i1(2, 2, 10);
  Interval i2(2, 2, 10);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);
 
  AtomSet as1(mi1);

  LMap lm1;
  lm1.addGO(0.5, 0.0);
  lm1.addGO(0.5, 0.0);

  PWAtomLMap pwatom1(as1, lm1);

  BOOST_CHECK(!pwatom1.empty());
}

void TestPWAtomImage1(){
  Interval i1(1, 1, 10);
  Interval i2(1, 1, 10);
  Interval i3(1, 1, 10);

  MultiInterval mi1; 
  mi1.addInter(i1);
  mi1.addInter(i2);
  mi1.addInter(i3);

  AtomSet as1(mi1);

  LMap lm1;

  lm1.addGO(2.0, 0.0);
  lm1.addGO(3.0, 0.0);
  lm1.addGO(1.0, 0.0);

  PWAtomLMap pwatom1(as1, lm1);

  AtomSet res1 = pwatom1.image(as1);

  Interval i4(2, 2, 20);
  Interval i5(3, 3, 30);
  Interval i6(1, 1, 10);

  MultiInterval mi2;
  mi2.addInter(i4);
  mi2.addInter(i5);
  mi2.addInter(i6);
 
  AtomSet res2(mi2);

  BOOST_CHECK(res1 == res2);
}

void TestPWAtomImage2(){
  Interval i1(1, 1, 10);
  Interval i2(1, 1, 10);
  Interval i3(1, 1, 10);

  MultiInterval mi1; 
  mi1.addInter(i1);
  mi1.addInter(i2);
  mi1.addInter(i3);

  AtomSet as1(mi1);

  LMap lm1;

  lm1.addGO(2.0, 0.0);
  lm1.addGO(3.0, 0.0);
  lm1.addGO(1.0, 0.0);

  PWAtomLMap pwatom1(as1, lm1);

  Interval i4(20, 5, 30);
  Interval i5(5, 1, 10);
  Interval i6(5, 1, 10);

  MultiInterval mi2;
  mi2.addInter(i4);
  mi2.addInter(i5);
  mi2.addInter(i6);

  AtomSet as2(mi2);

  AtomSet res1 = pwatom1.image(as2);
 
  AtomSet res2;

  BOOST_CHECK(res1 == res2);
}

void TestPWAtomImage3(){
  Interval i1(1, 1, 10);
  Interval i2(1, 1, 10);
  Interval i3(1, 1, 10);

  MultiInterval mi1; 
  mi1.addInter(i1);
  mi1.addInter(i2);
  mi1.addInter(i3);

  AtomSet as1(mi1);

  LMap lm1;

  lm1.addGO(2.0, 0.0);
  lm1.addGO(3.0, 0.0);
  lm1.addGO(1.0, 0.0);

  PWAtomLMap pwatom1(as1, lm1);

  Interval i4(1, 5, 30);
  Interval i5(5, 1, 10);
  Interval i6(5, 1, 10);

  MultiInterval mi2;
  mi2.addInter(i4);
  mi2.addInter(i5);
  mi2.addInter(i6);

  AtomSet as2(mi2);

  AtomSet res1 = pwatom1.image(as2);
 
  Interval i7(2, 10, 12);
  Interval i8(15, 3, 30);
  Interval i9(5, 1, 10);

  MultiInterval mi3;
  mi3.addInter(i7);
  mi3.addInter(i8);
  mi3.addInter(i9);  

  AtomSet res2(mi3);

  BOOST_CHECK(res1 == res2);
}

void TestPWAtomImage4(){
  Interval i1(1, 1, 10);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i1);

  AtomSet as1(mi1);

  LMap lm1;
  lm1.addGO(0.0, 1.0);
  lm1.addGO(0.0, 1.0);

  PWAtomLMap pwatom1(as1, lm1);

  AtomSet res1 = pwatom1.image(as1);  

  Interval i2(1, 0, 1);

  MultiInterval mi2;
  mi2.addInter(i2);
  mi2.addInter(i2);

  AtomSet res2(mi2);

  BOOST_CHECK(res1 == res2);
}

void TestPWAtomPre1(){
  Interval i1(1, 1, 10);
  Interval i2(1, 1, 10);
  Interval i3(1, 1, 10);

  MultiInterval mi1; 
  mi1.addInter(i1);
  mi1.addInter(i2);
  mi1.addInter(i3);

  AtomSet as1(mi1);

  LMap lm1;

  lm1.addGO(2.0, 0.0);
  lm1.addGO(3.0, 0.0);
  lm1.addGO(1.0, 0.0);

  PWAtomLMap pwatom1(as1, lm1);

  Interval i4(2, 2, 20);
  Interval i5(3, 3, 30);
  Interval i6(1, 1, 10);

  MultiInterval mi2;
  mi2.addInter(i4);
  mi2.addInter(i5);
  mi2.addInter(i6);

  AtomSet as2(mi2);

  AtomSet res1 = pwatom1.preImage(as2);

  Interval i7(1, 1, 10);
  Interval i8(1, 1, 10);
  Interval i9(1, 1, 10);

  MultiInterval mi3;
  mi3.addInter(i7);
  mi3.addInter(i8);
  mi3.addInter(i8);
 
  AtomSet res2(mi3);

  BOOST_CHECK(res1 == res2);
}

void TestPWAtomPre2(){
  Interval i1(1, 1, 10);
  Interval i2(1, 1, 10);
  Interval i3(1, 1, 10);

  MultiInterval mi1; 
  mi1.addInter(i1);
  mi1.addInter(i2);
  mi1.addInter(i3);

  AtomSet as1(mi1);

  LMap lm1;

  lm1.addGO(2.0, 0.0);
  lm1.addGO(3.0, 0.0);
  lm1.addGO(1.0, 0.0);

  PWAtomLMap pwatom1(as1, lm1);

  Interval i4(1, 1, 10);
  Interval i5(1, 1, 10);
  Interval i6(1, 1, 10);

  MultiInterval mi2;
  mi2.addInter(i4);
  mi2.addInter(i5);
  mi2.addInter(i6);

  AtomSet as2(mi2);

  AtomSet res1 = pwatom1.preImage(as2);

  Interval i7(1, 1, 5);
  Interval i8(1, 1, 3);
  Interval i9(1, 1, 10);

  MultiInterval mi3;
  mi3.addInter(i7);
  mi3.addInter(i8);
  mi3.addInter(i9);
 
  AtomSet res2(mi3);

  BOOST_CHECK(res1 == res2);
}

void TestPWAtomPre3(){
  Interval i1(1, 1, 10);
  Interval i2(1, 1, 10);
  Interval i3(1, 1, 10);

  MultiInterval mi1; 
  mi1.addInter(i1);
  mi1.addInter(i2);
  mi1.addInter(i3);

  AtomSet as1(mi1);

  LMap lm1;

  lm1.addGO(2.0, 0.0);
  lm1.addGO(3.0, 0.0);
  lm1.addGO(1.0, 0.0);

  PWAtomLMap pwatom1(as1, lm1);

  Interval i4(100, 1, 1000);
  Interval i5(1, 1, 10);
  Interval i6(1, 1, 10);

  MultiInterval mi2;
  mi2.addInter(i4);
  mi2.addInter(i5);
  mi2.addInter(i6);

  AtomSet as2(mi2);

  AtomSet res1 = pwatom1.preImage(as2);
 
  AtomSet res2;

  BOOST_CHECK(res1 == res2);
}

void TestPWAtomPre4(){
  Interval i1(1, 1, 10);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i1);

  AtomSet as1(mi1);

  LMap lm1;
  lm1.addGO(0.0, 3.0);
  lm1.addGO(0.0, 3.0);

  PWAtomLMap pwatom1(as1, lm1);  

  Interval i2(1, 1, 5);

  MultiInterval mi2;
  mi2.addInter(i2);
  mi2.addInter(i2);

  AtomSet as2(mi2);

  AtomSet res1 = pwatom1.preImage(as2);

  AtomSet res2 = as1;

  BOOST_CHECK(res1 == res2);
}

void TestPWAtomMin1(){
  BOOST_CHECK(true);
}

// -- Piece wise linear maps -------------------------------------------------//

void TestPWLMapCreation1(){
  Interval i1(1, 1, 10);
  Interval i2(1, 1, 10);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);

  AtomSet as1(mi1);

  Interval i3(20, 3, 30);
  Interval i4(20, 3, 30);

  MultiInterval mi2;
  mi2.addInter(i3);
  mi2.addInter(i4);

  AtomSet as2(mi2);

  Set s1;
  s1.addAtomSet(as1);
  s1.addAtomSet(as2);

  LMap lm1;
  lm1.addGO(1.0, 0.0);
  lm1.addGO(1.0, 0.0);

  Interval i5(15, 3, 18);
  Interval i6(15, 3, 18);

  MultiInterval mi3;
  mi3.addInter(i5);
  mi3.addInter(i6);

  AtomSet as3(mi3);

  Set s2;
  s2.addAtomSet(as3);

  LMap lm2;
  lm2.addGO(1.0, 0.0);

  PWLMap pw1;
  pw1.addSetLM(s1, lm1); 
  pw1.addSetLM(s2, lm2);
  
  BOOST_CHECK(pw1.empty());
}

void TestPWLMapImage1(){
  Interval i1(1, 1, 5);
  Interval i2(1, 1, 5);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);

  AtomSet as1(mi1);

  Interval i3(10, 1, 15);
  Interval i4(10, 1, 15);

  MultiInterval mi2;
  mi2.addInter(i3);
  mi2.addInter(i4);

  AtomSet as2(mi2);

  Set s1;
  s1.addAtomSet(as1);
  s1.addAtomSet(as2);

  Interval i5(20, 3, 30);
  Interval i6(20, 3, 30);

  MultiInterval mi3;
  mi3.addInter(i5);
  mi3.addInter(i6);

  AtomSet as3(mi3);

  Interval i7(45, 5, 50);
  Interval i8(45, 5, 50);

  MultiInterval mi4;
  mi4.addInter(i7);
  mi4.addInter(i8);

  AtomSet as4(mi4);

  Set s2;
  s2.addAtomSet(as3);
  s2.addAtomSet(as4);

  LMap lm1;
  lm1.addGO(1.0, 0.0);
  lm1.addGO(1.0, 0.0);

  LMap lm2;
  lm2.addGO(2.0, 0.0);
  lm2.addGO(2.0, 0.0);

  PWLMap pw1;
  pw1.addSetLM(s1, lm1);
  pw1.addSetLM(s2, lm2);

  Set aux = s1.cup(s2);
  Set res1 = pw1.image(aux);

  Interval i9(40, 6, 60);
  Interval i10(40, 6, 60);

  MultiInterval mi5;
  mi5.addInter(i9);
  mi5.addInter(i10);

  AtomSet as5(mi5);

  Interval i11(90, 10, 100);
  Interval i12(90, 10, 100);

  MultiInterval mi6;
  mi6.addInter(i11);
  mi6.addInter(i12);

  AtomSet as6(mi6);

  Set s3;
  s3.addAtomSet(as5);
  s3.addAtomSet(as6);

  Set res2 = s1.cup(s3);

  BOOST_CHECK(res1 == res2);
}

void TestPWLMapImage2(){
  Interval i1(1, 1, 5);
  Interval i2(1, 1, 5);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);

  AtomSet as1(mi1);

  Interval i3(10, 1, 15);
  Interval i4(10, 1, 15);

  MultiInterval mi2;
  mi2.addInter(i3);
  mi2.addInter(i4);

  AtomSet as2(mi2);

  Set s1;
  s1.addAtomSet(as1);
  s1.addAtomSet(as2);

  Interval i5(20, 3, 30);
  Interval i6(20, 3, 30);

  MultiInterval mi3;
  mi3.addInter(i5);
  mi3.addInter(i6);

  AtomSet as3(mi3);

  Interval i7(45, 5, 50);
  Interval i8(45, 5, 50);

  MultiInterval mi4;
  mi4.addInter(i7);
  mi4.addInter(i8);

  AtomSet as4(mi4);

  Set s2;
  s2.addAtomSet(as3);
  s2.addAtomSet(as4);

  LMap lm1;
  lm1.addGO(1.0, 0.0);
  lm1.addGO(1.0, 0.0);

  LMap lm2;
  lm2.addGO(2.0, 0.0);
  lm2.addGO(2.0, 0.0);

  PWLMap pw1;
  pw1.addSetLM(s1, lm1);
  pw1.addSetLM(s2, lm2);

  Set res1 = pw1.image(s1);

  Set res2 = s1;

  BOOST_CHECK(res1 == res2);
}

void TestPWLMapPre1(){
  Interval i1(1, 1, 10);
  
  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i1);

  AtomSet as1(mi1);

  Interval i2(20, 5, 30);
  
  MultiInterval mi2;
  mi2.addInter(i2);
  mi2.addInter(i2);

  AtomSet as2(mi2);

  Set s1;
  s1.addAtomSet(as1);
  s1.addAtomSet(as2);

  Interval i3(11, 1, 14);

  MultiInterval mi3;
  mi3.addInter(i3);
  mi3.addInter(i3);

  AtomSet as3(mi3);

  Interval i4(1, 1, 10);
  Interval i5(50, 5, 70);

  MultiInterval mi4;
  mi4.addInter(i4);
  mi4.addInter(i5);

  AtomSet as4(mi4);

  Set s2;
  s2.addAtomSet(as3);
  s2.addAtomSet(as4);

  LMap lm1; 
  lm1.addGO(0.0, 3.0);
  lm1.addGO(0.0, 4.0);

  LMap lm2;
  lm2.addGO(2.0, 0.0);
  lm2.addGO(2.0, 1.0);

  PWLMap pw1;
  pw1.addSetLM(s1, lm1);
  pw1.addSetLM(s2, lm2);

  Interval i6(0, 1, 25);

  MultiInterval mi5;
  mi5.addInter(i6);
  mi5.addInter(i6);

  AtomSet as5(mi5);

  Set s3;
  s3.addAtomSet(as5);

  Set res1 = pw1.preImage(s3);

  Interval i7(11, 1, 12);

  MultiInterval mi6;
  mi6.addInter(i7);
  mi6.addInter(i7);

  AtomSet as6(mi6);

  Set res2;
  res2.addAtomSet(as1);
  res2.addAtomSet(as2);
  res2.addAtomSet(as6);

  BOOST_CHECK(res1 == res2);
}

void TestPWLMapComp1(){
  Interval i1(1, 1, 10);
  Interval i2(1, 1, 5);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);

  AtomSet as1(mi1);

  Interval i3(20, 2, 30);

  MultiInterval mi2;
  mi2.addInter(i3);
  mi2.addInter(i3);

  AtomSet as2(mi2);

  Set s1;
  s1.addAtomSet(as1);
  s1.addAtomSet(as2);
  
  Interval i4(15, 3, 18);
  Interval i5(12, 3, 20);

  MultiInterval mi3;
  mi3.addInter(i4);
  mi3.addInter(i5);

  AtomSet as3(mi3);
 
  Set s2;
  s2.addAtomSet(as3);

  LMap lm1;
  lm1.addGO(2.0, 1.0);
  lm1.addGO(3.0, 0.0);

  LMap lm2;
  lm2.addGO(0.0, 0.0);
  lm2.addGO(0.0, 0.0);

  PWLMap pw1;
  pw1.addSetLM(s1, lm1);
  pw1.addSetLM(s2, lm2);

  Interval i6(1, 1, 30);

  MultiInterval mi4;
  mi4.addInter(i6);
  mi4.addInter(i6);

  AtomSet as4(mi4);

  Set s4;
  s4.addAtomSet(as4);

  LMap lm3;
  lm3.addGO(1.0, 1.0);
  lm3.addGO(1.0, 2.0);

  PWLMap pw2;
  pw2.addSetLM(s4, lm3);

  PWLMap res1 = pw1.compPW(pw2);

  Interval i7(1, 1, 9);
  Interval i8(1, 1, 3);

  MultiInterval mi5;
  mi5.addInter(i7);
  mi5.addInter(i8);

  AtomSet as5(mi5);

  Interval i9(19, 2, 29);
  Interval i10(18, 2, 28);

  MultiInterval mi6;
  mi6.addInter(i9);
  mi6.addInter(i10);

  AtomSet as6(mi6);

  Set s5;
  s5.addAtomSet(as5);
  s5.addAtomSet(as6);

  Interval i11(14, 3, 17);
  Interval i12(10, 3, 16);

  MultiInterval mi7;
  mi7.addInter(i11);
  mi7.addInter(i12);

  AtomSet as7(mi7);

  Set s6;
  s6.addAtomSet(as7);

  LMap lm4;
  lm4.addGO(2.0, 3.0);
  lm4.addGO(3.0, 6.0);

  PWLMap res2;
  res2.addSetLM(s5, lm4);
  res2.addSetLM(s6, lm2);
 
  BOOST_CHECK(res1 == res2);
}

void TestPWLMapComp2(){
  Interval i1(3, 1, 100);
  Interval i2(50, 5, 150);
  Interval i3(100, 10, 1000);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);
  mi1.addInter(i3);
 
  AtomSet as1(mi1);

  Interval i4(35, 7, 99);
  
  MultiInterval mi2;
  mi2.addInter(i1);
  mi2.addInter(i1);
  mi2.addInter(i4);

  AtomSet as2(mi2);

  Set s1;
  s1.addAtomSet(as1);
  s1.addAtomSet(as2);

  Interval i5(200, 20, 400);
  Interval i6(5, 5, 100);
  Interval i7(10, 9, 90);

  MultiInterval mi3;
  mi3.addInter(i5);
  mi3.addInter(i6);
  mi3.addInter(i7);

  AtomSet as3(mi3);

  Interval i8(150, 50, 300);
  Interval i9(200, 3, 500);
  Interval i10(4, 4, 80);

  MultiInterval mi4;
  mi4.addInter(i8);
  mi4.addInter(i9);
  mi4.addInter(i10);

  AtomSet as4(mi4);

  Set s2;
  s2.addAtomSet(as3);
  s2.addAtomSet(as4);

  LMap lm1;
  lm1.addGO(1, -3);
  lm1.addGO(0, 0);
  lm1.addGO(1, 1);

  LMap lm2;
  lm2.addGO(3, 2);
  lm2.addGO(1, -2);
  lm2.addGO(1, 0);

  PWLMap pw1;
  pw1.addSetLM(s1, lm1);
  pw1.addSetLM(s2, lm2);

  PWLMap res1 = pw1.compPW(pw1);

  PWLMap res2;

  BOOST_CHECK(res1 == res2);
}

void TestPWLMapMinInvComp1(){
  Interval i1(1, 1, 10);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i1);
  mi1.addInter(i1);

  AtomSet as1(mi1);

  Interval i2(15, 3, 30);
  Interval i3(1, 1, 5);

  MultiInterval mi2;
  mi2.addInter(i1);
  mi2.addInter(i2);
  mi2.addInter(i3);

  AtomSet as2(mi2);

  Interval i4(11, 1, 14);

  MultiInterval mi3;
  mi3.addInter(i4);
  mi3.addInter(i3);
  mi3.addInter(i1);

  AtomSet as3(mi3);

  Set s1;
  s1.addAtomSet(as1);
  s1.addAtomSet(as2);
  s1.addAtomSet(as3);

  LMap lm1;
  lm1.addGO(1.0, 1.0);
  lm1.addGO(1.0, 0.0);
  lm1.addGO(1.0, 0.0);

  PWLMap pw1;
  pw1.addSetLM(s1, lm1);

  PWLMap res1 = pw1.minInvCompact();

  Interval i5(2, 1, 11);

  MultiInterval mi4;
  mi4.addInter(i5);
  mi4.addInter(i1);
  mi4.addInter(i1);

  AtomSet as4(mi4);

  MultiInterval mi5;
  mi5.addInter(i5); 
  mi5.addInter(i2);
  mi5.addInter(i3);

  AtomSet as5(mi5);

  Interval i6(12, 1, 15);

  MultiInterval mi6;
  mi6.addInter(i6);
  mi6.addInter(i3);
  mi6.addInter(i1);

  AtomSet as6(mi6);

  Set s2;
  s2.addAtomSet(as4);
  s2.addAtomSet(as5);
  s2.addAtomSet(as6);

  LMap lm2;
  lm2.addGO(1.0, -1.0);
  lm2.addGO(1.0, 0.0);
  lm2.addGO(1.0, 0.0);

  PWLMap res2;
  res2.addSetLM(s2, lm2);

  BOOST_CHECK(res1 == res2);
}

void TestPWLMapCombine1(){
  Interval i1(1, 1, 10);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i1);
  mi1.addInter(i1);

  AtomSet as1(mi1);
 
  Interval i2(20, 3, 30);

  MultiInterval mi2;
  mi2.addInter(i1);
  mi2.addInter(i2);
  mi2.addInter(i2);

  AtomSet as2(mi2);

  Set s1;
  s1.addAtomSet(as1);
  s1.addAtomSet(as2);

  Interval i3(35, 5, 50);

  MultiInterval mi3;
  mi3.addInter(i1);
  mi3.addInter(i2);
  mi3.addInter(i3);

  AtomSet as3(mi3);

  MultiInterval mi4;
  mi4.addInter(i3);
  mi4.addInter(i3);
  mi4.addInter(i2);

  AtomSet as4(mi4);

  Set s2;
  s2.addAtomSet(as3);
  s2.addAtomSet(as4);

  LMap lm1;
  lm1.addGO(1, 0);
  lm1.addGO(1, 0);
  lm1.addGO(1, 0);

  LMap lm2;
  lm2.addGO(0, 3);
  lm2.addGO(0, 3);
  lm2.addGO(1, 1);

  PWLMap pw1;
  pw1.addSetLM(s1, lm1);
  pw1.addSetLM(s2, lm2);

  Interval i4(1, 1, 20);

  MultiInterval mi5;
  mi5.addInter(i4);
  mi5.addInter(i4);
  mi5.addInter(i4);

  AtomSet as5(mi5);

  Set s3;
  s3.addAtomSet(as5);

  LMap lm3;
  lm3.addGO(1, 1);
  lm3.addGO(1, 0);
  lm3.addGO(1, 0);
  
  PWLMap pw2;
  pw2.addSetLM(s3, lm3);

  PWLMap res1 = pw1.combine(pw2);

  Set s4 = pw1.wholeDom();

  Set s5 = s3.diff(s4); 

  PWLMap res2;
  res2.addSetLM(s1, lm1);
  res2.addSetLM(s2, lm2);
  res2.addSetLM(s5, lm3);

  BOOST_CHECK(res1 == res2);
}

void TestMinAtomPW1(){
  Interval i1(2, 2, 20);
  Interval i2(1, 1, 10);
  Interval i3(3, 3, 50);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);
  mi1.addInter(i3);

  AtomSet as1(mi1);

  LMap lm1;
  lm1.addGO(1.0, 60.0);
  lm1.addGO(2.0, 2.0);
  lm1.addGO(0.0, 35.0);

  LMap lm2; 
  lm2.addGO(1.0, 60.0);
  lm2.addGO(2.0, 2.0);
  lm2.addGO(1.0, 10.0);

  PWLMap res1 = minAtomPW(as1, lm1, lm2);

  Interval i4(3, 3, 24);

  MultiInterval mi2;
  mi2.addInter(i1);
  mi2.addInter(i2);
  mi2.addInter(i4);

  AtomSet as2(mi2);

  Set s1;
  s1.addAtomSet(as2);

  Interval i5(27, 3, 50);

  MultiInterval mi3;
  mi3.addInter(i1);
  mi3.addInter(i2);
  mi3.addInter(i5);

  AtomSet as3(mi3);

  Set s2;
  s2.addAtomSet(as3);

  PWLMap res2;
  res2.addSetLM(s1, lm2);
  res2.addSetLM(s2, lm1);

  BOOST_CHECK(res1 == res2);
}

void TestMinAtomPW2(){
  Interval i1(2, 2, 20);
  Interval i2(1, 1, 10);
  Interval i3(3, 3, 50);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);
  mi1.addInter(i3);

  AtomSet as1(mi1);

  LMap lm1;
  lm1.addGO(1.0, 60.0);
  lm1.addGO(2.0, 5.0);
  lm1.addGO(1.0, 60.0);

  LMap lm2; 
  lm2.addGO(1.0, 60.0);
  lm2.addGO(2.0, 3.0);
  lm2.addGO(1.0, 10.0);

  PWLMap res1 = minAtomPW(as1, lm1, lm2);

  Set s1;
  s1.addAtomSet(as1);

  PWLMap res2;
  res2.addSetLM(s1, lm2);

  BOOST_CHECK(res1 == res2);
}

void TestMinAtomPW3(){
  Interval i1(2, 2, 20);
  Interval i2(1, 1, 10);
  Interval i3(3, 3, 50);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);
  mi1.addInter(i3);

  AtomSet as1(mi1);

  LMap lm1;
  lm1.addGO(1.0, 60.0);
  lm1.addGO(2.0, 2.0);
  lm1.addGO(0.0, 35.0);

  PWLMap res1 = minAtomPW(as1, lm1, lm1);

  BOOST_CHECK(true);
}

void TestMinPW1(){
  Interval i1(1, 1, 5);

  MultiInterval mi1;
  mi1.addInter(i1);

  AtomSet as1(mi1);

  Interval i2(10, 1, 15);

  MultiInterval mi2;
  mi2.addInter(i2);

  AtomSet as2(mi2);

  Interval i3(20, 2, 30);
  
  MultiInterval mi3;
  mi3.addInter(i3);

  AtomSet as3(mi3); 

  Set s1;
  s1.addAtomSet(as1);
  s1.addAtomSet(as2);
  s1.addAtomSet(as3);

  LMap lm1;
  lm1.addGO(0, 12);

  LMap lm2;
  lm2.addGO(2, -12);

  PWLMap res1 = minPW(s1, lm1, lm2);
 
  Interval i4(10, 1, 12);

  MultiInterval mi4;
  mi4.addInter(i4);

  AtomSet as4(mi4);

  Set s2;
  s2.addAtomSet(as1);
  s2.addAtomSet(as4);

  Interval i5(13, 1, 15);

  MultiInterval mi5;
  mi5.addInter(i5);

  AtomSet as5(mi5);

  Set s3;
  s3.addAtomSet(as3);
  s3.addAtomSet(as5);

  PWLMap res2;
  res2.addSetLM(s2, lm2);
  res2.addSetLM(s3, lm1);

  BOOST_CHECK(res1 == res2);
}

void TestMinPW2(){
  Interval i1(2, 1, 5);

  MultiInterval mi1;
  mi1.addInter(i1);

  AtomSet as1(mi1);

  Interval i2(10, 1, 15);

  MultiInterval mi2;
  mi2.addInter(i2);

  AtomSet as2(mi2);

  Interval i3(20, 2, 30);
  
  MultiInterval mi3;
  mi3.addInter(i3);

  AtomSet as3(mi3); 

  Set s1;
  s1.addAtomSet(as1);
  s1.addAtomSet(as2);
  s1.addAtomSet(as3);

  LMap lm1;
  lm1.addGO(0, 1);

  LMap lm2;
  lm2.addGO(1, 0);

  PWLMap res1 = minPW(s1, lm1, lm2);

  PWLMap res2;
  res2.addSetLM(s1, lm1);

  BOOST_CHECK(res1 == res2);
}

void TestMinMap1(){
  Interval i1(1, 1, 10);
 
  MultiInterval mi1;
  mi1.addInter(i1);

  AtomSet as1(mi1);

  Interval i2(15, 3, 30);

  MultiInterval mi2;
  mi2.addInter(i2);

  AtomSet as2(mi2);

  Set s1;
  s1.addAtomSet(as1);
  s1.addAtomSet(as2);

  Interval i3(12, 3, 12);

  MultiInterval mi3;
  mi3.addInter(i3);

  AtomSet as3(mi3);

  Interval i4(50, 5, 100);

  MultiInterval mi4;
  mi4.addInter(i4);

  AtomSet as4(mi4);

  Set s2;
  s2.addAtomSet(as3);
  s2.addAtomSet(as4);

  LMap lm1;
  lm1.addGO(1, 0);

  LMap lm2;
  lm2.addGO(1, 1);

  PWLMap pw1;
  pw1.addSetLM(s1, lm1);
  pw1.addSetLM(s2, lm2); 

  Interval i5(1, 2, 20);

  MultiInterval mi5;
  mi5.addInter(i5);

  AtomSet as5(mi5);

  Interval i6(30, 5, 60);

  MultiInterval mi6;
  mi6.addInter(i6);

  AtomSet as6(mi6);

  Set s3;
  s3.addAtomSet(as5);
  s3.addAtomSet(as6);

  Interval i7(75, 5, 90);
  
  MultiInterval mi7;
  mi7.addInter(i7);

  AtomSet as7(mi7);

  Interval i8(95, 1, 100);

  MultiInterval mi8;
  mi8.addInter(i8);

  AtomSet as8(mi8);

  Set s4;
  s4.addAtomSet(as7);
  s4.addAtomSet(as8);

  LMap lm3;
  lm3.addGO(0, 100);

  LMap lm4;
  lm4.addGO(1, 0);

  PWLMap pw2;
  pw2.addSetLM(s3, lm3);
  pw2.addSetLM(s4, lm4);

  PWLMap res1 = minMap(pw1, pw2);

  Interval i9(1, 2, 9);

  MultiInterval mi9;
  mi9.addInter(i9);

  AtomSet as9(mi9);

  Interval i10(15, 6, 18);

  MultiInterval mi10;
  mi10.addInter(i10);

  AtomSet as10(mi10);

  Interval i11(30, 15, 30);

  MultiInterval mi11;
  mi11.addInter(i11);

  AtomSet as11(mi11);


  Set s5;
  s5.addAtomSet(as9);
  s5.addAtomSet(as10);
  s5.addAtomSet(as11);

  Interval i12(75, 5, 90);

  MultiInterval mi12;
  mi12.addInter(i12);

  AtomSet as12(mi12);

  Interval i13(95, 5, 100);

  MultiInterval mi13;
  mi13.addInter(i13);

  AtomSet as13(mi13);
  
  Set s6;
  s6.addAtomSet(as12);
  s6.addAtomSet(as13);

  Interval i14(50, 5, 60);

  MultiInterval mi14;
  mi14.addInter(i14);

  AtomSet as14(mi14);

  Set s7;
  s7.addAtomSet(as14);

  PWLMap res2;
  res2.addSetLM(s6, lm4);
  res2.addSetLM(s7, lm2);
  res2.addSetLM(s5, lm1);

  BOOST_CHECK(res1 == res2);
}

void TestReduce1(){
  Interval i1(4, 1, 15);
 
  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i1);
  mi1.addInter(i1);

  AtomSet as1(mi1);
 
  Interval i2(20, 2, 25);

  MultiInterval mi2;
  mi2.addInter(i1);
  mi2.addInter(i2);
  mi2.addInter(i1);

  AtomSet as2(mi2);

  Set s1;
  s1.addAtomSet(as1);
  s1.addAtomSet(as2);  

  Interval i3(15, 5, 50);

  MultiInterval mi3;
  mi3.addInter(i1);
  mi3.addInter(i3);
  mi3.addInter(i2);

  AtomSet as3(mi3);

  Interval i4(40, 5, 45);

  MultiInterval mi4;
  mi4.addInter(i2);
  mi4.addInter(i4);
  mi4.addInter(i4);

  AtomSet as4(mi4);

  Set s2; 
  s2.addAtomSet(as3);
  s2.addAtomSet(as4);

  LMap lm1;
  lm1.addGO(1, 0);
  lm1.addGO(1, -3);
  lm1.addGO(3, 0);

  LMap lm2;
  lm2.addGO(2, 0);
  lm2.addGO(1, 0);
  lm2.addGO(4, 4);

  PWLMap pw1;
  pw1.addSetLM(s1, lm1);
  pw1.addSetLM(s2, lm2);

  PWLMap res1 = reduceMapN(pw1, 2);

  Interval i5(4, 3, 15);

  MultiInterval mi5;
  mi5.addInter(i1);
  mi5.addInter(i5);
  mi5.addInter(i1);

  AtomSet as5(mi5);

  Set s3;
  s3.addAtomSet(as5);

  LMap lm3;
  lm3.addGO(1, 0);
  lm3.addGO(0, 1);
  lm3.addGO(3, 0);

  Interval i6(5, 3, 15);

  MultiInterval mi6;
  mi6.addInter(i1);
  mi6.addInter(i6);
  mi6.addInter(i1);

  AtomSet as6(mi6);

  Set s4;
  s4.addAtomSet(as6);

  LMap lm4;
  lm4.addGO(1, 0);
  lm4.addGO(0, 2);
  lm4.addGO(3, 0);

  Interval i7(6, 3, 15);

  MultiInterval mi7;
  mi7.addInter(i1);
  mi7.addInter(i7);
  mi7.addInter(i1);

  AtomSet as7(mi7);

  Set s5;
  s5.addAtomSet(as7);
 
  LMap lm5;
  lm5.addGO(1, 0); 
  lm5.addGO(0, 3); 
  lm5.addGO(3, 0); 

  Set s6;
  s6.addAtomSet(as2);

  PWLMap res2;
  res2.addSetLM(s6, lm1);
  res2.addSetLM(s2, lm2);
  res2.addSetLM(s3, lm3);
  res2.addSetLM(s4, lm4);
  res2.addSetLM(s5, lm5);

  BOOST_CHECK(res1 == res2); 
}

void TestMapInf1(){
  Interval i1(3, 1, 100);
  Interval i2(50, 5, 150);
  Interval i3(100, 10, 1000);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);
  mi1.addInter(i3);
 
  AtomSet as1(mi1);

  Interval i4(35, 7, 99);
  
  MultiInterval mi2;
  mi2.addInter(i1);
  mi2.addInter(i1);
  mi2.addInter(i4);

  AtomSet as2(mi2);

  Set s1;
  s1.addAtomSet(as1);
  s1.addAtomSet(as2);

  Interval i5(200, 20, 400);
  Interval i6(5, 5, 100);
  Interval i7(10, 9, 90);

  MultiInterval mi3;
  mi3.addInter(i5);
  mi3.addInter(i6);
  mi3.addInter(i7);

  AtomSet as3(mi3);

  Interval i8(150, 50, 300);
  Interval i9(200, 3, 500);
  Interval i10(4, 4, 80);

  MultiInterval mi4;
  mi4.addInter(i8);
  mi4.addInter(i9);
  mi4.addInter(i10);

  AtomSet as4(mi4);

  Set s2;
  s2.addAtomSet(as3);
  s2.addAtomSet(as4);

  LMap lm1;
  lm1.addGO(1, -3);
  lm1.addGO(0, 0);
  lm1.addGO(1, 1);

  LMap lm2;
  lm2.addGO(3, 2);
  lm2.addGO(1, -2);
  lm2.addGO(1, 0);

  PWLMap pw1;
  pw1.addSetLM(s1, lm1);
  pw1.addSetLM(s2, lm2);

  PWLMap res1 = mapInf(pw1);

  PWLMap res2;

  BOOST_CHECK(res1 == res2);
}

void TestMapInf2(){
  Interval i1(1, 1, 100);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i1);
  mi1.addInter(i1);

  AtomSet as1(mi1);

  Set s1;
  s1.addAtomSet(as1);

  Interval i2(200, 4, 500);

  MultiInterval mi2;
  mi2.addInter(i2);
  mi2.addInter(i2);
  mi2.addInter(i2);

  AtomSet as2(mi2);

  Interval i3(200, 1, 2000);

  MultiInterval mi3;
  mi3.addInter(i1);
  mi3.addInter(i1);
  mi3.addInter(i3);

  AtomSet as3(mi3);

  Set s2;
  s2.addAtomSet(as2);
  s2.addAtomSet(as3);

  LMap lm1;
  lm1.addGO(1, 0);
  lm1.addGO(1, 0);
  lm1.addGO(1, 0);

  LMap lm2;
  lm2.addGO(0, 1);
  lm2.addGO(0, 1);
  lm2.addGO(0, 1);

  PWLMap pw1;
  pw1.addSetLM(s1, lm1);
  pw1.addSetLM(s2, lm2);

  PWLMap res1 = mapInf(pw1);

  PWLMap res2 = pw1;

  BOOST_CHECK(res1 == res2);
}

void TestMapInf3(){
  Interval i1(1, 1, 100);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i1);
  mi1.addInter(i1);

  AtomSet as1(mi1);

  Set s1;
  s1.addAtomSet(as1);

  Interval i2(200, 4, 500);

  MultiInterval mi2;
  mi2.addInter(i2);
  mi2.addInter(i2);
  mi2.addInter(i2);

  AtomSet as2(mi2);

  Interval i3(200, 1, 2000);

  MultiInterval mi3;
  mi3.addInter(i1);
  mi3.addInter(i1);
  mi3.addInter(i3);

  AtomSet as3(mi3);

  Set s2;
  s2.addAtomSet(as2);
  s2.addAtomSet(as3);

  LMap lm1;
  lm1.addGO(1, 0);
  lm1.addGO(1, 0);
  lm1.addGO(1, 0);

  LMap lm2;
  lm2.addGO(0, 150);
  lm2.addGO(0, 150);
  lm2.addGO(0, 150);

  PWLMap pw1;
  pw1.addSetLM(s1, lm1);
  pw1.addSetLM(s2, lm2);

  PWLMap res1 = mapInf(pw1);

  PWLMap res2;
  res2.addSetLM(s1, lm1);

  BOOST_CHECK(res1 == res2);
}

void TestMinAdjComp1(){
  Interval i1(50, 1, 100);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i1);
  mi1.addInter(i1);

  AtomSet as1(mi1);

  Interval i2(150, 5, 200);

  MultiInterval mi2;
  mi2.addInter(i2);
  mi2.addInter(i2);
  mi2.addInter(i1);

  AtomSet as2(mi2);  

  Set s1;
  s1.addAtomSet(as1);
  s1.addAtomSet(as2);

  LMap lm1;
  lm1.addGO(1, 0);
  lm1.addGO(1, 0);
  lm1.addGO(1, 0);

  PWLMap pw1;
  pw1.addSetLM(s1, lm1);

  Interval i3(75, 5, 150);
  Interval i4(80, 5, 150);
  Interval i5(85, 5, 150);

  MultiInterval mi3;
  mi3.addInter(i3);
  mi3.addInter(i4);
  mi3.addInter(i5);

  AtomSet as3(mi3);

  Set s2;
  s2.addAtomSet(as3);

  Interval i6(200, 1, 200);

  MultiInterval mi4;
  mi4.addInter(i6);
  mi4.addInter(i6);
  mi4.addInter(i6);

  AtomSet as4(mi4);

  Set s3;
  s3.addAtomSet(as4);

  LMap lm2;
  lm2.addGO(0, 3);
  lm2.addGO(0, 3);
  lm2.addGO(0, 3);

  LMap lm3;
  lm3.addGO(1, -10);
  lm3.addGO(1, -10);
  lm3.addGO(0, -10);

  PWLMap pw2;
  pw2.addSetLM(s2, lm2);
  pw2.addSetLM(s3, lm3);

  PWLMap res1 = minAdjCompMap(pw1, pw2); 

  // pw1.inv = pw1
  PWLMap res2 = pw2.compPW(pw1);

  BOOST_CHECK(res1 == res2);
}

void TestMinAdjComp2(){
  Interval i1(50, 1, 100);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i1);
  mi1.addInter(i1);

  AtomSet as1(mi1);

  Interval i2(150, 5, 200);

  MultiInterval mi2;
  mi2.addInter(i2);
  mi2.addInter(i2);
  mi2.addInter(i1);

  AtomSet as2(mi2);  

  Set s1;
  s1.addAtomSet(as1);
  s1.addAtomSet(as2);

  LMap lm1;
  lm1.addGO(1, 0);
  lm1.addGO(0, 1);
  lm1.addGO(1, 0);

  PWLMap pw1;
  pw1.addSetLM(s1, lm1);

  Interval i3(75, 5, 150);
  Interval i4(80, 5, 150);
  Interval i5(85, 5, 150);

  MultiInterval mi3;
  mi3.addInter(i3);
  mi3.addInter(i4);
  mi3.addInter(i5);

  AtomSet as3(mi3);

  Set s2;
  s2.addAtomSet(as3);

  Interval i6(200, 1, 200);

  MultiInterval mi4;
  mi4.addInter(i6);
  mi4.addInter(i6);
  mi4.addInter(i6);

  AtomSet as4(mi4);

  Set s3;
  s3.addAtomSet(as4);

  LMap lm2;
  lm2.addGO(0, 3);
  lm2.addGO(0, 3);
  lm2.addGO(0, 3);

  LMap lm3;
  lm3.addGO(1, -10);
  lm3.addGO(1, -10);
  lm3.addGO(0, -10);

  PWLMap pw2;
  pw2.addSetLM(s2, lm2);
  pw2.addSetLM(s3, lm3);

  PWLMap res1 = minAdjCompMap(pw1, pw2); 

  PWLMap res2;

  BOOST_CHECK(res1 == res2);
}

void TestMinAdjComp3(){
  Interval i1(85, 1, 100);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i1);
  mi1.addInter(i1);

  AtomSet as1(mi1);

  Interval i2(150, 5, 200);

  MultiInterval mi2;
  mi2.addInter(i2);
  mi2.addInter(i2);
  mi2.addInter(i1);

  AtomSet as2(mi2);  

  Set s1;
  s1.addAtomSet(as1);
  s1.addAtomSet(as2);

  LMap lm1;
  lm1.addGO(1, 0);
  lm1.addGO(0, 1);
  lm1.addGO(1, 0);

  PWLMap pw1;
  pw1.addSetLM(s1, lm1);

  Interval i3(75, 5, 150);
  Interval i4(80, 5, 150);
  Interval i5(85, 5, 150);

  MultiInterval mi3;
  mi3.addInter(i3);
  mi3.addInter(i4);
  mi3.addInter(i5);

  AtomSet as3(mi3);

  Set s2;
  s2.addAtomSet(as3);

  Interval i6(200, 1, 200);

  MultiInterval mi4;
  mi4.addInter(i6);
  mi4.addInter(i6);
  mi4.addInter(i6);

  AtomSet as4(mi4);

  Set s3;
  s3.addAtomSet(as4);

  LMap lm2;
  lm2.addGO(0, 3);
  lm2.addGO(0, 3);
  lm2.addGO(0, 3);

  LMap lm3;
  lm3.addGO(1, -10);
  lm3.addGO(1, -10);
  lm3.addGO(0, -10);

  PWLMap pw2;
  pw2.addSetLM(s2, lm2);
  pw2.addSetLM(s3, lm3);

  PWLMap res1 = minAdjCompMap(pw1, pw2); 

  PWLMap res2;

  Interval i7(85, 5, 100);
  Interval i8(1, 1, 1);

  MultiInterval mi5;
  mi5.addInter(i7);
  mi5.addInter(i8); 
  mi5.addInter(i7);

  AtomSet as5(mi5);

  Interval i9(150, 5, 150);

  MultiInterval mi6;
  mi6.addInter(i9);
  mi6.addInter(i8);
  mi6.addInter(i7);

  AtomSet as6(mi6);

  Set s4;
  s4.addAtomSet(as5);
  s4.addAtomSet(as6);

  res2.addSetLM(s4, lm2);

  BOOST_CHECK(res1 == res2);
}

void TestMinAdjComp4(){
  Interval i1(85, 1, 100);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i1);
  mi1.addInter(i1);

  AtomSet as1(mi1);

  Interval i2(150, 5, 200);

  MultiInterval mi2;
  mi2.addInter(i2);
  mi2.addInter(i2);
  mi2.addInter(i1);

  AtomSet as2(mi2);  

  Set s1;
  s1.addAtomSet(as1);
  s1.addAtomSet(as2);

  LMap lm1;
  lm1.addGO(0, 1);
  lm1.addGO(0, 1);
  lm1.addGO(0, 1);

  PWLMap pw1;
  pw1.addSetLM(s1, lm1);

  Interval i3(75, 5, 150);
  Interval i4(80, 5, 150);
  Interval i5(85, 5, 150);

  MultiInterval mi3;
  mi3.addInter(i3);
  mi3.addInter(i4);
  mi3.addInter(i5);

  AtomSet as3(mi3);

  Set s2;
  s2.addAtomSet(as3);

  Interval i6(200, 1, 200);

  MultiInterval mi4;
  mi4.addInter(i6);
  mi4.addInter(i6);
  mi4.addInter(i6);

  AtomSet as4(mi4);

  Set s3;
  s3.addAtomSet(as4);

  LMap lm2;
  lm2.addGO(1, 3);
  lm2.addGO(0, 3);
  lm2.addGO(2, 3);

  LMap lm3;
  lm3.addGO(1, -10);
  lm3.addGO(1, -10);
  lm3.addGO(0, -10);

  PWLMap pw2;
  pw2.addSetLM(s2, lm2);
  pw2.addSetLM(s3, lm3);

  PWLMap res1 = minAdjCompMap(pw1, pw2); 

  PWLMap res2;

  Interval i7(1, 1, 1);

  MultiInterval mi5;
  mi5.addInter(i7);
  mi5.addInter(i7); 
  mi5.addInter(i7);

  AtomSet as5(mi5);

  Set s4;
  s4.addAtomSet(as5);

  LMap lm4;
  lm4.addGO(0, 88);
  lm4.addGO(0, 3);
  lm4.addGO(0, 173);

  res2.addSetLM(s4, lm4);

  BOOST_CHECK(res1 == res2);
}

void TestMinAdj1(){
  Interval i1(10, 1, 100);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i1);

  AtomSet as1(mi1);

  Interval i2(101, 2, 200);

  MultiInterval mi2;
  mi2.addInter(i2);
  mi2.addInter(i2);

  AtomSet as2(mi2);

  Set s1;
  s1.addAtomSet(as1);
  s1.addAtomSet(as2);

  MultiInterval mi3;
  mi3.addInter(i1);
  mi3.addInter(i2);

  AtomSet as3(mi3);

  Set s2;
  s2.addAtomSet(as3);

  LMap lm1;
  lm1.addGO(1, 0);
  lm1.addGO(1, 0);

  LMap lm2;
  lm2.addGO(2, 0);
  lm2.addGO(2, 0);

  PWLMap pw1;
  pw1.addSetLM(s1, lm1);
  pw1.addSetLM(s2, lm2);

  Interval i3(5, 5, 50);

  MultiInterval mi4;
  mi4.addInter(i3);
  mi4.addInter(i3);

  AtomSet as4(mi4);

  Set s3;
  s3.addAtomSet(as4);

  Interval i4(51, 3, 80);
  
  MultiInterval mi5;
  mi5.addInter(i4);
  mi5.addInter(i4);

  AtomSet as5(mi5);

  Interval i6(90, 1, 150);
  Interval i7(95, 1, 150);

  MultiInterval mi6;
  mi6.addInter(i6);
  mi6.addInter(i7);

  AtomSet as6(mi6);

  Set s4;
  s4.addAtomSet(as5);
  s4.addAtomSet(as6);

  LMap lm3;
  lm3.addGO(0, 1);
  lm3.addGO(1, 0);

  LMap lm4;
  lm4.addGO(2, 3);
  lm4.addGO(2, 1);

  PWLMap pw2;
  pw2.addSetLM(s3, lm3);
  pw2.addSetLM(s4, lm4);

  PWLMap res1 = minAdjMap(pw1, pw2);

  Interval i8(180, 2, 200);
  Interval i9(202, 4, 298);

  MultiInterval mi7;
  mi7.addInter(i8);
  mi7.addInter(i9);

  AtomSet as7(mi7);

  Set s5;
  s5.addAtomSet(as7);

  Interval i10(10, 5, 50);

  MultiInterval mi8;
  mi8.addInter(i10); 
  mi8.addInter(i10); 

  AtomSet as8(mi8);
  
  Set s6;
  s6.addAtomSet(as8);

  Interval i11(101, 2, 149);

  MultiInterval mi9;
  mi9.addInter(i11);
  mi9.addInter(i11);

  AtomSet as9(mi9);

  MultiInterval mi10;
  mi10.addInter(i4);
  mi10.addInter(i4);

  AtomSet as10(mi10);

  Interval i12(90, 1, 100);
  Interval i13(95, 1, 100);

  MultiInterval mi11;
  mi11.addInter(i12);
  mi11.addInter(i13);

  AtomSet as11(mi11);

  Set s7;
  s7.addAtomSet(as9);
  s7.addAtomSet(as10);
  s7.addAtomSet(as11);

  LMap lm5;
  lm5.addGO(1, 3);
  lm5.addGO(1, 1);

  PWLMap res2;
  res2.addSetLM(s5, lm5);
  res2.addSetLM(s7, lm4);
  res2.addSetLM(s6, lm3);

  BOOST_CHECK(res1 == res2);
}

// -- Graph algorithms -------------------------------------------------------//

void TestRC1(){
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

  SBGraph g;

  SetVertexDesc v1 = boost::add_vertex(g);
  SetVertexDesc v2 = boost::add_vertex(g);
  SetVertexDesc v3 = boost::add_vertex(g);
  SetVertexDesc v4 = boost::add_vertex(g);
  SetVertexDesc v5 = boost::add_vertex(g);
  SetVertexDesc v6 = boost::add_vertex(g);
  SetVertexDesc v7 = boost::add_vertex(g);

  g[v1] = V1;
  g[v2] = V2;
  g[v3] = V3;
  g[v4] = V4;
  g[v5] = V5;
  g[v6] = V6;
  g[v7] = V7;

  SetEdgeDesc e1;
  bool b1;
  boost::tie(e1, b1) = boost::add_edge(v1, v4, g);
  SetEdgeDesc e2; 
  bool b2;
  boost::tie(e2, b2) = boost::add_edge(v2, v3, g);
  SetEdgeDesc e3;
  bool b3;
  boost::tie(e3, b3) = boost::add_edge(v5, v6, g);
  SetEdgeDesc e4;
  bool b4;
  boost::tie(e4, b4) = boost::add_edge(v5, v4, g);
  SetEdgeDesc e5;
  bool b5;
  boost::tie(e5, b5) = boost::add_edge(v7, v3, g);

  g[e1] = E1;
  g[e2] = E2;
  g[e3] = E3;
  g[e4] = E4;
  g[e5] = E5;

  PWLMap res1 = connectedComponents(g);
  cout << res1 << "\n";

  BOOST_CHECK(true);
}

void TestGraph3c(){
  float offsp = 0;
  float offsn = 1;
  float offgp = 2;
  float offrp = 1000;
  float offrn = 2 * offrp;
  float offcp = 3 * offrp;
  float offcn = 4 * offrp;

  Interval i1(1 + offsp, 1, 1 + offsp);
  MultiInterval mi1;
  mi1.addInter(i1);
  AtomSet as1(mi1);
  Set sp;
  sp.addAtomSet(as1);
  SetVertex V1("V1", 1, sp, 0);

  Interval i2(1 + offsn, 1, 1 + offsn);
  MultiInterval mi2;
  mi2.addInter(i2);
  AtomSet as2(mi2);
  Set sn;
  sn.addAtomSet(as2);
  SetVertex V2("V2", 2, sn, 0);

  Interval i3(1 + offgp, 1, 1 + offgp);
  MultiInterval mi3;
  mi3.addInter(i3);
  AtomSet as3(mi3);
  Set gp;
  gp.addAtomSet(as3);
  SetVertex V3("V3", 3, gp, 0);

  Interval i4(1 + offrp, 1, offrp + offrp);
  MultiInterval mi4;
  mi4.addInter(i4);
  AtomSet as4(mi4);
  Set rp;
  rp.addAtomSet(as4);
  SetVertex V4("V4", 4, rp, 0);

  Interval i5(1 + offrn, 1, offrp + offrn);
  MultiInterval mi5;
  mi5.addInter(i5);
  AtomSet as5(mi5);
  Set rn;
  rn.addAtomSet(as5);
  SetVertex V5("V5", 5, rn, 0);

  Interval i6(1 + offcp, 1, offrp + offcp);
  MultiInterval mi6;
  mi6.addInter(i6);
  AtomSet as6(mi6);
  Set cp;
  cp.addAtomSet(as6);
  SetVertex V6("V6", 6, cp, 0);

  Interval i7(1 + offcn, 1, offrp + offcn);
  MultiInterval mi7;
  mi7.addInter(i7);
  AtomSet as7(mi7);
  Set cn;
  cn.addAtomSet(as7);
  SetVertex V7("V7", 7, cn, 0);

  float offE1 = 0;
  float offE2 = 1;
  float offE3 = 2;
  float offE4 = 2 + offrp;
  float offE5 = 2 + 2 * offrp - 1;
  float offE6 = 2 + 2 * offrp;

  Interval i8(1 + offE1, 1, offE2);
  MultiInterval mi8;
  mi8.addInter(i8);
  AtomSet as8(mi8);
  Set domE1;
  domE1.addAtomSet(as8);
  LMap lm1;
  lm1.addGO(0, 1 + offsp);
  LMap lm2;
  lm2.addGO(0, 1 + offrp);
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
  lm3.addGO(0, 1 + offsn);
  LMap lm4;
  lm4.addGO(0, 1 + offgp);
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
  lm5.addGO(1, offrn - offE3); 
  LMap lm6;
  lm6.addGO(1, offcp - offE3);
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
  lm7.addGO(1, offrn - offE4);
  LMap lm8;
  lm8.addGO(1, 1 + offrp - offE4);
  PWLMap mapE4rn;
  mapE4rn.addSetLM(domE4, lm7);
  PWLMap mapE4rp;
  mapE4rp.addSetLM(domE4, lm8);
  SetEdge E4("E4", 4, mapE4rn, mapE4rp, 0);

  Interval i12(1 + offE5, 1, offE6);
  MultiInterval mi12;
  mi12.addInter(i12);
  AtomSet as12(mi12);
  Set domE5;
  domE5.addAtomSet(as12);
  LMap lm9;
  lm9.addGO(1, offcn - offE5);
  LMap lm10;
  lm10.addGO(0, 1 + offgp);
  PWLMap mapE5cn;
  mapE5cn.addSetLM(domE5, lm9);
  PWLMap mapE5gp;
  mapE5gp.addSetLM(domE5, lm10);
  SetEdge E5("E5", 1, mapE5cn, mapE5gp, 0);

  Interval i13(1 + offE6, 1, 1 + offE6 + offrp - 2);
  MultiInterval mi13;
  mi13.addInter(i13);
  AtomSet as13(mi13);
  Set domE6;
  domE6.addAtomSet(as13);
  LMap lm11;
  lm11.addGO(1, offcn - offE6);
  LMap lm12;
  lm12.addGO(1, offcn - offE6 + 1);
  PWLMap mapE6cn;
  mapE6cn.addSetLM(domE6, lm11);
  PWLMap mapE6cn2;
  mapE6cn2.addSetLM(domE6, lm12); 
  SetEdge E6("E6", 1, mapE6cn, mapE6cn2, 0);

  SBGraph g;

  SetVertexDesc v1 = boost::add_vertex(g);
  SetVertexDesc v2 = boost::add_vertex(g);
  SetVertexDesc v3 = boost::add_vertex(g);
  SetVertexDesc v4 = boost::add_vertex(g);
  SetVertexDesc v5 = boost::add_vertex(g);
  SetVertexDesc v6 = boost::add_vertex(g);
  SetVertexDesc v7 = boost::add_vertex(g);

  g[v1] = V1;
  g[v2] = V2;
  g[v3] = V3;
  g[v4] = V4;
  g[v5] = V5;
  g[v6] = V6;
  g[v7] = V7;

  SetEdgeDesc e1;
  bool b1;
  boost::tie(e1, b1) = boost::add_edge(v1, v4, g);
  SetEdgeDesc e2; 
  bool b2;
  boost::tie(e2, b2) = boost::add_edge(v2, v3, g);
  SetEdgeDesc e3;
  bool b3;
  boost::tie(e3, b3) = boost::add_edge(v5, v6, g);
  SetEdgeDesc e4;
  bool b4;
  boost::tie(e4, b4) = boost::add_edge(v5, v4, g);
  SetEdgeDesc e5;
  bool b5;
  boost::tie(e5, b5) = boost::add_edge(v7, v3, g);
  SetEdgeDesc e6;
  bool b6;
  boost::tie(e6, b6) = boost::add_edge(v7, v7, g);

  g[e1] = E1;
  g[e2] = E2;
  g[e3] = E3;
  g[e4] = E4;
  g[e5] = E5;
  g[e6] = E6;

  PWLMap res1 = connectedComponents(g);
  cout << "res1: " << res1 << "\n";

  BOOST_CHECK(true); 
}

void Test2D(){
  float offsp1 = 0;
  float offsn1 = 1;
  float offgp1 = 2;
  float offcl1 = 1000;
  float offcr1 = 2 * offcl1;
  float offcu1 = 3 * offcl1;
  float offcd1 = 4 * offcl1;

  float offsp2 = 0;
  float offsn2 = 1;
  float offgp2 = 2;
  float offcl2 = 100;
  float offcr2 = 2 * offcl2;
  float offcu2 = 3 * offcl2;
  float offcd2 = 4 * offcl2;

  Interval i1(1 + offsp1, 1, 1 + offsp1);
  Interval i2(1 + offsp2, 1, 1 + offsp2);
  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);
  AtomSet as1(mi1);
  Set sp;
  sp.addAtomSet(as1);
  SetVertex V1("V1", 1, sp, 0);
 
  Interval i3(1 + offsn1, 1, 1 + offsn1);
  Interval i4(1 + offsn2, 1, 1 + offsn2);
  MultiInterval mi2;
  mi2.addInter(i3);
  mi2.addInter(i4);
  AtomSet as2(mi2);
  Set sn;
  sn.addAtomSet(as2);
  SetVertex V2("V2", 2, sn, 0);

  Interval i5(1 + offgp1, 1, 1 + offgp1);
  Interval i6(1 + offgp2, 1, 1 + offgp2);
  MultiInterval mi3;
  mi3.addInter(i5);
  mi3.addInter(i6);
  AtomSet as3(mi3);
  Set gp;
  gp.addAtomSet(as3);
  SetVertex V3("V3", 3, gp, 0);

  Interval i7(1 + offcl1, 1, offcr1);
  Interval i8(1 + offcl2, 1, offcr2);
  MultiInterval mi4;
  mi4.addInter(i7);
  mi4.addInter(i8);
  AtomSet as4(mi4);
  Set cl;
  cl.addAtomSet(as4);
  SetVertex V4("V4", 4, cl, 0);

  Interval i9(1 + offcr1, 1, offcu1);
  Interval i10(1 + offcr2, 1, offcu2);
  MultiInterval mi5;
  mi5.addInter(i9);
  mi5.addInter(i10);
  AtomSet as5(mi5);
  Set cr;
  cr.addAtomSet(as5);
  SetVertex V5("V5", 5, cr, 0);

  Interval i11(1 + offcu1, 1, offcd1); 
  Interval i12(1 + offcu2, 1, offcd2);
  MultiInterval mi6;
  mi6.addInter(i11);
  mi6.addInter(i12);
  AtomSet as6(mi6);
  Set cu;
  cu.addAtomSet(as6);
  SetVertex V6("V6", 6, cu, 0);

  Interval i13(1 + offcd1, 1, offcl1 + offcd1);
  Interval i14(1 + offcd2, 1, offcl2 + offcd2);
  MultiInterval mi7;
  mi7.addInter(i13);
  mi7.addInter(i14);
  AtomSet as7(mi7);
  Set cd;
  cd.addAtomSet(as7);
  SetVertex V7("V7", 7, cd, 0);

  cout << "sp: " << sp << "\n";
  cout << "sn: " << sn << "\n";
  cout << "gp: " << gp << "\n";
  cout << "cd: " << cd << "\n";
  cout << "cl: " << cl << "\n";
  cout << "cu: " << cu << "\n";
  cout << "cr: " << cr << "\n";

  float offE11 = 0;
  float offE12 = 0;
  float offE21 = offE11 + offcl1;
  float offE22 = offE12 + offcl2 - 1;
  float offE31 = offE21 + offcl1 - 1;
  float offE32 = offE22 + offcl2;
  float offE41 = offE31 + offcl1;
  float offE42 = offE32 + 1;
  float offE51 = offE41 + 1; 
  float offE52 = offE42 + offcl2;
  float offE61 = offE51 + 1;
  float offE62 = offE52 + offcl2;
  float offE71 = offE61 + 1;
  float offE72 = offE62 + 1;

  Interval i15(1 + offE11, 1, offE21);
  Interval i16(1 + offE12, 1, offE22);
  MultiInterval mi8;
  mi8.addInter(i15);
  mi8.addInter(i16);
  AtomSet as8(mi8);
  Set domE1;
  domE1.addAtomSet(as8);
  LMap lm1;
  lm1.addGO(1, offcl1 - offE11);
  lm1.addGO(1, offcl2 - offE12 + 1);
  LMap lm2;
  lm2.addGO(1, offcr1 - offE11);
  lm2.addGO(1, offcr2 - offE12);
  PWLMap mapE1cl;
  mapE1cl.addSetLM(domE1, lm1);
  PWLMap mapE1cr;
  mapE1cr.addSetLM(domE1, lm2);
  SetEdge E1("E1", 1, mapE1cl, mapE1cr, 0);

  cout << "cl, cr: " << mapE1cl << ", " << mapE1cr << "\n";

  Interval i17(1 + offE21, 1, offE31);
  Interval i18(1 + offE22, 1, offE32);
  MultiInterval mi9;
  mi9.addInter(i17);
  mi9.addInter(i18);
  AtomSet as9(mi9);
  Set domE2;
  domE2.addAtomSet(as9);
  LMap lm3;
  lm3.addGO(1, offcu1 - offE21 + 1);
  lm3.addGO(1, offcu2 - offE22);
  LMap lm4;
  lm4.addGO(1, offcd1 - offE21);
  lm4.addGO(1, offcd2 - offE22);
  PWLMap mapE2cu;
  mapE2cu.addSetLM(domE2, lm3);
  PWLMap mapE2cd;
  mapE2cd.addSetLM(domE2, lm4); 
  SetEdge E2("E2", 2, mapE2cu, mapE2cd, 0);

  cout << "cu, cd: " << mapE2cu << ", " << mapE2cd << "\n";

  Interval i19(1 + offE31, 1, offE41);
  Interval i20(1 + offE32, 1, offE42); 
  MultiInterval mi10;
  mi10.addInter(i19);
  mi10.addInter(i20);
  AtomSet as10(mi10);
  Set domE3;
  domE3.addAtomSet(as10);
  LMap lm5;
  lm5.addGO(1, offcl1 - offE31);
  lm5.addGO(1, offcl2 - offE32);
  LMap lm6;
  lm6.addGO(1, offcr1 - offE31);
  lm6.addGO(1, offcr2 - offE32 + offcl2 - 1);
  PWLMap mapE3cl;
  mapE3cl.addSetLM(domE3, lm5);
  PWLMap mapE3cr;
  mapE3cr.addSetLM(domE3, lm6);
  SetEdge E3("E3", 3, mapE3cl, mapE3cr, 0);

  cout << "cl, cr: " << mapE3cl << ", " << mapE3cr << "\n";

  Interval i21(1 + offE41, 1, offE51);
  Interval i22(1 + offE42, 1, offE52);
  MultiInterval mi11;
  mi11.addInter(i21);
  mi11.addInter(i22);
  AtomSet as11(mi11);
  Set domE4;
  domE4.addAtomSet(as11);
  LMap lm7;
  lm7.addGO(1, offcu1 - offE41);
  lm7.addGO(1, offcu2 - offE42);
  LMap lm8;
  lm8.addGO(0, offsp1 + 1);
  lm8.addGO(0, offsp2 + 1);
  PWLMap mapE4cu;
  mapE4cu.addSetLM(domE4, lm7);
  PWLMap mapE4sp;
  mapE4sp.addSetLM(domE4, lm8);
  SetEdge E4("E4", 4, mapE4cu, mapE4sp, 0);

  cout << "cu, sp: " << mapE4cu << ", " << mapE4sp << "\n";

  Interval i23(1 + offE51, 1, offE61);
  Interval i24(1 + offE52, 1, offE62);
  MultiInterval mi12;
  mi12.addInter(i23);
  mi12.addInter(i24);
  AtomSet as12(mi12);
  Set domE5;
  domE5.addAtomSet(as12);
  LMap lm9;
  lm9.addGO(1, offcd1 - offE51 + offcl1 - 1);
  lm9.addGO(1, offcd2 - offE52);
  LMap lm10;
  lm10.addGO(0, offgp1 + 1);
  lm10.addGO(0, offgp2 + 1);
  PWLMap mapE5cd;
  mapE5cd.addSetLM(domE5, lm9);
  PWLMap mapE5gp;
  mapE5gp.addSetLM(domE5, lm10);
  SetEdge E5("E5", 5, mapE5cd, mapE5gp, 0);

  cout << "cd, gp: " << mapE5cd << ", " << mapE5gp << "\n";
 
  Interval i25(1 + offE61, 1, offE71);
  Interval i26(1 + offE62, 1, offE72);
  MultiInterval mi13;
  mi13.addInter(i25);
  mi13.addInter(i26);
  AtomSet as13(mi13);
  Set domE6;
  domE6.addAtomSet(as13);
  LMap lm11;
  lm11.addGO(0, offsn1 + 1);
  lm11.addGO(0, offsn2 + 1);
  LMap lm12;
  lm12.addGO(0, offgp1 + 1);
  lm12.addGO(0, offgp2 + 1);
  PWLMap mapE6sn;
  mapE6sn.addSetLM(domE6, lm11);
  PWLMap mapE6gp;
  mapE6gp.addSetLM(domE6, lm12);
  SetEdge E6("E6", 6, mapE6sn, mapE6gp, 0); 

  cout << "sn, gp: " << mapE6sn << ", " << mapE6gp << "\n";

  SBGraph g;

  SetVertexDesc v1 = boost::add_vertex(g);
  SetVertexDesc v2 = boost::add_vertex(g);
  SetVertexDesc v3 = boost::add_vertex(g);
  SetVertexDesc v4 = boost::add_vertex(g);
  SetVertexDesc v5 = boost::add_vertex(g);
  SetVertexDesc v6 = boost::add_vertex(g);
  SetVertexDesc v7 = boost::add_vertex(g);

  g[v1] = V1;
  g[v2] = V2;
  g[v3] = V3;
  g[v4] = V4;
  g[v5] = V5;
  g[v6] = V6;
  g[v7] = V7;

  SetEdgeDesc e1;
  bool b1;
  boost::tie(e1, b1) = boost::add_edge(v4, v5, g);
  SetEdgeDesc e2; 
  bool b2;
  boost::tie(e2, b2) = boost::add_edge(v6, v7, g);
  SetEdgeDesc e3;
  bool b3;
  boost::tie(e3, b3) = boost::add_edge(v4, v5, g);
  SetEdgeDesc e4;
  bool b4;
  boost::tie(e4, b4) = boost::add_edge(v6, v1, g);
  SetEdgeDesc e5;
  bool b5;
  boost::tie(e5, b5) = boost::add_edge(v7, v3, g);
  SetEdgeDesc e6;
  bool b6;
  boost::tie(e6, b6) = boost::add_edge(v2, v3, g);

  g[e1] = E1;
  g[e2] = E2;
  g[e3] = E3;
  g[e4] = E4;
  g[e5] = E5;
  g[e6] = E6;

  PWLMap res1 = connectedComponents(g);
  cout << "res1: " << res1 << "\n";

  Graph::GraphPrinter gp3(g, -1);
  std::string fn3("graph3.dot");
  gp3.printGraph(fn3);
  
  BOOST_CHECK(true);
}

//____________________________________________________________________________//

test_suite *init_unit_test_suite(int, char *[]){
  framework::master_test_suite().p_name.value = "Set Based Graphs";

  framework::master_test_suite().add(BOOST_TEST_CASE(&TestIntCreation1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestIntCreation2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestIntCreation3));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestIntCreation4));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestIntQuery1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestIntQuery2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestIntQuery3));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestIntQuery4));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestIntQuery5));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestIntCap1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestIntCap2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestIntCap3));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestIntCap4));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestIntCap5));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestIntDiff1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestIntDiff2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestIntDiff3));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestIntDiff4));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestIntMin1));

  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMultiCreation1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMultiCreation2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMultiEmpty1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMultiEmpty2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMultiEmpty3));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMultiQuery1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMultiQuery2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMultiQuery3));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMultiAddInter1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMultiAddInter2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMultiCap1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMultiCap2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMultiCap3));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMultiCap4));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMultiDiff1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMultiDiff2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMultiDiff3));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMultiDiff4));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMultiDiff5));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMultiCrossProd1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMultiMin1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMultiReplace1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMultiReplace2));

  framework::master_test_suite().add(BOOST_TEST_CASE(&TestASetCreation1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestASetEmpty1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestASetEmpty2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestASetEmpty3));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestASetCap1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestASetCap2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestASetDiff1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestASetMin1));

  framework::master_test_suite().add(BOOST_TEST_CASE(&TestSetCreation1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestCompSets1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestSetEmpty1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestAddASets1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestSetCap1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestSetCap2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestSetCap3));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestSetCap4));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestSetDiff1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestSetMin1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestSetMin2));

  framework::master_test_suite().add(BOOST_TEST_CASE(&TestLMCreation1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestLMCompose1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestLMCompose2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestInvLMap1));

  framework::master_test_suite().add(BOOST_TEST_CASE(&TestPWAtomCreation1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestPWAtomCreation2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestPWAtomCreation3));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestPWAtomImage1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestPWAtomImage2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestPWAtomImage3));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestPWAtomImage4));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestPWAtomPre1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestPWAtomPre2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestPWAtomPre3));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestPWAtomPre4));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestPWAtomMin1));

  framework::master_test_suite().add(BOOST_TEST_CASE(&TestPWLMapCreation1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestPWLMapImage1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestPWLMapImage2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestPWLMapPre1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestPWLMapComp1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestPWLMapComp2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestPWLMapMinInvComp1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestPWLMapCombine1));

  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMinAtomPW1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMinAtomPW2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMinAtomPW3));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMinPW1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMinPW2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMinMap1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestReduce1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMapInf1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMapInf2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMapInf3));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMinAdjComp1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMinAdjComp2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMinAdjComp3));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMinAdjComp4));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMinAdj1));

  framework::master_test_suite().add(BOOST_TEST_CASE(&TestRC1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestGraph3c));
  framework::master_test_suite().add(BOOST_TEST_CASE(&Test2D));

  return 0;
}

//____________________________________________________________________________//

// EOF
