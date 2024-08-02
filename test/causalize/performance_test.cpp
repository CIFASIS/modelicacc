#include <causalize/graph_implementation/unknowns_collector.hpp>
#include <causalize/graph_implementation/causalization_strategy.hpp>

#include <parser/parser.hpp>
#include <ast/ast_types.hpp>
#include <ast/equation.hpp>
#include <mmo/mmo_class.hpp>
#include <util/table.hpp>
#include <util/debug.hpp>
#include <util/ast_visitors/contains.hpp>

#include <boost/variant/get.hpp>

#include <sys/time.hpp>
#include <stdio.hpp>

using namespace Modelica::AST;

void test(std::string filename)
{
  cout << "Testing file " << filename << "\t";

  bool r;
  struct timeval tval_before, tval_after, tval_result;

  // debugInit("p");

  StoredDef sd = parseFile(filename, r);

  if (!r) ERROR("Can't parse file\n");

  Class ast_c = boost::get<Class>(sd.classes().front());
  MMO_Class mmo(ast_c);

  UnknownsCollector collector(mmo);
  ExpList unkowns = collector.collectUnknowns();

  CausalizationStrategy cStrategy(mmo);

  gettimeofday(&tval_before, NULL);

  cStrategy.causalize_simple("anything");

  gettimeofday(&tval_after, NULL);

  timersub(&tval_after, &tval_before, &tval_result);

  printf("Simple strategy: %ld.%06ld\t", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);

  StoredDef sd2 = parseFile(filename, r);

  if (!r) ERROR("Can't parse file\n");

  Class ast_c2 = boost::get<Class>(sd2.classes().front());
  MMO_Class mmo2(ast_c2);

  CausalizationStrategy cStrategy2(mmo2);

  gettimeofday(&tval_before, NULL);

  cStrategy2.causalize_tarjan("anything");

  gettimeofday(&tval_after, NULL);

  timersub(&tval_after, &tval_before, &tval_result);

  printf("Tarjan strategy: %ld.%06ld\t", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);

  StoredDef sd3 = parseFile(filename, r);

  if (!r) ERROR("Can't parse file\n");

  Class ast_c3 = boost::get<Class>(sd3.classes().front());
  MMO_Class mmo3(ast_c3);

  CausalizationStrategy cStrategy3(mmo3);

  gettimeofday(&tval_before, NULL);

  cStrategy3.causalize("anything");

  gettimeofday(&tval_after, NULL);

  timersub(&tval_after, &tval_before, &tval_result);

  printf("Full Causalization strategy: %ld.%06ld\n", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);
}

int main(int argc, char const *argv[])
{
  char const *char_filename;
  if (argv[1] != NULL) {
    char_filename = argv[1];
  } else {
    printf("Usage:\n\tperformance_test model.mo\n");
    exit(1);
  }
  std::string filename(char_filename);
  test(filename);
  return 0;
}
