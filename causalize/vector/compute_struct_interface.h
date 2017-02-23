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

#ifndef COMPUTE_STRUCT_INTERFACE
#define COMPUTE_STRUCT_INTERFACE

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif
typedef struct {
  int lower, upper;
} Interval;

typedef struct {
  int dimension;
  Interval *ranges;
} CMDI;

typedef struct {
  char *var;
  CMDI uk_mdi;
  int eqs;
  int *eq;
  CMDI *ranges;
} Block;

typedef struct {
  int blocks;
  Block *block;
} VarDependencies;

EXTERNC VarDependencies ComputeDependencies(const char *filename);
#endif
