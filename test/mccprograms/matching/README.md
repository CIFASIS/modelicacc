**Matching Algorithm test models**

This folder contains four test Modelica models, where the program constructs the corresponding SB-Graph, and then applies the matching algorithm for SB-Graphs implemented [here](https://github.com/CIFASIS/sb-graph/blob/739ba2969552fa3373137df29dcb045c397e3e93/sbg/sbg_algorithms.cpp#L671). It also contains the expected results for each of them. 

The models are the following: 

* A simple model with small algebraic loops disconnected from each other `Test1.mo` -> Expected results `results1.txt`
* A model with connections between different indices `Test2.mo` -> Expected results `results2.txt`
* Similar to Test2, but with border conditions changed `Test3.mo` -> Expected results `results3.txt`
* Two copies of Test3 `Test4.mo` -> Expected results `results4.txt`

In order to run the matching algorithm the following steps must be taken:

* Install the ModelicaCC compiler as described [here](https://github.com/CIFASIS/modelicacc#installation)
* From the ModelicaCC root folder run: `./bin/causalize ./test/mccprograms/matching/{TEST_MODEL} > {OUTPUT_MODEL}` where `{TEST_MODEL}` is one of the models defined above and `{OUTPUT_MODEL}` is the result output Modelica model.

**Note:** 
The execution will generate a SBG.log file located in the same folder where the command is called, this file contains detailed information about the different steps taken by the matching algorithm
and also information about the generated graph used by the algorithm. Additionally, a dot file with the definition of the graph is created, to generate a pdf file from the dot file, run:

`dot -Grankdir=LR -Tpdf <PATH_TO_DOT_FILE>`
