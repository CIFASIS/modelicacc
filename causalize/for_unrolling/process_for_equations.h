#include <mmo/mmo_class.h>

/**
 * Performs a loop unrolling over the for-equations
 * decleared on the equation section of the class.
 */

namespace Causalize {
  void process_for_equations(Modelica::MMO_Class &mmo_class);
  Equation instantiate_equation(Equation, Name, Real, VarSymbolTable &);
}
