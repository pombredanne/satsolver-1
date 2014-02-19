#include <cppunit/ui/text/TestRunner.h>
#include "structures_tests.h"
#include "parser_tests.h"
#include "dpll_tests.h"

int main(void)
{
    CppUnit::TextUi::TestRunner runner;
    runner.addTest(StructuresTests::suite());
    runner.addTest(ParserTests::suite());
    runner.addTest(DpllTests::suite());
    runner.run();
    return 0;
}
