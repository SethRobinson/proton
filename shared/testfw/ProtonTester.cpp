#include "ProtonTester.h"

#include <list>

#include "Entity/EntityUtils.h"

// Template specialization for a boolean type
template<>
std::string CheckResult::valueToString(const bool& value)
{
	return value ? "true" : "false";
}

std::string CheckResult::GetTestName() const
{
	return m_testName;
}

CheckResult::Result CheckResult::GetResult() const
{
	return m_result;
}

std::string CheckResult::GetResultString() const
{
	return m_resultStr;
}

void CheckResult::constructResultStr(const CheckLocation& testLocation, Result result, const std::string& expectedStr, const std::string& expected, const std::string& actualStr, const std::string& actual)
{
	m_resultStr = testLocation.testName + " (" + testLocation.fileName + ":" + toString(testLocation.lineNumber) + "):\n";

	switch (result)
	{
	case PASS:
		m_resultStr += "  " + actualStr + "\n  is\n  " + expectedStr;
		break;

	case FAIL:
		m_resultStr += "  Expected:    " + actualStr + "\n";
		m_resultStr += "  to be:       " + expectedStr + "\n";
		m_resultStr += "  which is:    " + expected + "\n";
		m_resultStr += "  but it was:  " + actual;
		break;
	}
}

TestResults gTestResults;

TestResults::TestResults()
{
	clear();
}

void TestResults::clear()
{
	m_passedTests.clear();
	m_failedTests.clear();

	m_resultStr.clear();
}

void TestResults::add(const CheckResult &result)
{
	if (result.GetResult() == CheckResult::FAIL)
	{
		gTestResults.m_passedTests.erase(result.GetTestName());
		gTestResults.m_failedTests.insert(result.GetTestName());

		if (!gTestResults.m_resultStr.empty()) {
			gTestResults.m_resultStr.append("\n\n");
		}

		gTestResults.m_resultStr.append(result.GetResultString());
	} else
	{
		if (gTestResults.m_failedTests.count(result.GetTestName()) == 0) {
			gTestResults.m_passedTests.insert(result.GetTestName());
		}
	}
}

unsigned int TestResults::GetTotalRun() const
{
	return m_passedTests.size() + m_failedTests.size();
}

unsigned int TestResults::GetNumPassed() const
{
	return m_passedTests.size();
}

unsigned int TestResults::GetNumFailed() const
{
	return m_failedTests.size();
}

std::string TestResults::GetResultString() const
{
	return m_resultStr + "\n\n" + toString(GetTotalRun()) + " tests run. " + toString(GetNumFailed()) + " failed, " + toString(GetNumPassed()) + " passed.";
}


class TestRunner
{
public:
	static TestRunner& get()
	{
		static TestRunner* instance = NULL;
		if (instance == NULL)
		{
			instance = new TestRunner;
		}
		return *instance;
	}

	void registerInitFunction(Function *func)
	{
		m_initFunctions.push_back(func);
	}

	void registerCleanFunction(Function *func)
	{
		m_cleanFunctions.push_back(func);
	}

	void addTestCase(TestCase *testCase)
	{
		m_testCases.push_back(testCase);
	}

	void runAllTests()
	{
		gTestResults.clear();

		for (TestCases::iterator it(m_testCases.begin()); it != m_testCases.end(); it++)
		{
			m_currentlyRunningTest = (*it);

			for (Functions::iterator initIt(m_initFunctions.begin()); initIt != m_initFunctions.end(); initIt++)
			{
				(*initIt)->execute();
			}

			(*it)->runTest();

			for (Functions::iterator cleanIt(m_cleanFunctions.begin()); cleanIt != m_cleanFunctions.end(); cleanIt++)
			{
				(*cleanIt)->execute();
			}
		}
		m_currentlyRunningTest = NULL;
	}

	std::string getCurrentlyRunningTestName() const
	{
		if (m_currentlyRunningTest != NULL)
		{
			return m_currentlyRunningTest->getTestName();
		} else
		{
			return string();
		}
	}

private:
	TestRunner() :
	    m_currentlyRunningTest(NULL)
	{
	}

	typedef std::list<Function*> Functions;

	Functions m_initFunctions;
	Functions m_cleanFunctions;

	typedef std::list<TestCase*> TestCases;
	TestCases m_testCases;

	TestCase *m_currentlyRunningTest;
};



TestCase::TestCase(const std::string& testName) :
    m_testName(testName)
{
	TestRunner::get().addTestCase(this);
}

std::string TestCase::getTestName() const
{
	return m_testName;
}


namespace ProtonTester
{

void RegisterInitFunction(Function *func)
{
	TestRunner::get().registerInitFunction(func);
}

void RegisterCleanFunction(Function *func)
{
	TestRunner::get().registerCleanFunction(func);
}

void runAllTests()
{
	TestRunner::get().runAllTests();
	LogMsg("%s", gTestResults.GetResultString().c_str());
}

std::string GetCurrentlyRunningTestName()
{
	return TestRunner::get().getCurrentlyRunningTestName();
}

unsigned int GetTotalRun()
{
	return gTestResults.GetTotalRun();
}

unsigned int GetNumPassed()
{
	return gTestResults.GetNumPassed();
}

unsigned int GetNumFailed()
{
	return gTestResults.GetNumFailed();
}

}
