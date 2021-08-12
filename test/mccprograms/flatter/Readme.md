**Flatter Algorithm test models**

This folder contains three test Modelica models where the flatter algorithm that makes use of the connected components algorithm for SB Graphs implemented [here](https://github.com/CIFASIS/modelicacc/blob/06788f123c8620b1e05da8a0b1c0153f6c1c3db3/util/graph/sbg/sbg_algorithms.cpp#L17) is applied. 
It also contains the expected results for each of them and a Modelica package that can be directly opened with **OMEdit** that contains all the models and clases needed to compile the models (`OM_flatter_package.mo`).

The models are the following: 

* A simple RC network `RLC.mo` -> Expected results `mcc_RLC.mo`
* An RC network with recursive connections `RRLC.mo` -> Expected results `mcc_RRLC.mo`
* A two-dimensional RC network `N2D.mo`-> Expected results `mcc_N2D.mo`


In order to run the flatter algorithm the following steps must be taken:

* Install the ModelicaCC compiler as described [here](https://github.com/CIFASIS/modelicacc#installation)
* From the ModelicaCC root folder run: `./bin/flatter -d ./test/mccprograms/flatter/{TEST_MODEL} > {OUTPUT_MODEL}` where `{TEST_MODEL}` is one of the models defined above and `{OUTPUT_MODEL}` is the result output Modelica model.

The obtained Modelica model can be directly compiled and simulated using [OpenModelica](https://www.openmodelica.org/#). 

**Note:* 
The -d flag will generate a log file located in the same folder where the command is called, this file contains detailed information about the differente steps taken by the flatter algorithm
and also information about the generated graph used by the algorithm. Additionally, a dot file with the definition of the graph is created, to generate a pdf file from the dot file, run:
`dot -Grankdir=LR -Tpdf <PATH_TO_DOT_FILE>`
