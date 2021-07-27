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

#include <util/graph/sbg/sbg.h>
#include <util/graph/sbg/sbg_algorithms.h>
#include <util/graph/sbg/sbg_printer.h>

using namespace boost::unit_test;

using namespace SBG;

//____________________________________________________________________________//

// -- Intervals --------------------------------------------------------------//

void TestIntCreation1()
{
  Interval i(10, 3, 3);

  BOOST_REQUIRE_MESSAGE(i.empty(), "\nInterval " << i << " isn't empty");
}

void TestIntCreation2()
{
  Interval i(10, 20, 15);

  BOOST_REQUIRE_MESSAGE(i.hi() == 10, "\n" << i << " upper bound isn't 10");
}

void TestIntCreation3()
{
  Interval i(10, 5, 23);

  BOOST_REQUIRE_MESSAGE(i.hi() == 20, "\n" << i << " upper bound isn't 20");
}

void TestIntCreation4()
{
  Interval i(10, 1, Inf);

  BOOST_REQUIRE_MESSAGE(i.hi() == Inf, "\n" << i << " upper bound isn't Inf");
}

void TestIntQuery1()
{
  Interval i(10, 2, 20);

  BOOST_REQUIRE_MESSAGE(!i.isIn(13), "\n13 shouldn't be in " << i);
}

void TestIntQuery2()
{
  Interval i(10, 2, 20);

  BOOST_REQUIRE_MESSAGE(i.isIn(18), "\n18 should be in " << i);
}

void TestIntQuery3()
{
  Interval i(10, 2, 20);

  BOOST_REQUIRE_MESSAGE(!i.isIn(100), "\n100 is shouldn't be in " << i);
}

void TestIntQuery4()
{
  Interval i1(10, 2, 20);
  Interval i2(0, 3, 25);

  bool b1 = i1.isIn(12);
  bool b2 = i2.isIn(12);

  BOOST_REQUIRE_MESSAGE(b1 && b2, "\n12 should be in " << i1 << " and " << i2);
}

void TestIntQuery5()
{
  Interval i(true);

  BOOST_REQUIRE_MESSAGE(!i.isIn(10), "\n" << i << " should be empty");
}

void TestIntCap1()
{
  Interval i1(10, 2, 20);
  Interval i2(0, 3, 25);

  Interval i3(i1.cap(i2));
  Interval i4(i2.cap(i1));

  BOOST_REQUIRE_MESSAGE(i3 == i4, "\nIntersection should be commutative");
}

void TestIntCap2()
{
  Interval i1(10, 2, 20);
  Interval i2(0, 3, 25);

  Interval i3 = i1.cap(i2);

  Interval i4(12, 6, 18);

  BOOST_REQUIRE_MESSAGE(i3 == i4, "\n" << i1 << " cap " << i2 << " = " << i3 << "\nExpected: " << i4);
}

void TestIntCap3()
{
  Interval i1(14, 2, 16);
  Interval i2(12, 3, 15);

  Interval i3 = i1.cap(i2);

  Interval i4(true);

  BOOST_REQUIRE_MESSAGE(i3 == i4, "\n" << i1 << " cap " << i2 << " = " << i3 << "\nExpected: " << i4);
}

void TestIntCap4()
{
  Interval i1(14, 2, 28);
  Interval i2(0, 1, Inf);

  Interval i3 = i1.cap(i2);

  Interval i4(14, 2, 28);

  BOOST_REQUIRE_MESSAGE(i3 == i4, "\n" << i1 << " cap " << i2 << " = " << i3 << "\nExpected: " << i4);
}

void TestIntCap5()
{
  Interval i1(1, 1, 10);

  Interval i2 = i1.cap(i1);

  BOOST_REQUIRE_MESSAGE(i1 == i2, "\n" << i1 << " cap " << i1 << " = " << i2 << "\nExpected: " << i1);
}

void TestIntDiff1()
{
  Interval i1(0, 2, 30);
  Interval i2(true);

  UNORD_INTERS res1 = i1.diff(i2);

  UNORD_INTERS res2;
  res2.insert(i1);

  BOOST_REQUIRE_MESSAGE(res1 == res2, "\n" << i1 << " - " << i2 << " = " << res1 << "\nExpected: " << res2);
}

void TestIntDiff2()
{
  Interval i1(0, 2, 30);
  Interval i2(10, 3, 40);

  UNORD_INTERS res1 = i1.diff(i2);

  Interval i3(0, 2, 8);
  Interval i4(12, 6, 24);
  Interval i5(14, 6, 26);
  Interval i6(30, 2, 30);

  UNORD_INTERS res2;
  res2.insert(i3);
  res2.insert(i4);
  res2.insert(i5);
  res2.insert(i6);

  BOOST_REQUIRE_MESSAGE(res1 == res2, "\n" << i1 << " - " << i2 << " = " << res1 << "\nExpected: " << res2);
}

void TestIntDiff3()
{
  Interval i1(0, 2, Inf);
  Interval i2(10, 3, 40);

  UNORD_INTERS res1 = i1.diff(i2);

  Interval i3(0, 2, 8);
  Interval i4(12, 6, 36);
  Interval i5(14, 6, 38);
  Interval i6(42, 2, Inf);

  UNORD_INTERS res2;
  res2.insert(i3);
  res2.insert(i4);
  res2.insert(i5);
  res2.insert(i6);

  BOOST_REQUIRE_MESSAGE(res1 == res2, "\n" << i1 << " - " << i2 << " = " << res1 << "\nExpected: " << res2);
}

void TestIntDiff4()
{
  Interval i1(0, 1, 10);
  Interval i2(true);

  UNORD_INTERS res1 = i1.diff(i1);

  UNORD_INTERS res2;

  BOOST_REQUIRE_MESSAGE(res1 == res2, "\n" << i1 << " - " << i2 << " = " << res1 << "\nExpected: " << res2);
}

void TestIntMin1()
{
  Interval i(10, 3, 40);

  INT res1 = i.minElem();

  BOOST_REQUIRE_MESSAGE(res1 == 10, "\n" << "min(" << i << ")" << " = " << res1 << "\nExpected: " << 10);
}

// -- MultiIntervals --------------------------------------------------------------//

void TestMultiCreation1()
{
  Interval i1(1, 1, 10);
  Interval i2(true);

  MultiInterval res1;
  res1.addInter(i1);
  res1.addInter(i2);
  res1.addInter(i2);

  BOOST_REQUIRE_MESSAGE(!res1.empty(), "\n" << res1 << " shouldn't be empty");
}

void TestMultiCreation2()
{
  Interval i1(1, 1, 10);
  Interval i2(0, 2, 50);
  Interval i3(3, 1, 5);
  Interval i4(3, 8, 24);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);
  mi1.addInter(i3);
  mi1.addInter(i4);

  ORD_INTERS res;
  ORD_INTERS::iterator it = res.begin();
  it = res.insert(it, i1);
  ++it;
  it = res.insert(it, i2);
  ++it;
  it = res.insert(it, i3);
  ++it;
  res.insert(it, i4);

  MultiInterval mi2(res);

  BOOST_REQUIRE_MESSAGE(mi1 == mi2, "\n" << mi1 << " should be equal to " << mi2);
}

void TestMultiEmpty1()
{
  MultiInterval mi;

  BOOST_REQUIRE_MESSAGE(mi.empty(), "\n" << mi << " should be empty");
}

void TestMultiEmpty2()
{
  Interval i1(true);
  Interval i2(true);
  Interval i3(true);

  MultiInterval mi;
  mi.addInter(i1);
  mi.addInter(i2);
  mi.addInter(i3);

  BOOST_REQUIRE_MESSAGE(mi.empty(), "\n" << mi << " should be empty");
}

void TestMultiEmpty3()
{
  Interval i1(true);
  Interval i2(0, 1, 10);
  Interval i3(true);

  MultiInterval mi;
  mi.addInter(i1);
  mi.addInter(i2);
  mi.addInter(i3);

  BOOST_REQUIRE_MESSAGE(!mi.empty(), "\n" << mi << " shouldn't be empty");
}

void TestMultiQuery1()
{
  Interval i1(1, 1, 10);
  Interval i2(true);
  Interval i3(10, 2, 21);

  MultiInterval mi;
  mi.addInter(i1);
  mi.addInter(i2);
  mi.addInter(i3);

  ORD_INTS elem1;
  ORD_INTS::iterator it1 = elem1.begin();
  it1 = elem1.insert(it1, 5);
  ++it1;
  it1 = elem1.insert(it1, 10);
  ++it1;
  elem1.insert(it1, 21);

  BOOST_REQUIRE_MESSAGE(!mi.isIn(elem1), "\nElem shouldn't be in " << mi);
}

void TestMultiQuery2()
{
  Interval i1(1, 1, 10);
  Interval i2(10, 20, 10);
  Interval i3(10, 2, 21);

  MultiInterval mi;
  mi.addInter(i1);
  mi.addInter(i2);
  mi.addInter(i3);

  ORD_INTS elem1;
  ORD_INTS::iterator it1 = elem1.begin();
  it1 = elem1.insert(it1, 5);
  ++it1;
  it1 = elem1.insert(it1, 10);
  ++it1;
  elem1.insert(it1, 21);

  BOOST_REQUIRE_MESSAGE(!mi.isIn(elem1), "\nElem shouldn't be in " << mi);
}

void TestMultiQuery3()
{
  Interval i1(1, 1, 10);
  Interval i2(10, 20, 10);
  Interval i3(10, 2, 21);

  MultiInterval mi;
  mi.addInter(i1);
  mi.addInter(i2);
  mi.addInter(i3);

  ORD_INTS elem1;
  ORD_INTS::iterator it1 = elem1.begin();
  it1 = elem1.insert(it1, 5);
  ++it1;
  it1 = elem1.insert(it1, 10);
  ++it1;
  elem1.insert(it1, 20);

  BOOST_REQUIRE_MESSAGE(mi.isIn(elem1), "\nElem shouldn't be in " << mi);
}

void TestMultiAddInter1()
{
  Interval i1(0, 2, 10);

  MultiInterval mi1;
  mi1.addInter(i1);

  ORD_INTERS ints2;
  ORD_INTERS::iterator it2 = ints2.begin();
  ints2.insert(it2, i1);

  MultiInterval mi2(ints2);

  BOOST_REQUIRE_MESSAGE(mi1 == mi2, "\n" << mi1 << " should be equal to " << mi2);
}

void TestMultiAddInter2()
{
  Interval i1(0, 2, 10);
  Interval i2(30, 2, 40);
  Interval i3(25, 1, 30);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);
  mi1.addInter(i3);

  ORD_INTERS ints2;
  ORD_INTERS::iterator it2 = ints2.end();
  it2 = ints2.insert(it2, i1);
  ++it2;
  it2 = ints2.insert(it2, i2);
  ++it2;
  ints2.insert(it2, i3);

  MultiInterval mi2(ints2);

  BOOST_REQUIRE_MESSAGE(mi1 == mi2, "\n" << mi1 << " should be equal to " << mi2);
}

void TestMultiCap1()
{
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

  BOOST_REQUIRE_MESSAGE(mi2 == mi3 && mi3 == mi4, "\n" << mi2 << " should be equal to " << mi3 << " and " << mi4);
}

void TestMultiCap2()
{
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

  ORD_INTERS aux;

  MultiInterval res1(aux);

  MultiInterval res2 = mi1.cap(mi2);
  MultiInterval res3 = mi2.cap(mi1);

  BOOST_REQUIRE_MESSAGE(res1 == res2 && res2 == res3, "\n" << res1 << " should be equal to " << res2 << " and " << res3);
}

void TestMultiCap3()
{
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

  BOOST_REQUIRE_MESSAGE(mi3.empty(), "\n" << mi3 << " should be empty");
}

void TestMultiCap4()
{
  Interval i1(1, 1, 10);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i1);
  mi1.addInter(i1);

  MultiInterval mi2 = mi1.cap(mi1);

  BOOST_REQUIRE_MESSAGE(mi1 == mi2, "\n" << mi1 << " should be equal to " << mi2);
}

void TestMultiDiff1()
{
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

  UNORD_MI res1 = mi1.diff(mi2);

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

  UNORD_MI res2;
  res2.insert(mi3);
  res2.insert(mi4);
  res2.insert(mi5);
  res2.insert(mi6);
  res2.insert(mi7);
  res2.insert(mi8);

  BOOST_REQUIRE_MESSAGE(res1 == res2, "\n" << mi1 << " - " << mi2 << " = " << res1 << "\nExpected: " << res2);
}

void TestMultiDiff2()
{
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

  UNORD_MI res1 = mi1.diff(mi2);

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

  UNORD_MI res2;
  res2.insert(mi3);
  res2.insert(mi4);
  res2.insert(mi5);
  res2.insert(mi6);
  res2.insert(mi7);

  BOOST_REQUIRE_MESSAGE(res1 == res2, "\n" << mi1 << " - " << mi2 << " = " << res1 << "\nExpected: " << res2);
}

void TestMultiDiff3()
{
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

  UNORD_MI res1 = mi1.diff(mi2);

  BOOST_REQUIRE_MESSAGE(res1.empty(), "\n" << mi1 << " - " << mi2 << " = " << res1 << " should be empty");
}

void TestMultiDiff4()
{
  Interval i1(1, 1, 10);
  Interval i2(20, 3, 33);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);

  UNORD_MI res1 = mi1.diff(mi1);

  BOOST_REQUIRE_MESSAGE(res1.empty(), "\n" << mi1 << " - " << mi1 << " = " << res1 << " should be empty");
}

void TestMultiDiff5()
{
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

  UNORD_MI res1 = mi1.diff(mi2);

  BOOST_REQUIRE_MESSAGE(res1.empty(), "\n" << mi1 << " - " << mi2 << " = " << res1 << " should be empty");
}

void TestMultiCrossProd1()
{
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

  BOOST_REQUIRE_MESSAGE(res1 == res2, "\n" << mi1 << " x " << mi2 << " = " << res1 << "\nExpected: " << res2);
}

void TestMultiMin1()
{
  Interval i1(0, 1, 40);
  Interval i2(15, 3, 18);
  Interval i3(50, 2, 70);

  MultiInterval mi;

  mi.addInter(i1);
  mi.addInter(i2);
  mi.addInter(i3);

  ORD_INTS res1 = mi.minElem();

  ORD_INTS res2;
  ORD_INTS::iterator it2 = res2.begin();

  it2 = res2.insert(it2, 0);
  ++it2;
  it2 = res2.insert(it2, 15);
  ++it2;
  res2.insert(it2, 50);

  BOOST_CHECK(res1 == res2);
}

void TestMultiReplace1()
{
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

  BOOST_REQUIRE_MESSAGE(res1 == res2, "\nreplace(" << mi1 << ", 1) = " << res1 << "\nExpected: " << res2);
}

void TestMultiReplace2()
{
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

  BOOST_REQUIRE_MESSAGE(res1 == res2, "\nreplace(" << mi1 << ", 4) = " << res1 << "\nExpected: " << res2);
}

// -- AtomicSets --------------------------------------------------------------//

void TestASetCreation1()
{
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

  BOOST_REQUIRE_MESSAGE(mi == as.aset(), "\naset = " << as.aset() << "\nExpected: " << mi);
}

void TestASetEmpty1()
{
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

  BOOST_REQUIRE_MESSAGE(!as.empty(), "\n" << as << " shouldn't be empty");
}

void TestASetEmpty2()
{
  Interval i1(true);
  Interval i2(true);
  Interval i3(true);

  MultiInterval mi;
  mi.addInter(i1);
  mi.addInter(i2);
  mi.addInter(i3);

  AtomSet as(mi);

  BOOST_REQUIRE_MESSAGE(as.empty(), "\n" << as << " should be empty");
}

void TestASetEmpty3()
{
  Interval i1(true);
  Interval i2(true);
  Interval i3(1, 1, 10);

  MultiInterval mi;
  mi.addInter(i1);
  mi.addInter(i2);
  mi.addInter(i3);

  AtomSet as(mi);

  BOOST_REQUIRE_MESSAGE(!as.empty(), "\n" << as << " shouldn't be empty");
}

void TestASetCap1()
{
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

  BOOST_REQUIRE_MESSAGE(res1 == res2 && res2 == res3, "\n" << mi1 << " cap " << mi2 << " = " << res1 << "\nExpected: " << res3);
}

void TestASetCap2()
{
  Interval i1(1, 1, 10);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i1);
  mi1.addInter(i1);

  AtomSet as1(mi1);

  AtomSet as2 = as1.cap(as1);

  BOOST_REQUIRE_MESSAGE(as1 == as2, "\n" << as1 << " cap " << as2 << " = " << as2 << "\nExpected: " << as1);
}

void TestASetDiff1()
{
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

  UNORD_AS res1 = as1.diff(as2);

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

  UNORD_AS res2;
  res2.insert(as3);
  res2.insert(as4);
  res2.insert(as5);
  res2.insert(as6);
  res2.insert(as7);
  res2.insert(as8);

  BOOST_REQUIRE_MESSAGE(res1 == res2, "\n" << as1 << " - " << as2 << " = " << res1 << "\nExpected: " << res2);
}

void TestASetMin1()
{
  Interval i1(0, 1, 40);
  Interval i2(15, 3, 18);
  Interval i3(50, 2, 70);

  MultiInterval mi;

  mi.addInter(i1);
  mi.addInter(i2);
  mi.addInter(i3);

  AtomSet as1(mi);

  ORD_INTS res1 = mi.minElem();

  ORD_INTS res2;
  ORD_INTS::iterator it2 = res2.begin();

  it2 = res2.insert(it2, 0);
  ++it2;
  it2 = res2.insert(it2, 15);
  ++it2;
  res2.insert(it2, 50);

  BOOST_CHECK(res1 == res2);
}

// -- Sets -------------------------------------------------------------------//

void TestSetCreation1()
{
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

  UNORD_AS res2;
  res2.insert(as1);
  res2.insert(as2);

  Set s2(res2);

  BOOST_REQUIRE_MESSAGE(s1 == s2, "\n" << s1 << "\nshould be equal to\n" << s2);
}

void TestCompSets1()
{
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

  BOOST_REQUIRE_MESSAGE(s1 != s2, "\n" << s1 << "\nshouldn't be equal to\n" << s2);
}

void TestSetEmpty1()
{
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

  BOOST_REQUIRE_MESSAGE(!s2.empty(), "\n" << s2 << " shouldn't be empty");
}

void TestAddASets1()
{
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

  UNORD_AS aux;
  UNORD_AS::iterator itaux = aux.begin();
  aux.insert(itaux, as1);

  Set s2(aux);

  BOOST_REQUIRE_MESSAGE(s1 == s2, "\n" << s1 << "\nshould be equal to\n" << s2);
}

void TestSetCap1()
{
  Set s1;
  Set s2;

  Set res1 = s1.cap(s2);
  Set res2 = s2.cap(s1);

  BOOST_REQUIRE_MESSAGE(res1 == res2 && res1.empty() && res2.empty(), "Empty sets, should have empty intersection");
}

void TestSetCap2()
{
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

  BOOST_REQUIRE_MESSAGE(res1 == res2 && res1.empty() && res2.empty(), 
                        "\n" << s1 << "\ncap\n" << s2 << "\n=\n" << res1 << "\nExpected empty set");
}

void TestSetCap3()
{
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
  BOOST_REQUIRE_MESSAGE(res1 == res2 && res2 == res3
                        , "\n" << s1 << "\ncap\n" << s2 << "\n=\n" << res1 << "\nExpected: " << res3);
}

void TestSetCap4()
{
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

  BOOST_REQUIRE_MESSAGE(s1 == s2, "\n" << s1 << "\ncap\n" << s2 << "\n=\n" << s2 << "\nExpected: " << s1);
}

void TestSetDiff1()
{
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

  BOOST_REQUIRE_MESSAGE(res1 == res2, "\n" << s1 << "\ndiff\n" << s2 << "\n=\n" << res1 << "\nExpected: " << res2);
}

void TestSetDiff2()
{
  Interval i1(1, 1, 100);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i1);

  AtomSet as1(mi1);

  Interval i2(150, 5, 250);

  MultiInterval mi2;
  mi2.addInter(i1);
  mi2.addInter(i2);

  AtomSet as2(mi2);

  Set s1;
  s1.addAtomSet(as1);
  s1.addAtomSet(as2);

  Interval i3(3, 3, 30);

  MultiInterval mi3;
  mi3.addInter(i3);
  mi3.addInter(i3);

  AtomSet as3(mi3);

  Interval i4(75, 8, 100);
  Interval i5(100, 1, 200);

  MultiInterval mi4;
  mi4.addInter(i4);
  mi4.addInter(i5);

  AtomSet as4(mi4);

  Set s2;
  s2.addAtomSet(as3);
  s2.addAtomSet(as4);

  Set res1 = s2.diff(s1);

  Interval i6(101, 1, 149);

  MultiInterval mi5;
  mi5.addInter(i4);
  mi5.addInter(i6);

  AtomSet as5(mi5);

  Interval i7(151, 5, 200);

  MultiInterval mi6;
  mi6.addInter(i4);
  mi6.addInter(i7);

  AtomSet as6(mi6);

  Interval i8(152, 5, 200);

  MultiInterval mi7;
  mi7.addInter(i4);
  mi7.addInter(i8);

  AtomSet as7(mi7);

  Interval i9(153, 5, 200);

  MultiInterval mi8;
  mi8.addInter(i4);
  mi8.addInter(i9);

  AtomSet as8(mi8);

  Interval i10(154, 5, 200);

  MultiInterval mi9;
  mi9.addInter(i4);
  mi9.addInter(i10);

  AtomSet as9(mi9);

  Set res2;
  res2.addAtomSet(as5);
  res2.addAtomSet(as6);
  res2.addAtomSet(as7);
  res2.addAtomSet(as8);
  res2.addAtomSet(as9);

  BOOST_CHECK(res1 == res2);
  BOOST_REQUIRE_MESSAGE(res1 == res2, "\n" << s1 << "\n-\n" << s2 << "\n=\n" << res1 << "\nExpected: " << res2);
}

void TestSetMin1()
{
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

  ORD_INTS res1 = s.minElem();

  ORD_INTS res2;
  res2.insert(res2.end(), 5);

  BOOST_CHECK(res1 == res2);
}

void TestSetMin2()
{
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

  ORD_INTS res1 = s.minElem();

  ORD_INTS res2;
  res2.insert(res2.end(), 20);
  res2.insert(res2.end(), 1);

  BOOST_CHECK(res1 == res2);
}

// -- LinearMaps -------------------------------------------------------------------//

void TestLMCreation1()
{
  LMap res;

  BOOST_REQUIRE_MESSAGE(res.empty(), "Linear map should be empty");
}

void TestLMCompose1()
{
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

  BOOST_REQUIRE_MESSAGE(res1 == res2, "\n" << lm1 << "\n°\n" << lm2 << "\n=\n" << res1 << "\nExpected: " << res2);
}

void TestLMCompose2()
{
  LMap lm1;

  lm1.addGO(5.0, 1.0);
  lm1.addGO(10.0, 2.0);
  lm1.addGO(3.0, 3.0);

  LMap lm2;

  lm2.addGO(2.0, 3.0);
  lm2.addGO(2.0, 3.0);

  LMap res1 = lm1.compose(lm2);

  BOOST_REQUIRE_MESSAGE(res1.empty(), "\n" << lm1 << "\n°\n" << lm2 << "\n=\n" << res1 << "\nExpected empty result");
}

void TestInvLMap1()
{
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
  BOOST_REQUIRE_MESSAGE(res1 == res2, "\ninv(" << lm1 << ") = " << res1 << "\nExpected: " << res2);
}

// -- Piece wise atomic linear maps ------------------------------------------//

void TestAtomPWCreation1()
{
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

  AtomPWLMap pwatom1(as1, lm1);

  BOOST_REQUIRE_MESSAGE(pwatom1.empty(), "\n" << pwatom1 << " should be empty");
}

void TestAtomPWCreation2()
{
  Interval i1(1, 1, 10);
  Interval i2(1, 1, 10);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);

  AtomSet as1(mi1);

  LMap lm1;
  lm1.addGO(0.5, 0.0);
  lm1.addGO(0.5, 0.0);

  AtomPWLMap pwatom1(as1, lm1);

  BOOST_REQUIRE_MESSAGE(pwatom1.empty(), "\n" << pwatom1 << " should be empty");
}

void TestAtomPWCreation3()
{
  Interval i1(2, 2, 10);
  Interval i2(2, 2, 10);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i2);

  AtomSet as1(mi1);

  LMap lm1;
  lm1.addGO(0.5, 0.0);
  lm1.addGO(0.5, 0.0);

  AtomPWLMap pwatom1(as1, lm1);

  BOOST_REQUIRE_MESSAGE(!pwatom1.empty(), "\n" << pwatom1 << " shouldn't be empty");
}

void TestAtomPWImage1()
{
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

  AtomPWLMap pwatom1(as1, lm1);

  AtomSet res1 = pwatom1.image(as1);

  Interval i4(2, 2, 20);
  Interval i5(3, 3, 30);
  Interval i6(1, 1, 10);

  MultiInterval mi2;
  mi2.addInter(i4);
  mi2.addInter(i5);
  mi2.addInter(i6);

  AtomSet res2(mi2);

  BOOST_REQUIRE_MESSAGE(res1 == res2, 
                        "\nimage(" << pwatom1 << ", " << as1 << ")" << "\n=\n" << res1 << "\nExpected: " << res2);
}

void TestAtomPWImage2()
{
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

  AtomPWLMap pwatom1(as1, lm1);

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

  BOOST_REQUIRE_MESSAGE(res1 == res2, 
                        "\nimage(" << pwatom1 << ", " << as2 << ")" << "\n=\n" << res1 << "\nExpected: " << res2);
}

void TestAtomPWImage3()
{
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

  AtomPWLMap pwatom1(as1, lm1);

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

  BOOST_REQUIRE_MESSAGE(res1 == res2, 
                        "\nimage(" << pwatom1 << ", " << as2 << ")" << "\n=\n" << res1 << "\nExpected: " << res2);
}

void TestAtomPWImage4()
{
  Interval i1(1, 1, 10);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i1);

  AtomSet as1(mi1);

  LMap lm1;
  lm1.addGO(0.0, 1.0);
  lm1.addGO(0.0, 1.0);

  AtomPWLMap pwatom1(as1, lm1);

  AtomSet res1 = pwatom1.image(as1);

  Interval i2(1, 0, 1);

  MultiInterval mi2;
  mi2.addInter(i2);
  mi2.addInter(i2);

  AtomSet res2(mi2);

  BOOST_CHECK(res1 == res2);
  BOOST_REQUIRE_MESSAGE(res1 == res2, 
                        "\nimage(" << pwatom1 << ", " << as1 << ")" << "\n=\n" << res1 << "\nExpected: " << res2);
}

void TestAtomPWPre1()
{
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

  AtomPWLMap pwatom1(as1, lm1);

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

  BOOST_REQUIRE_MESSAGE(res1 == res2, 
                        "\npre(" << pwatom1 << ", " << as2 << ")" << "\n=\n" << res1 << "\nExpected: " << res2);
}

void TestAtomPWPre2()
{
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

  AtomPWLMap pwatom1(as1, lm1);

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

  BOOST_REQUIRE_MESSAGE(res1 == res2, 
                        "\npre(" << pwatom1 << ", " << as2 << ")" << "\n=\n" << res1 << "\nExpected: " << res2);
}

void TestAtomPWPre3()
{
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

  AtomPWLMap pwatom1(as1, lm1);

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

  BOOST_REQUIRE_MESSAGE(res1 == res2, 
                        "\npre(" << pwatom1 << ", " << as2 << ")" << "\n=\n" << res1 << "\nExpected: " << res2);
}

void TestAtomPWPre4()
{
  Interval i1(1, 1, 10);

  MultiInterval mi1;
  mi1.addInter(i1);
  mi1.addInter(i1);

  AtomSet as1(mi1);

  LMap lm1;
  lm1.addGO(0.0, 3.0);
  lm1.addGO(0.0, 3.0);

  AtomPWLMap pwatom1(as1, lm1);

  Interval i2(1, 1, 5);

  MultiInterval mi2;
  mi2.addInter(i2);
  mi2.addInter(i2);

  AtomSet as2(mi2);

  AtomSet res1 = pwatom1.preImage(as2);

  AtomSet res2 = as1;

  BOOST_REQUIRE_MESSAGE(res1 == res2, 
                        "\npre(" << pwatom1 << ", " << as2 << ")" << "\n=\n" << res1 << "\nExpected: " << res2);
}

// -- Piece wise linear maps -------------------------------------------------//

void TestPWLMapCreation1()
{
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

  BOOST_REQUIRE_MESSAGE(pw1.empty(), "\n" << pw1 << " should be empty");
}

void TestPWLMapImage1()
{
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

  BOOST_REQUIRE_MESSAGE(res1 == res2, 
                        "\nimage(" << pw1 << ", " << aux << ")" << "\n=\n" << res1 << "\nExpected: " << res2);
}

void TestPWLMapImage2()
{
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

  BOOST_REQUIRE_MESSAGE(res1 == res2, 
                        "\nimage(" << pw1 << ", " << s1 << ")" << "\n=\n" << res1 << "\nExpected: " << res2);
}

void TestPWLMapPre1()
{
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

  BOOST_REQUIRE_MESSAGE(res1 == res2, 
                        "\npre(" << pw1 << ", " << s3 << ")" << "\n=\n" << res1 << "\nExpected: " << res2);
}

void TestPWLMapComp1()
{
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

  BOOST_REQUIRE_MESSAGE(res1.equivalentPW(res2), 
                        "\n" << pw1 << "\n°\n" << pw2 << "\n=\n" << res1 << "\nExpected: " << res2);
}

void TestPWLMapComp2()
{
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

  BOOST_REQUIRE_MESSAGE(res1.equivalentPW(res2), 
                        "\n" << pw1 << "\n°\n" << pw1 << "\n=\n" << res1 << "\nExpected: " << res2);
}

void TestPWLMapCombine1()
{
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

  BOOST_REQUIRE_MESSAGE(res1.equivalentPW(res2), 
                        "\ncombine(" << pw1 << ",\n        " << pw2 << ")\n=\n" << res1 << "\nExpected: " << res2);
}

void TestMinAS1()
{
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

  PWLMap res1 = minMapAtomSet(as1, lm1, lm2);

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

  BOOST_REQUIRE_MESSAGE(res1.equivalentPW(res2), 
                        "\nmin(" << as1 << ",  " << lm1 << ", " << lm2 << ")\n=\n" << res1 << "\nExpected: " << res2);
}

void TestMinAS2()
{
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

  PWLMap res1 = minMapAtomSet(as1, lm1, lm2);

  Set s1;
  s1.addAtomSet(as1);

  PWLMap res2;
  res2.addSetLM(s1, lm2);

  BOOST_REQUIRE_MESSAGE(res1.equivalentPW(res2), 
                        "\nmin(" << as1 << ",  " << lm1 << ", " << lm2 << ")\n=\n" << res1 << "\nExpected: " << res2);
}

void TestMinSet1()
{
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

  PWLMap res1 = minMapSet(s1, lm1, lm2);

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
  res2.addSetLM(s3, lm1);
  res2.addSetLM(s2, lm2);

  BOOST_REQUIRE_MESSAGE(res1.equivalentPW(res2), 
                        "\nmin(" << s1 << ",  " << lm1 << ", " << lm2 << ")\n=\n" << res1 << "\nExpected: " << res2);
}

void TestMinSet2()
{
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

  PWLMap res1 = minMapSet(s1, lm1, lm2);

  PWLMap res2;
  res2.addSetLM(s1, lm1);

  BOOST_REQUIRE_MESSAGE(res1.equivalentPW(res2), 
                        "\nmin(" << s1 << ",  " << lm1 << ", " << lm2 << ")\n=\n" << res1 << "\nExpected: " << res2);
}

void TestMinMap1()
{
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

  PWLMap res1 = pw1.minMap(pw2);

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

  BOOST_REQUIRE_MESSAGE(res1.equivalentPW(res2), 
                        "\nmin(" << pw1 << "\n,\n" << pw2 << ")\n=\n" << res1 << "\nExpected: " << res2);
}

void TestReduce1()
{
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

  PWLMap res1 = pw1.reduceMapN(2);

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

  BOOST_REQUIRE_MESSAGE(res1.equivalentPW(res2), 
                        "\nreduceN(" << pw1 << ", 2)\n=\n" << res1 << "\nExpected: " << res2);
}

void TestMinAdjComp1()
{
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

  PWLMap res1 = pw1.minAdjCompMap(pw2);

  PWLMap res2 = pw2.compPW(pw1);

  BOOST_REQUIRE_MESSAGE(res1.equivalentPW(res2), 
                        "\nminAdj(" << pw1 << ",\n" << pw2 << "\n=\n" << res1 << "\nExpected: " << res2);
}

void TestMinAdjComp2()
{
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

  PWLMap res1 = pw1.minAdjCompMap(pw2);

  PWLMap res2;

  BOOST_REQUIRE_MESSAGE(res1.equivalentPW(res2), 
                        "\nminAdj(" << pw1 << ",\n" << pw2 << "\n=\n" << res1 << "\nExpected: " << res2);
}

void TestMinAdjComp3()
{
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

  PWLMap res1 = pw1.minAdjCompMap(pw2);

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

  BOOST_REQUIRE_MESSAGE(res1.equivalentPW(res2), 
                        "\nminAdj(" << pw1 << ",\n" << pw2 << "\n=\n" << res1 << "\nExpected: " << res2);
}

void TestMinAdjComp4()
{
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

  PWLMap res1 = pw1.minAdjCompMap(pw2);

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

  BOOST_REQUIRE_MESSAGE(res1.equivalentPW(res2), 
                        "\nminAdj(" << pw1 << ",\n" << pw2 << "\n=\n" << res1 << "\nExpected: " << res2);
}

void TestMinAdj1()
{
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

  PWLMap res1 = pw1.minAdjMap(pw2);

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
  res2.addSetLM(s6, lm3);
  res2.addSetLM(s7, lm4);

  BOOST_REQUIRE_MESSAGE(res1.equivalentPW(res2), 
                        "\nminAdj(" << pw1 << ",\n" << pw2 << "\n=\n" << res1 << "\nExpected: " << res2);
}

// -- Connected component-----------------------------------------------------//

void TestRC1()
{
  float offSp = 0;
  float offSn = 1;
  float offGp = 2;
  float offRp = 1000;
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

  Interval i21(1, 1, 1);
  Set s21 = createSet(i21);

  LMap lm21;
  lm21.addGO(0.0, 1.0);

  Interval i22(2, 1, 2);
  Set s22 = createSet(i22);

  LMap lm22;
  lm22.addGO(0.0, 2.0);

  Interval i23(3, 1, 3);
  Set s23 = createSet(i23);

  LMap lm23;
  lm23.addGO(0.0, 2.0);

  Interval i24(1001, 1, 1001);
  Set s24 = createSet(i24);

  LMap lm24;
  lm24.addGO(0.0, 1.0);

  Interval i25(1002, 1, 2000);
  Set s25 = createSet(i25);

  LMap lm25;
  lm25.addGO(1.0, 0.0);

  Interval i26(2001, 1, 2999);
  Set s26 = createSet(i26);

  LMap lm26;
  lm26.addGO(1.0, -999.0);

  Interval i27(3000, 1, 3000);
  Set s27 = createSet(i27);

  LMap lm27;
  lm27.addGO(1.0, 0.0);

  Interval i28(3001, 1, 3999);
  Set s28 = createSet(i28);
  
  LMap lm28;
  lm28.addGO(1.0, -1999.0);

  Interval i29(4000, 1, 4000);
  Set s29 = createSet(i29);

  LMap lm29;
  lm29.addGO(1.0, -1000.0);

  Interval i30(4001, 1, 5000);
  Set s30 = createSet(i30);

  LMap lm30;
  lm30.addGO(0.0, 2.0);

  PWLMap res2;
  res2.addSetLM(s21, lm21);
  res2.addSetLM(s22, lm22);
  res2.addSetLM(s23, lm23);
  res2.addSetLM(s24, lm24);
  res2.addSetLM(s25, lm25);
  res2.addSetLM(s26, lm26);
  res2.addSetLM(s27, lm27);
  res2.addSetLM(s28, lm28);
  res2.addSetLM(s29, lm29);
  res2.addSetLM(s30, lm30);

  BOOST_CHECK(res1.equivalentPW(res2));
}

void TestGraph3c()
{
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

  Interval i21(1, 1, 1);
  Set s21 = createSet(i21);

  LMap lm21;
  lm21.addGO(0.0, 1.0);

  Interval i22(2, 1, 2);
  Set s22 = createSet(i22);

  LMap lm22;
  lm22.addGO(0.0, 2.0);

  Interval i23(3, 1, 3);
  Set s23 = createSet(i23);

  LMap lm23;
  lm23.addGO(0.0, 2.0);

  Interval i24(1001, 1, 1001);
  Set s24 = createSet(i24);

  LMap lm24;
  lm24.addGO(0.0, 1.0);

  Interval i25(1002, 1, 2000);
  Set s25 = createSet(i25);

  LMap lm25;
  lm25.addGO(1.0, 0.0);

  Interval i26(2001, 1, 2999);
  Set s26 = createSet(i26);

  LMap lm26;
  lm26.addGO(1.0, -999.0);

  Interval i27(3000, 1, 3000);
  Set s27 = createSet(i27);

  LMap lm27;
  lm27.addGO(1.0, 0.0);

  Interval i28(3001, 1, 3999);
  Set s28 = createSet(i28);
  
  LMap lm28;
  lm28.addGO(1.0, -1999.0);

  Interval i29(4000, 1, 4000);
  Set s29 = createSet(i29);

  LMap lm29;
  lm29.addGO(1.0, -1000.0);

  Interval i30(4001, 1, 4001);
  Set s30 = createSet(i30);

  LMap lm30;
  lm30.addGO(0.0, 2.0);

  Interval i31(4002, 1, 4999);
  Set s31 = createSet(i31);

  LMap lm31;
  lm31.addGO(0.0, 2.0);

  Interval i32(5000, 1, 5000);
  Set s32 = createSet(i32);

  LMap lm32;
  lm32.addGO(0.0, 2.0);

  PWLMap res2;
  res2.addSetLM(s21, lm21);
  res2.addSetLM(s22, lm22);
  res2.addSetLM(s23, lm23);
  res2.addSetLM(s24, lm24);
  res2.addSetLM(s25, lm25);
  res2.addSetLM(s26, lm26);
  res2.addSetLM(s27, lm27);
  res2.addSetLM(s28, lm28);
  res2.addSetLM(s29, lm29);
  res2.addSetLM(s30, lm30);
  res2.addSetLM(s31, lm31);
  res2.addSetLM(s32, lm32);

  BOOST_CHECK(res1.equivalentPW(res2));
}

void Test2D()
{
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

  Interval i30(1, 1, 1);
  MultiInterval mi30;
  mi30.addInter(i30); 
  mi30.addInter(i30); 
  Set s30 = createSet(mi30);

  LMap lm30;
  lm30.addGO(0.0, 1.0);
  lm30.addGO(0.0, 1.0);

  Interval i31(2, 1, 2);
  MultiInterval mi31;
  mi31.addInter(i31);
  mi31.addInter(i31);
  Set s31 = createSet(mi31);

  LMap lm31;
  lm31.addGO(0.0, 2.0);
  lm31.addGO(0.0, 2.0);

  Interval i32(3, 1, 3);
  MultiInterval mi32;
  mi32.addInter(i32);
  mi32.addInter(i32);
  Set s32 = createSet(mi32);

  LMap lm32;
  lm32.addGO(0.0, 2.0);
  lm32.addGO(0.0, 2.0);

  Interval i33(1001, 1, 2000);
  Interval i34(101, 1, 101);
  MultiInterval mi33;
  mi33.addInter(i33);
  mi33.addInter(i34);
  Set s33 = createSet(mi33);

  LMap lm33;
  lm33.addGO(1.0, 0.0);
  lm33.addGO(1.0, 0.0);

  Interval i35(1001, 1, 2000);
  Interval i36(102, 1, 200);
  MultiInterval mi35;
  mi35.addInter(i35);
  mi35.addInter(i36);
  Set s35 = createSet(mi35);

  LMap lm35;
  lm35.addGO(1.0, 0.0);
  lm35.addGO(1.0, 0.0);

  Interval i37(2001, 1, 3000);
  Interval i38(201, 1, 299);
  MultiInterval mi37;
  mi37.addInter(i37);
  mi37.addInter(i38);
  Set s37 = createSet(mi37);

  LMap lm37;
  lm37.addGO(1.0, -1000.0);
  lm37.addGO(1.0, -99.0);

  Interval i39(2001, 1, 3000);
  Interval i40(300, 1, 300);
  MultiInterval mi39;
  mi39.addInter(i39);
  mi39.addInter(i40);
  Set s39 = createSet(mi39);

  LMap lm39;
  lm39.addGO(1.0, -1000.0);
  lm39.addGO(1.0, -199.0);

  Interval i41(3001, 1, 3001);
  Interval i42(301, 1, 400);
  MultiInterval mi41;
  mi41.addInter(i41);
  mi41.addInter(i42);
  Set s41 = createSet(mi41);

  LMap lm41;
  lm41.addGO(0.0, 1.0);
  lm41.addGO(0.0, 1.0);

  Interval i43(3002, 1, 4000);
  Interval i44(301, 1, 400);
  MultiInterval mi43;
  mi43.addInter(i43);
  mi43.addInter(i44);
  Set s43 = createSet(mi43);

  LMap lm43;
  lm43.addGO(1.0, 0.0);
  lm43.addGO(1.0, 0.0);

  Interval i45(4001, 1, 4999);
  Interval i46(401, 1, 500);
  MultiInterval mi45;
  mi45.addInter(i45);
  mi45.addInter(i46);
  Set s45 = createSet(mi45);

  LMap lm45;
  lm45.addGO(1.0, -999.0);
  lm45.addGO(1.0, -100.0);

  Interval i47(5000, 1, 5000);
  Interval i48(401, 1, 500);
  MultiInterval mi47;
  mi47.addInter(i47);
  mi47.addInter(i48);
  Set s47 = createSet(mi47);

  LMap lm47;
  lm47.addGO(0.0, 2.0);
  lm47.addGO(0.0, 2.0);

  PWLMap res2;
  res2.addSetLM(s30, lm30);
  res2.addSetLM(s31, lm31);
  res2.addSetLM(s32, lm32);
  res2.addSetLM(s33, lm33);
  res2.addSetLM(s35, lm35);
  res2.addSetLM(s37, lm37);
  res2.addSetLM(s39, lm39);
  res2.addSetLM(s41, lm41);
  res2.addSetLM(s43, lm43);
  res2.addSetLM(s45, lm45);
  res2.addSetLM(s47, lm47);

  BOOST_CHECK(res1.equivalentPW(res2));
}

// -- Matching ---------------------------------------------------------------//

void TestSplit()
{
  float n = 100;

  // Equations

  float offF1 = 1;
  float offF2 = offF1 + n;
  float offF3 = offF2 + n;
  float offF4 = offF3 + n - 1;
  float offF5 = offF4 + n - 1;
  float offF6 = offF5 + 1;

  Interval i1(offF1, 1, offF1);
  MultiInterval mi1;
  mi1.addInter(i1);
  AtomSet as1(mi1);
  Set f1;
  f1.addAtomSet(as1);
  SetVertex F1("F1", 1, f1, 0);

  Interval i2(1 + offF1, 1, offF2);
  MultiInterval mi2;
  mi2.addInter(i2);
  AtomSet as2(mi2);
  Set f2;
  f2.addAtomSet(as2);
  SetVertex F2("F2", 2, f2, 0);

  Interval i3(1 + offF2, 1, offF3);
  MultiInterval mi3;
  mi3.addInter(i3);
  AtomSet as3(mi3);
  Set f3;
  f3.addAtomSet(as3);
  SetVertex F3("F3", 3, f3, 0);

  Interval i4(1 + offF3, 1, offF4);
  MultiInterval mi4;
  mi4.addInter(i4);
  AtomSet as4(mi4);
  Set f4;
  f4.addAtomSet(as4);
  SetVertex F4("F4", 4, f4, 0);

  Interval i5(1 + offF4, 1, offF5);
  MultiInterval mi5;
  mi5.addInter(i5);
  AtomSet as5(mi5);
  Set f5;
  f5.addAtomSet(as5);
  SetVertex F5("F5", 1, f5, 0);

  Interval i6(1 + offF5, 1, offF6);
  MultiInterval mi6;
  mi6.addInter(i6);
  AtomSet as6(mi6);
  Set f6;
  f6.addAtomSet(as6);
  SetVertex F6("F6", 6, f6, 0);

  // Unknowns

  float offU1 = offF6 + n;
  float offU2 = offU1 + n;
  float offU3 = offU2 + n;
  float offU4 = offU3 + n;

  Interval i7(1 + offF6, 1, offU1);
  MultiInterval mi7;
  mi7.addInter(i7);
  AtomSet as7(mi7);
  Set u1;
  u1.addAtomSet(as7);
  SetVertex U1("U1", 7, u1, 0);

  Interval i8(1 + offU1, 1, offU2);
  MultiInterval mi8;
  mi8.addInter(i8);
  AtomSet as8(mi8);
  Set u2;
  u2.addAtomSet(as8);
  SetVertex U2("U2", 8, u2, 0);

  Interval i9(1 + offU2, 1, offU3);
  MultiInterval mi9;
  mi9.addInter(i9);
  AtomSet as9(mi9);
  Set u3;
  u3.addAtomSet(as9);
  SetVertex U3("U3", 9, u3, 0);

  Interval i10(1 + offU3, 1, offU4);
  MultiInterval mi10;
  mi10.addInter(i10);
  AtomSet as10(mi10);
  Set u4;
  u4.addAtomSet(as10);
  SetVertex U4("U4", 10, u4, 0);

  // Edges

  float offE1 = 1;
  float offE2 = offE1 + n;
  float offE3 = offE2 + n;
  float offE4 = offE3 + n;
  float offE5 = offE4 + n;
  float offE6 = offE5 + n - 1;
  float offE7 = offE6 + n - 1;
  float offE8 = offE7 + n - 1;
  float offE9 = offE8 + n - 1;
  float offE10 = offE9 + 1;
  float offE11 = offE10 + 1;

  Interval i11(offE1, 1, offE1);
  MultiInterval mi11;
  mi11.addInter(i11);
  AtomSet as11(mi11);
  Set domE1;
  domE1.addAtomSet(as11);
  LMap lm1;
  lm1.addGO(1, offF1 - offE1);
  LMap lm2;
  lm2.addGO(1, 1 + offF6 - offE1);
  PWLMap mapE1l;
  mapE1l.addSetLM(domE1, lm1);
  PWLMap mapE1r;
  mapE1r.addSetLM(domE1, lm2);
  SetEdge E1("E1", 1, mapE1l, mapE1r, 0);

  Interval i12(1 + offE1, 1, offE2);
  MultiInterval mi12;
  mi12.addInter(i12);
  AtomSet as12(mi12);
  Set domE2;
  domE2.addAtomSet(as12);
  LMap lm3;
  lm3.addGO(1, offF1 - offE1);
  LMap lm4;
  lm4.addGO(1, offF6 - offE1);
  PWLMap mapE2l;
  mapE2l.addSetLM(domE2, lm3);
  PWLMap mapE2r;
  mapE2r.addSetLM(domE2, lm4);
  SetEdge E2("E2", 2, mapE2l, mapE2r, 0);

  Interval i13(1 + offE2, 1, offE3);
  MultiInterval mi13;
  mi13.addInter(i13);
  AtomSet as13(mi13);
  Set domE3;
  domE3.addAtomSet(as13);
  LMap lm5;
  lm5.addGO(1, offF1 - offE2);
  LMap lm6;
  lm6.addGO(1, offU1 - offE2);
  PWLMap mapE3l;
  mapE3l.addSetLM(domE3, lm5);
  PWLMap mapE3r;
  mapE3r.addSetLM(domE3, lm6);
  SetEdge E3("E3", 3, mapE3l, mapE3r, 0);

  Interval i14(1 + offE3, 1, offE4);
  MultiInterval mi14;
  mi14.addInter(i14);
  AtomSet as14(mi14);
  Set domE4;
  domE4.addAtomSet(as14);
  LMap lm7;
  lm7.addGO(1, offF2 - offE3);
  LMap lm8;
  lm8.addGO(1, offF6 - offE3);
  PWLMap mapE4l;
  mapE4l.addSetLM(domE4, lm7);
  PWLMap mapE4r;
  mapE4r.addSetLM(domE4, lm8);
  SetEdge E4("E4", 4, mapE4l, mapE4r, 0);

  Interval i15(1 + offE4, 1, offE5);
  MultiInterval mi15;
  mi15.addInter(i15);
  AtomSet as15(mi15);
  Set domE5;
  domE5.addAtomSet(as15);
  LMap lm9;
  lm9.addGO(1, offF2 - offE4);
  LMap lm10;
  lm10.addGO(1, offU2 - offE4);
  PWLMap mapE5l;
  mapE5l.addSetLM(domE5, lm9);
  PWLMap mapE5r;
  mapE5r.addSetLM(domE5, lm10);
  SetEdge E5("E5", 5, mapE5l, mapE5r, 0);

  Interval i16(1 + offE5, 1, offE6);
  MultiInterval mi16;
  mi16.addInter(i16);
  AtomSet as16(mi16);
  Set domE6;
  domE6.addAtomSet(as16);
  LMap lm11;
  lm11.addGO(1, offF3 - offE5);
  LMap lm12;
  lm12.addGO(1, offU1 - offE5);
  PWLMap mapE6l;
  mapE6l.addSetLM(domE6, lm11);
  PWLMap mapE6r;
  mapE6r.addSetLM(domE6, lm12);
  SetEdge E6("E6", 6, mapE6l, mapE6r, 0);

  Interval i17(1 + offE6, 1, offE7);
  MultiInterval mi17;
  mi17.addInter(i17);
  AtomSet as17(mi17);
  Set domE7;
  domE7.addAtomSet(as17);
  LMap lm13;
  lm13.addGO(1, offF3 - offE6);
  LMap lm14;
  lm14.addGO(1, offU2 - offE6);
  PWLMap mapE7l;
  mapE7l.addSetLM(domE7, lm13);
  PWLMap mapE7r;
  mapE7r.addSetLM(domE7, lm14);
  SetEdge E7("E7", 7, mapE7l, mapE7r, 0);

  Interval i18(1 + offE7, 1, offE8);
  MultiInterval mi18;
  mi18.addInter(i18);
  AtomSet as18(mi18);
  Set domE8;
  domE8.addAtomSet(as18);
  LMap lm15;
  lm15.addGO(1, offF4 - offE7);
  LMap lm16;
  lm16.addGO(1, offF6 - offE7);
  PWLMap mapE8l;
  mapE8l.addSetLM(domE8, lm15);
  PWLMap mapE8r;
  mapE8r.addSetLM(domE8, lm16);
  SetEdge E8("E8", 8, mapE8l, mapE8r, 0);

  Interval i19(1 + offE8, 1, offE9);
  MultiInterval mi19;
  mi19.addInter(i19);
  AtomSet as19(mi19);
  Set domE9;
  domE9.addAtomSet(as19);
  LMap lm17;
  lm17.addGO(1, offF4 - offE8);
  LMap lm18;
  lm18.addGO(1, offU3 - offE8);
  PWLMap mapE9l;
  mapE9l.addSetLM(domE9, lm17);
  PWLMap mapE9r;
  mapE9r.addSetLM(domE9, lm18);
  SetEdge E9("E9", 9, mapE9l, mapE9r, 0);

  Interval i20(1 + offE9, 1, offE10);
  MultiInterval mi20;
  mi20.addInter(i20);
  AtomSet as20(mi20);
  Set domE10;
  domE10.addAtomSet(as20);
  LMap lm19;
  lm19.addGO(1, offF5 - offE9);
  LMap lm20;
  lm20.addGO(1, offF6 - offE9 + n - 1);
  PWLMap mapE10l;
  mapE10l.addSetLM(domE10, lm19);
  PWLMap mapE10r;
  mapE10r.addSetLM(domE10, lm20);
  SetEdge E10("E10", 10, mapE10l, mapE10r, 0);

  Interval i21(1 + offE10, 1, offE11);
  MultiInterval mi21;
  mi21.addInter(i21);
  AtomSet as21(mi21);
  Set domE11;
  domE11.addAtomSet(as21);
  LMap lm21;
  lm21.addGO(1, offF5 - offE10);
  LMap lm22;
  lm22.addGO(1, offU3 - offE10 + n - 1);
  PWLMap mapE11l;
  mapE11l.addSetLM(domE11, lm21);
  PWLMap mapE11r;
  mapE11r.addSetLM(domE11, lm22);
  SetEdge E11("E11", 11, mapE11l, mapE11r, 0);

  SBGraph g;

  SetVertexDesc v1 = boost::add_vertex(g);
  SetVertexDesc v2 = boost::add_vertex(g);
  SetVertexDesc v3 = boost::add_vertex(g);
  SetVertexDesc v4 = boost::add_vertex(g);
  SetVertexDesc v5 = boost::add_vertex(g);
  SetVertexDesc v6 = boost::add_vertex(g);
  SetVertexDesc v7 = boost::add_vertex(g);
  SetVertexDesc v8 = boost::add_vertex(g);
  SetVertexDesc v9 = boost::add_vertex(g);
  SetVertexDesc v10 = boost::add_vertex(g);

  g[v1] = F1;
  g[v2] = F2;
  g[v3] = F3;
  g[v4] = F4;
  g[v5] = F5;
  g[v6] = F6;
  g[v7] = U1;
  g[v8] = U2;
  g[v9] = U3;
  g[v10] = U4;

  SetEdgeDesc e1;
  bool b1;
  boost::tie(e1, b1) = boost::add_edge(v1, v7, g);
  SetEdgeDesc e2;
  bool b2;
  boost::tie(e2, b2) = boost::add_edge(v2, v7, g);
  SetEdgeDesc e3;
  bool b3;
  boost::tie(e3, b3) = boost::add_edge(v2, v8, g);
  SetEdgeDesc e4;
  bool b4;
  boost::tie(e4, b4) = boost::add_edge(v3, v7, g);
  SetEdgeDesc e5;
  bool b5;
  boost::tie(e5, b5) = boost::add_edge(v3, v9, g);
  SetEdgeDesc e6;
  bool b6;
  boost::tie(e6, b6) = boost::add_edge(v4, v8, g);
  SetEdgeDesc e7;
  bool b7;
  boost::tie(e7, b7) = boost::add_edge(v4, v9, g);
  SetEdgeDesc e8;
  bool b8;
  boost::tie(e8, b8) = boost::add_edge(v5, v7, g);
  SetEdgeDesc e9;
  bool b9;
  boost::tie(e9, b9) = boost::add_edge(v5, v10, g);
  SetEdgeDesc e10;
  bool b10;
  boost::tie(e10, b10) = boost::add_edge(v6, v7, g);
  SetEdgeDesc e11;
  bool b11;
  boost::tie(e11, b11) = boost::add_edge(v6, v10, g);

  g[e1] = E1;
  g[e2] = E2;
  g[e3] = E3;
  g[e4] = E4;
  g[e5] = E5;
  g[e6] = E6;
  g[e7] = E7;
  g[e8] = E8;
  g[e9] = E9;
  g[e10] = E10;
  g[e11] = E11;

  MatchingStruct match(g);
  Interval i22(75, 3, 125);
  MultiInterval mi22;
  mi22.addInter(i22);
  AtomSet as22(mi22);
  Set s22;
  s22.addAtomSet(as22);
  match.matchedE = s22;

  UnordCT<Set> res = match.split(f2);

  BOOST_CHECK(true);
}

void TestMatching()
{
  float n = 100;

  // Equations

  float offF1 = 1;
  float offF2 = offF1 + n;
  float offF3 = offF2 + n;
  float offF4 = offF3 + n - 1;
  float offF5 = offF4 + n - 1;
  float offF6 = offF5 + 1;

  Interval i1(offF1, 1, offF1);
  MultiInterval mi1;
  mi1.addInter(i1);
  AtomSet as1(mi1);
  Set f1;
  f1.addAtomSet(as1);
  SetVertex F1("F1", 1, f1, 0);

  Interval i2(1 + offF1, 1, offF2);
  MultiInterval mi2;
  mi2.addInter(i2);
  AtomSet as2(mi2);
  Set f2;
  f2.addAtomSet(as2);
  SetVertex F2("F2", 2, f2, 0);

  Interval i3(1 + offF2, 1, offF3);
  MultiInterval mi3;
  mi3.addInter(i3);
  AtomSet as3(mi3);
  Set f3;
  f3.addAtomSet(as3);
  SetVertex F3("F3", 3, f3, 0);

  Interval i4(1 + offF3, 1, offF4);
  MultiInterval mi4;
  mi4.addInter(i4);
  AtomSet as4(mi4);
  Set f4;
  f4.addAtomSet(as4);
  SetVertex F4("F4", 4, f4, 0);

  Interval i5(1 + offF4, 1, offF5);
  MultiInterval mi5;
  mi5.addInter(i5);
  AtomSet as5(mi5);
  Set f5;
  f5.addAtomSet(as5);
  SetVertex F5("F5", 1, f5, 0);

  Interval i6(1 + offF5, 1, offF6);
  MultiInterval mi6;
  mi6.addInter(i6);
  AtomSet as6(mi6);
  Set f6;
  f6.addAtomSet(as6);
  SetVertex F6("F6", 6, f6, 0);

  // Unknowns

  float offU1 = offF6 + n;
  float offU2 = offU1 + n;
  float offU3 = offU2 + n;
  float offU4 = offU3 + n;

  Interval i7(1 + offF6, 1, offU1);
  MultiInterval mi7;
  mi7.addInter(i7);
  AtomSet as7(mi7);
  Set u1;
  u1.addAtomSet(as7);
  SetVertex U1("U1", 7, u1, 0);

  Interval i8(1 + offU1, 1, offU2);
  MultiInterval mi8;
  mi8.addInter(i8);
  AtomSet as8(mi8);
  Set u2;
  u2.addAtomSet(as8);
  SetVertex U2("U2", 8, u2, 0);

  Interval i9(1 + offU2, 1, offU3);
  MultiInterval mi9;
  mi9.addInter(i9);
  AtomSet as9(mi9);
  Set u3;
  u3.addAtomSet(as9);
  SetVertex U3("U3", 9, u3, 0);

  Interval i10(1 + offU3, 1, offU4);
  MultiInterval mi10;
  mi10.addInter(i10);
  AtomSet as10(mi10);
  Set u4;
  u4.addAtomSet(as10);
  SetVertex U4("U4", 10, u4, 0);

  // Edges

  float offE1 = 1;
  float offE2 = offE1 + n;
  float offE3 = offE2 + n;
  float offE4 = offE3 + n;
  float offE5 = offE4 + n;
  float offE6 = offE5 + n - 1;
  float offE7 = offE6 + n - 1;
  float offE8a = offE7 + n - 1;
  float offE8b = offE8a + n - 1;
  float offE9 = offE8b + n - 1;
  float offE10 = offE9 + 1;
  float offE11 = offE10 + 1;

  Interval i11(offE1, 1, offE1);
  MultiInterval mi11;
  mi11.addInter(i11);
  AtomSet as11(mi11);
  Set domE1;
  domE1.addAtomSet(as11);
  LMap lm1;
  lm1.addGO(1, offF1 - offE1);
  LMap lm2;
  lm2.addGO(1, 1 + offF6 - offE1);
  PWLMap mapE1l;
  mapE1l.addSetLM(domE1, lm1);
  PWLMap mapE1r;
  mapE1r.addSetLM(domE1, lm2);
  SetEdge E1("E1", 1, mapE1l, mapE1r, 0);

  Interval i12(1 + offE1, 1, offE2);
  MultiInterval mi12;
  mi12.addInter(i12);
  AtomSet as12(mi12);
  Set domE2;
  domE2.addAtomSet(as12);
  LMap lm3;
  lm3.addGO(1, offF1 - offE1);
  LMap lm4;
  lm4.addGO(1, offF6 - offE1);
  PWLMap mapE2l;
  mapE2l.addSetLM(domE2, lm3);
  PWLMap mapE2r;
  mapE2r.addSetLM(domE2, lm4);
  SetEdge E2("E2", 2, mapE2l, mapE2r, 0);

  Interval i13(1 + offE2, 1, offE3);
  MultiInterval mi13;
  mi13.addInter(i13);
  AtomSet as13(mi13);
  Set domE3;
  domE3.addAtomSet(as13);
  LMap lm5;
  lm5.addGO(1, offF1 - offE2);
  LMap lm6;
  lm6.addGO(1, offU1 - offE2);
  PWLMap mapE3l;
  mapE3l.addSetLM(domE3, lm5);
  PWLMap mapE3r;
  mapE3r.addSetLM(domE3, lm6);
  SetEdge E3("E3", 3, mapE3l, mapE3r, 0);

  Interval i14(1 + offE3, 1, offE4);
  MultiInterval mi14;
  mi14.addInter(i14);
  AtomSet as14(mi14);
  Set domE4;
  domE4.addAtomSet(as14);
  LMap lm7;
  lm7.addGO(1, offF2 - offE3);
  LMap lm8;
  lm8.addGO(1, offF6 - offE3);
  PWLMap mapE4l;
  mapE4l.addSetLM(domE4, lm7);
  PWLMap mapE4r;
  mapE4r.addSetLM(domE4, lm8);
  SetEdge E4("E4", 4, mapE4l, mapE4r, 0);

  Interval i15(1 + offE4, 1, offE5);
  MultiInterval mi15;
  mi15.addInter(i15);
  AtomSet as15(mi15);
  Set domE5;
  domE5.addAtomSet(as15);
  LMap lm9;
  lm9.addGO(1, offF2 - offE4);
  LMap lm10;
  lm10.addGO(1, offU2 - offE4);
  PWLMap mapE5l;
  mapE5l.addSetLM(domE5, lm9);
  PWLMap mapE5r;
  mapE5r.addSetLM(domE5, lm10);
  SetEdge E5("E5", 5, mapE5l, mapE5r, 0);

  Interval i16(1 + offE5, 1, offE6);
  MultiInterval mi16;
  mi16.addInter(i16);
  AtomSet as16(mi16);
  Set domE6;
  domE6.addAtomSet(as16);
  LMap lm11;
  lm11.addGO(1, offF3 - offE5);
  LMap lm12;
  lm12.addGO(1, offU1 - offE5 + 1);
  PWLMap mapE6l;
  mapE6l.addSetLM(domE6, lm11);
  PWLMap mapE6r;
  mapE6r.addSetLM(domE6, lm12);
  SetEdge E6("E6", 6, mapE6l, mapE6r, 0);

  Interval i17(1 + offE6, 1, offE7);
  MultiInterval mi17;
  mi17.addInter(i17);
  AtomSet as17(mi17);
  Set domE7;
  domE7.addAtomSet(as17);
  LMap lm13;
  lm13.addGO(1, offF3 - offE6);
  LMap lm14;
  lm14.addGO(1, offU2 - offE6 + 1);
  PWLMap mapE7l;
  mapE7l.addSetLM(domE7, lm13);
  PWLMap mapE7r;
  mapE7r.addSetLM(domE7, lm14);
  SetEdge E7("E7", 7, mapE7l, mapE7r, 0);

  Interval i18a(1 + offE7, 1, offE8a);
  MultiInterval mi18a;
  mi18a.addInter(i18a);
  AtomSet as18a(mi18a);
  Set domE8a;
  domE8a.addAtomSet(as18a);
  Interval i18b(1 + offE8a, 1, offE8b);
  MultiInterval mi18b;
  mi18b.addInter(i18b);
  AtomSet as18b(mi18b);
  Set domE8b;
  domE8b.addAtomSet(as18b);
  LMap lm15a;
  lm15a.addGO(1, offF4 - offE7);
  LMap lm15b;
  lm15b.addGO(1, offF4 - offE7 - n + 1);
  LMap lm16a;
  lm16a.addGO(1, offF6 - offE7);
  LMap lm16b;
  lm16b.addGO(1, offF6 - offE7 - n + 2);
  PWLMap mapE8l;
  mapE8l.addSetLM(domE8a, lm15a);
  mapE8l.addSetLM(domE8b, lm15b);
  PWLMap mapE8r;
  mapE8r.addSetLM(domE8a, lm16a);
  mapE8r.addSetLM(domE8b, lm16b);
  SetEdge E8("E8", 8, mapE8l, mapE8r, 0);

  Interval i19(1 + offE8b, 1, offE9);
  MultiInterval mi19;
  mi19.addInter(i19);
  AtomSet as19(mi19);
  Set domE9;
  domE9.addAtomSet(as19);
  LMap lm17;
  lm17.addGO(1, offF4 - offE8b);
  LMap lm18;
  lm18.addGO(1, offU3 - offE8b);
  PWLMap mapE9l;
  mapE9l.addSetLM(domE9, lm17);
  PWLMap mapE9r;
  mapE9r.addSetLM(domE9, lm18);
  SetEdge E9("E9", 9, mapE9l, mapE9r, 0);

  Interval i20(1 + offE9, 1, offE10);
  MultiInterval mi20;
  mi20.addInter(i20);
  AtomSet as20(mi20);
  Set domE10;
  domE10.addAtomSet(as20);
  LMap lm19;
  lm19.addGO(1, offF5 - offE9);
  LMap lm20;
  lm20.addGO(1, offF6 - offE9 + n - 1);
  PWLMap mapE10l;
  mapE10l.addSetLM(domE10, lm19);
  PWLMap mapE10r;
  mapE10r.addSetLM(domE10, lm20);
  SetEdge E10("E10", 10, mapE10l, mapE10r, 0);

  Interval i21(1 + offE10, 1, offE11);
  MultiInterval mi21;
  mi21.addInter(i21);
  AtomSet as21(mi21);
  Set domE11;
  domE11.addAtomSet(as21);
  LMap lm21;
  lm21.addGO(1, offF5 - offE10);
  LMap lm22;
  lm22.addGO(1, offU3 - offE10 + n - 1);
  PWLMap mapE11l;
  mapE11l.addSetLM(domE11, lm21);
  PWLMap mapE11r;
  mapE11r.addSetLM(domE11, lm22);
  SetEdge E11("E11", 11, mapE11l, mapE11r, 0);

  SBGraph g;

  SetVertexDesc v1 = boost::add_vertex(g);
  SetVertexDesc v2 = boost::add_vertex(g);
  SetVertexDesc v3 = boost::add_vertex(g);
  SetVertexDesc v4 = boost::add_vertex(g);
  SetVertexDesc v5 = boost::add_vertex(g);
  SetVertexDesc v6 = boost::add_vertex(g);
  SetVertexDesc v7 = boost::add_vertex(g);
  SetVertexDesc v8 = boost::add_vertex(g);
  SetVertexDesc v9 = boost::add_vertex(g);
  SetVertexDesc v10 = boost::add_vertex(g);

  g[v1] = F1;
  g[v2] = F2;
  g[v3] = F3;
  g[v4] = F4;
  g[v5] = F5;
  g[v6] = F6;
  g[v7] = U1;
  g[v8] = U2;
  g[v9] = U3;
  g[v10] = U4;

  SetEdgeDesc e1;
  bool b1;
  boost::tie(e1, b1) = boost::add_edge(v1, v7, g);
  SetEdgeDesc e2;
  bool b2;
  boost::tie(e2, b2) = boost::add_edge(v2, v7, g);
  SetEdgeDesc e3;
  bool b3;
  boost::tie(e3, b3) = boost::add_edge(v2, v8, g);
  SetEdgeDesc e4;
  bool b4;
  boost::tie(e4, b4) = boost::add_edge(v3, v7, g);
  SetEdgeDesc e5;
  bool b5;
  boost::tie(e5, b5) = boost::add_edge(v3, v9, g);
  SetEdgeDesc e6;
  bool b6;
  boost::tie(e6, b6) = boost::add_edge(v4, v8, g);
  SetEdgeDesc e7;
  bool b7;
  boost::tie(e7, b7) = boost::add_edge(v4, v9, g);
  SetEdgeDesc e8;
  bool b8;
  boost::tie(e8, b8) = boost::add_edge(v5, v7, g);
  SetEdgeDesc e9;
  bool b9;
  boost::tie(e9, b9) = boost::add_edge(v5, v10, g);
  SetEdgeDesc e10;
  bool b10;
  boost::tie(e10, b10) = boost::add_edge(v6, v7, g);
  SetEdgeDesc e11;
  bool b11;
  boost::tie(e11, b11) = boost::add_edge(v6, v10, g);

  g[e1] = E1;
  g[e2] = E2;
  g[e3] = E3;
  g[e4] = E4;
  g[e5] = E5;
  g[e6] = E6;
  g[e7] = E7;
  g[e8] = E8;
  g[e9] = E9;
  g[e10] = E10;
  g[e11] = E11;

  MatchingStruct match(g);
  Set res = match.SBGMatching();

  BOOST_CHECK(true);
}

//____________________________________________________________________________//

test_suite *init_unit_test_suite(int, char *[])
{
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
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestSetDiff2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestSetMin1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestSetMin2));

  framework::master_test_suite().add(BOOST_TEST_CASE(&TestLMCreation1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestLMCompose1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestLMCompose2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestInvLMap1));

  framework::master_test_suite().add(BOOST_TEST_CASE(&TestAtomPWCreation1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestAtomPWCreation2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestAtomPWCreation3));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestAtomPWImage1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestAtomPWImage2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestAtomPWImage3));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestAtomPWImage4));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestAtomPWPre1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestAtomPWPre2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestAtomPWPre3));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestAtomPWPre4));

  framework::master_test_suite().add(BOOST_TEST_CASE(&TestPWLMapCreation1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestPWLMapImage1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestPWLMapImage2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestPWLMapPre1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestPWLMapComp1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestPWLMapComp2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestPWLMapCombine1));

  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMinAS1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMinAS2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMinSet1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMinSet2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMinMap1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestReduce1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMinAdjComp1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMinAdjComp2));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMinAdjComp3));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMinAdjComp4));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMinAdj1));

  framework::master_test_suite().add(BOOST_TEST_CASE(&TestRC1));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestGraph3c));
  framework::master_test_suite().add(BOOST_TEST_CASE(&Test2D));

  framework::master_test_suite().add(BOOST_TEST_CASE(&TestSplit));
  framework::master_test_suite().add(BOOST_TEST_CASE(&TestMatching));

  return 0;
}

//____________________________________________________________________________//

// EOF
