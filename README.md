## ModelicaCC

ModelicaCC is a Modelica C Compiler implemented in C++, the main goal of this project is to provide an environment to develop and test novel algorithms involved in the different compilation stages of large scale Modelica models. The different stages of the compilation pipeline follows the usual order: 

  * Parsing
  * Flattening
  * Index Reduction
  * Sorting
  * Code Generation
  * Simulation Model

The input/output of each stage are valid Modelica models (except for the very last stage which produce C code). Each stage converts the Modelica model fed as input into a “simpler” equivalent one. As the compiler uses the [QSS Solver](https://github.com/CIFASIS/qss-solver) for the C code generation and simulation, the goal of the previous stages is to obtain a valid μ–Modelica model.

The following tools are generated:  

  * parser
  * antialias
  * flatter
  * causalize
  * mmo

## Related Publications

[1] Federico Bergero, Mariano Botta, Esteban Campostrini, Ernesto Kofman.
**Efficient Compilation of Large Scale Dynamical Systems**
Proceedings of the 11th International Modelica Conference 2015

[2] Pablo Zimmermann, Joaquin Fernandez, Ernesto Kofman.
**Set-based graph methods for fast equation sorting in large DAE systems**
 EOOLT '19: Proceedings of the 9th International Workshop on Equation-based Object-oriented Modeling Languages and Tools 2019

## Installation

These are generic installation instructions.

## Dependences

In order to be able to install and compile ModelicaCC, 
the following  dependencies must be installed: 

    * autoconf 2.69 (avoid 2.71)
    * boost1.71
    * cmake
    * doxygen
    * g++
    * libginac-dev 
    * make     

## Basic Installation

The simplest way to compile this package is:

  1. `cd` to the directory containing the package's source code and type
     `autoconf` to generate the configuration scripts.
  
  2. Type `./configure` to run the configuration script.
  
  3. Type `make` to compile all the binaries. The different tools are located 
     in the bin folder. 

  4. You can remove the program binaries and object files from the
     source code directory by typing `make clean`.  


## Changelog

## [2.0] - 2020-11-17
### Added
- Implemented vector graph [**data structures**](https://github.com/CIFASIS/modelicacc/blob/modelicacc-dev/causalize/vg_implementation/vector/vector_graph_definition.h)
- Implemented matching algorithm using vector graph data structures.
- Implemented Tarjan algorithm using vector graph data structures.

## [1.0] - 2020-11-04
### Added
- Implemented first version of flattening algorithm
- Implemented Tarjan algorithm
- Implemented causalization algorithm

## Licensing

Please see the file called LICENSE.

## Bug Reporting

Report bugs to: fernandez@cifasis-conicet.gov.ar
