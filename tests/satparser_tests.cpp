#include <sstream>
#include <iostream>
#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>

#include "parsers/sat.h"

#include "satparser_tests.h"

void SatParserTests::testBasic() {
    std::istringstream stream("p cnf 5 3\n3 0\n1 2 4 0\n5 1 0\n");
    std::istringstream &stream2 = stream;
    std::vector<std::shared_ptr<satsolver::Clause>> clauses;
    std::set<std::set<int>> clauses_set;
    std::shared_ptr<satsolver::Formula> formula;
    satsolver::SatParser *parser;
    parser = new satsolver::SatParser(stream2);
    CPPUNIT_ASSERT_NO_THROW(parser->parse());
    clauses = parser->get_clauses();
    CPPUNIT_ASSERT(clauses[0]->to_set() == std::set<int>({3}));
    CPPUNIT_ASSERT(clauses[1]->to_set() == std::set<int>({1, 2, 4}));
    CPPUNIT_ASSERT(clauses[2]->to_set() == std::set<int>({1, 5}));
    CPPUNIT_ASSERT(clauses[0]->to_set() == std::set<int>({3}));
    formula = parser->get_formula();
    //clauses_set.insert(clauses[0]->to_set()); -> deleted by pretreatment in formula instanciation
    clauses_set.insert(clauses[1]->to_set());
    clauses_set.insert(clauses[2]->to_set());
    CPPUNIT_ASSERT(formula->to_set() == clauses_set);
    std::set<int> *set = formula->get_aff()->to_set();
    CPPUNIT_ASSERT(*set == std::set<int>({3}));
    delete parser;
    delete set;
}
void SatParserTests::testExtraWhitespaces() {
    std::istringstream stream("c 0 foo bar\np cnf 5 3\n3 0\n\n\n1 2      4 0\nc oof\n5 1 0\n");
    std::istringstream &stream2 = stream;
    std::vector<std::shared_ptr<satsolver::Clause>> clauses;
    satsolver::SatParser *parser;
    parser = new satsolver::SatParser(stream2);
    CPPUNIT_ASSERT_NO_THROW(parser->parse());
    clauses = parser->get_clauses();
    CPPUNIT_ASSERT(clauses[0]->to_set() == std::set<int>({3}));
    CPPUNIT_ASSERT(clauses[1]->to_set() == std::set<int>({1, 2, 4}));
    CPPUNIT_ASSERT(clauses[2]->to_set() == std::set<int>({1, 5}));
    delete parser;
}


CppUnit::Test* SatParserTests::suite() {
    CppUnit::TestSuite *suite = new CppUnit::TestSuite("SatParserTests");
    suite->addTest(new CppUnit::TestCaller<SatParserTests>("SatParserTest_testBasic",
                &SatParserTests::testBasic));
    suite->addTest(new CppUnit::TestCaller<SatParserTests>("SatParserTest_testExtraWhitespaces",
                &SatParserTests::testExtraWhitespaces));
    return suite;
}
