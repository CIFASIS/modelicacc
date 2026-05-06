# Causalization Strategy Documentation

## Overview

The `causalization_strategy.cpp` file implements the core causalization algorithm for the Modelica C Compiler. This component transforms a system of simultaneous equations into a causally executable form by determining the order in which equations should be solved.

## Class Structure

### CausalizationStrategy Class

**Purpose**: Main orchestrator for the causalization process using graph-based algorithms.

**Key Components**:
- `_graph`: Bipartite graph connecting equations to unknown variables
- `_mmo_class`: Reference to the Modelica class being processed
- `_causalEqsBegining/Middle/End`: Storage for causally ordered equations

## Constructor Analysis (Lines 39-123)

The constructor performs initialization and graph building:

1. **For Equation Processing**: Processes Modelica `for` loops by unrolling them
2. **Unknown Collection**: Uses `UnknownsCollector` to identify all variables
3. **Balance Validation**: Ensures equation count matches unknown count
4. **Graph Construction**: Creates bipartite graph with:
   - **Equation vertices**: Each equation becomes a vertex
   - **Unknown vertices**: Each variable becomes a vertex  
   - **Edges**: Connect equations to variables they contain

**Key Methods**:
- `PartialEvalExpression`: Evaluates constant expressions
- `ContainsExpression`: Checks variable occurrence in equations

## Main Causalization Methods

### Causalize() (Lines 125-161)

**Strategy**: Hybrid approach combining simple and advanced algorithms

**Process**:
1. Execute `SimpleCausalizationStrategy()` for straightforward cases
2. If graph remains, apply `MakeCausalMiddle()` using Tarjan's algorithm
3. Merge results from all phases
4. Generate C code output file

### SimpleCausalizationStrategy() (Lines 216-263)

**Algorithm**: Iterative removal of degree-1 vertices

**Process**:
1. **Identify degree-1 vertices**: Equations/variables with single connections
2. **Process equations**: When equation has one variable, solve for that variable
3. **Process variables**: When variable appears in one equation, solve equation for variable
4. **Update graph**: Remove processed vertices and update degrees

**Key Functions**:
- `GetUniqueEdge()`: Retrieves the single edge for degree-1 vertices
- `CollectDegree1Verts()`: Updates degree-1 vertex list after graph modifications

### MakeCausalMiddle() (Lines 327-359)

**Algorithm**: Tarjan's strongly connected components for complex systems

**Process**:
1. **Component Detection**: Uses Tarjan algorithm to find strongly connected components
2. **Component Processing**: Each component becomes a system of simultaneous equations
3. **Numerical Solution**: Generates code for numerical solvers (GSL multi-root finding)

## Helper Methods

### MakeCausalBegining() (Lines 290-305)
- Solves equations where unknown appears on left side
- Adds to beginning of causal equation list

### MakeCausalEnd() (Lines 307-322)  
- Solves equations where unknown appears on right side
- Adds to end of causal equation list

## Key Data Structures

**VertexProperty**: Contains either equation or unknown information
**EdgeProperty**: Represents connections between equations and variables
**CausalizationGraph`: Boost Graph Library implementation

## Algorithm Flow

1. **Initialization**: Build bipartite graph from Modelica equations
2. **Simple Phase**: Remove all solvable degree-1 cases
3. **Complex Phase**: Apply Tarjan to remaining strongly connected components
4. **Code Generation**: Output C code with GSL solver integration

## Integration Points

- **EquationSolver**: Handles symbolic and numerical solution of equations
- **UnknownsCollector**: Identifies all variables in the system
- **GraphPrinter**: Debugging visualization of causalization graphs

This implementation efficiently handles both simple algebraic systems and complex differential-algebraic equations by combining graph-theoretic algorithms with symbolic manipulation techniques.
