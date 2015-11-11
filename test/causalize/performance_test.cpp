#include <causalize/unknowns_collector.h>
#include <causalize/causalization_strategy.h>

#include <parser/parser.h>
#include <ast/ast_types.h>
#include <ast/equation.h>
#include <mmo/mmo_class.h>
#include <util/table.h>
#include <util/debug.h>
#include <util/ast_visitors/contains.h>

#include <boost/variant/get.hpp>

#include <ctime>

using namespace Modelica::AST;

int main(int argc, char const *argv[])
{
	bool r;

	StoredDef sd = parseFile("OneDHeatTransferTI_FD.mo",r);

	if (!r)
	ERROR("Can't parse file\n");

	Class ast_c = boost::get<Class>(sd.classes().front());
	MMO_Class mmo(ast_c);

	UnknownsCollector collector(mmo);
	ExpList unkowns = collector.collectUnknowns();

	CausalizationStrategy cStrategy(mmo);

	time_t causalize_t0  = time(NULL);

	cStrategy.causalize("anything");

	time_t causalize_t1  = time(NULL);

	std::cout << difftime(causalize_t1, causalize_t0) << std::endl;

	MMO_Class mmo2(ast_c);

	CausalizationStrategy cStrategy2(mmo2);

	time_t causalize_no_opt_t0  = time(NULL);

	cStrategy2.causalize_no_opt("anything");

	time_t causalize_no_opt_t1  = time(NULL);

	std::cout << difftime(causalize_no_opt_t1, causalize_no_opt_t0) << std::endl;

	return 0;
}