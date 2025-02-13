## ModelicaCC

ModelicaCC is a Modelica C Compiler implemented in C++, the main goal of this project is to provide an environment to develop and test novel algorithms involved in the different compilation stages of large scale Modelica models. The different stages of the compilation pipeline follows the usual order: 

  * Parsing
  * Flattening
  * Index Reduction
  * Sorting
  * Code Generation
  * Model Simulation

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

[3] Denise Marzorati, Joaquín Fernández, and Ernesto Kofman. 2024. Efficient Matching in Large DAE Models. ACM Trans. Math. Softw. Just Accepted (June 2024). https://doi.org/10.1145/3674831

[4] Denise Marzorati, Joaquin Fernández, Ernesto Kofman. Connected Components in Undirected Set--Based Graphs. Applications in Object--Oriented Model Manipulation Applied Mathematics and Computation, Volume 418, 2022, 126842,ISSN 0096-3003, https://doi.org/10.1016/j.amc.2021.126842.

[5] Ernesto Kofman, Joaquín Fernández, Denise Marzorati. Compact sparse symbolic Jacobian computation in large systems of ODEs Applied Mathematics and Computation, Volume 403, 2021, 126181, ISSN 0096-3003, https://doi.org/10.1016/j.amc.2021.126181.

## Installation

These are generic installation instructions.

## Dependences

In order to be able to install and compile ModelicaCC, 
the following  dependencies must be installed: 

    * autoconf 2.69 (avoid 2.71)
    * boost1.81
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

## SBG library instructions

To use the newest versions of SBG algorithms type `make update-sbg`, and
then re-compile binaries by typing `make`. This is done automatically the
first time the whole project is built.

If Python3 is not installed, replace the occurrences of `python3` with
`python`.

## Changelog

## [4.0] - 2022-05-04
### Added
- Implemented SBGraphs as an independent library. As such, new algorithms
for causalization will be listed in its corresponding repository.

## [3.0] - 2021-08-20
### Added
- Implemented SBGraphs data structures and helper classes.
- Implemented **new flatter** algorithm based on **SBGraphs**.

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
