#ifndef PROTONTESTER_H
#define PROTONTESTER_H

#include "ProtonTester-internal.h"

namespace ProtonTester
{
	void runAllTests();

	std::string GetCurrentlyRunningTestName();

	unsigned int GetTotalRun();

	unsigned int GetNumPassed();

	unsigned int GetNumFailed();
}

#define CheckEq(expected, actual) CmpEq(expected, #expected, actual, #actual, RT_CURRENT_CHECK_LOCATION)

#define CheckTrue(boolValue) CmpEq(true, "true", boolValue, #boolValue, RT_CURRENT_CHECK_LOCATION)

#define INIT_TEST() RT_TEST_HARNESS_FUNC(Init)

#define CLEAN_TEST() RT_TEST_HARNESS_FUNC(Clean)

#define TEST(testName) class testName : public TestCase { \
public: \
testName() : TestCase(#testName) {} \
virtual ~testName() {} \
virtual void runTest(); \
}; \
testName g_##testName; \
void testName::runTest()


#endif // PROTONTESTER_H
