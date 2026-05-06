# Integration Changes for EquationSolver with SBG Implementation

## Required Changes to Make SBG Main File Compile with EquationSolver

### 1. Header Dependencies to Add

In `/home/joaquin/work/modelicacc/causalize/sbg_implementation/main.cpp`, add these includes:

```cpp
#include <util/solve/solve.hpp>           // For EquationSolver class
#include <ast/equation.hpp>               // For EquationList, ExpList types
#include <util/table.hpp>                  // For VarSymbolTable
#include <causalize/graph_implementation/apply_tarjan.h>  // For Tarjan algorithm
#include <causalize/graph_implementation/graph/graph_definition.h>  // For graph structures
```

### 2. Data Structures to Add

Add these member variables to handle EquationSolver requirements:

```cpp
// In main.cpp or create a wrapper class
std::list<std::string> c_code;           // For generated C code
Modelica::AST::ClassList _cl;            // For new function classes
```

### 3. JSON Processing Integration Point

After line 123 in `main.cpp` (after SBG binary call), add JSON processing and EquationSolver integration:

```cpp
// Add after line 123: std::cout << "Result: " << res << std::endl;

// TODO: Add your JSON file interpretation here
// std::string json_content = read_json_file(CAUSALIZED_JSON);

// Convert JSON results to EquationList and ExpList
// Modelica::AST::EquationList causalized_eqs = parse_json_to_equations(json_content);
// Modelica::AST::ExpList unknowns = parse_json_to_unknowns(json_content);

// Apply EquationSolver to generate C code
// std::stringstream output_path;
// output_path << mmo_class.name() << ".c";
// EquationList final_eqs = EquationSolver::Solve(
//     causalized_eqs, 
//     unknowns, 
//     mmo_class.syms_ref(), 
//     c_code, 
//     _cl, 
//     output_path.str()
// );
```

### 4. Compilation Dependencies

Ensure the following libraries are linked in the Makefile:

```makefile
# Add to Makefile.include in sbg_implementation directory
LIBS += -lutil_solve -last -lmmo
```

### 5. Missing Function Implementations

You'll need to implement these functions for JSON processing:

```cpp
// Add these functions to parse SBG JSON output
Modelica::AST::EquationList parse_json_to_equations(const std::string& json_content);
Modelica::AST::ExpList parse_json_to_unknowns(const std::string& json_content);
std::string read_json_file(const std::string& filename);
```

### 6. Required Type Conversions

The EquationSolver expects these types from the SBG JSON output:

- `EquationList`: List of Modelica equations
- `ExpList`: List of variable expressions  
- `VarSymbolTable&`: Symbol table from MMO_Class
- `std::list<std::string>&`: Container for generated C code
- `ClassList&`: Container for new function classes

### 7. Error Handling

Add error checking for:

```cpp
// Check if SBG processing succeeded
if (res != 0) {
    std::cerr << "SBG processing failed with code: " << res << std::endl;
    return res;
}

// Validate JSON file exists
std::ifstream json_file(CAUSALIZED_JSON);
if (!json_file.good()) {
    std::cerr << "Failed to open SBG output file: " << CAUSALIZED_JSON << std::endl;
    return -1;
}
```

### 8. Build System Updates

Update the build system to include the new dependencies:

1. Add `util/solve` to the include path
2. Link against the solve library
3. Ensure Boost Graph Library is available (for Tarjan algorithm)

### 9. Alternative: Create Wrapper Class

Instead of modifying main.cpp directly, consider creating a wrapper class:

```cpp
class SBGToEquationSolver {
public:
    SBGToEquationSolver(Modelica::MMO_Class& mmo_class);
    int processSBGOutput(const std::string& json_file);
    
private:
    Modelica::MMO_Class& _mmo_class;
    std::list<std::string> _c_code;
    Modelica::AST::ClassList _cl;
    
    Modelica::AST::EquationList parseJSONToEquations(const std::string& json);
    Modelica::AST::ExpList parseJSONToUnknowns(const std::string& json);
    void generateCCode(const Modelica::AST::EquationList& eqs, const Modelica::AST::ExpList& unknowns);
};
```

## Summary of Required Changes

1. **Add 5 new header includes** for EquationSolver and dependencies
2. **Add 2 member variables** for C code and class list storage  
3. **Implement 3 JSON parsing functions** to convert SBG output to Modelica types
4. **Add error handling** for SBG processing and file I/O
5. **Update build system** to link required libraries
6. **Integrate EquationSolver call** after JSON processing

The main integration point is after the SBG binary call (line 123), where you'll parse the JSON output and feed it to EquationSolver to generate the final C code.
