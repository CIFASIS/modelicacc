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

#include <sys/time.h>
#include <stdio.h>

using namespace Modelica::AST;

void test(std::string filename){

	cout << "Testing file " << filename << "\t";

	bool r;
	struct timeval tval_before, tval_after, tval_result;

	debugInit("p");

	StoredDef sd = parseFile(filename,r);

	if (!r)
	ERROR("Can't parse file\n");

	Class ast_c = boost::get<Class>(sd.classes().front());
	MMO_Class mmo(ast_c);

	UnknownsCollector collector(mmo);
	ExpList unkowns = collector.collectUnknowns();

	CausalizationStrategy cStrategy(mmo);

	gettimeofday(&tval_before, NULL);

	cStrategy.causalize_simple("anything");

	gettimeofday(&tval_after, NULL);

	timersub(&tval_after, &tval_before, &tval_result);

	printf("Simple strategy: Time elapsed: %ld.%06ld\t", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);

	StoredDef sd2 = parseFile(filename,r);

	if (!r)
	ERROR("Can't parse file\n");

	Class ast_c2 = boost::get<Class>(sd2.classes().front());
	MMO_Class mmo2(ast_c2);

	CausalizationStrategy cStrategy2(mmo2);

	gettimeofday(&tval_before, NULL);

	cStrategy2.causalize_tarjan("anything");

	gettimeofday(&tval_after, NULL);

	timersub(&tval_after, &tval_before, &tval_result);

	printf("Tarjan strategy: Time elapsed: %ld.%06ld\t", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);

	StoredDef sd3 = parseFile(filename,r);

	if (!r)
	ERROR("Can't parse file\n");

	Class ast_c3 = boost::get<Class>(sd3.classes().front());
	MMO_Class mmo3(ast_c3);

	CausalizationStrategy cStrategy3(mmo3);

	gettimeofday(&tval_before, NULL);

	cStrategy3.causalize("anything");

	gettimeofday(&tval_after, NULL);

	timersub(&tval_after, &tval_before, &tval_result);

	printf("Full Causalization strategy: Time elapsed: %ld.%06ld\n", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);
}

int main(int argc, char const *argv[])
{
	test("OneDHeatTransferTI_FD.mo");
	test("OneDHeatTransferTI_FD_loop.mo");
	return 0;
}
