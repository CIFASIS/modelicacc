#include <mmo/mmo_class.h>
#include <causalize/state_variables_finder.h>

using namespace std;
using namespace Modelica;
using namespace Modelica::AST;

class UnknownsCollector {
	public:
		UnknownsCollector(MMO_Class &c);
		ExpList collectUnknowns();
	private:
		int getCompRefVal(Reference compRef, VarSymbolTable &symbolTable);
		MMO_Class &_c;
		StateVariablesFinder _finder;
};
