#ifndef PROTONTESTER_INTERNAL_H
#define PROTONTESTER_INTERNAL_H

#include "util/MiscUtils.h"
#include <set>

struct CheckLocation {
	std::string testName;
	std::string fileName;
	int lineNumber;

	CheckLocation(const std::string& testName, const std::string& fileName, int lineNumber) :
	    testName(testName),
	    fileName(fileName),
	    lineNumber(lineNumber)
	{
	}
};

class CheckResult
{
public:
	enum Result {
		PASS,
		FAIL
	};

	template<typename T>
	CheckResult(const CheckLocation& testLocation, Result result, const std::string& expectedStr, const T& expected, const std::string& actualStr, const T& actual) :
	    m_testName(testLocation.testName),
	    m_result(result)
	{
		constructResultStr(testLocation, result, expectedStr, valueToString(expected), actualStr, valueToString(actual));
	}

	std::string GetTestName() const;

	Result GetResult() const;

	std::string GetResultString() const;

private:
	template<typename T>
	static std::string valueToString(const T& value)
	{
		return toString(value);
	}

	void constructResultStr(const CheckLocation& testLocation, Result result, const std::string& expectedStr, const std::string& expected, const std::string& actualStr, const std::string& actual);

	std::string m_testName;
	Result m_result;
	std::string m_resultStr;
};

template<>
std::string CheckResult::valueToString(const bool& value);

class TestResults
{
public:
	TestResults();

	void clear();

	static void add(const CheckResult &result);

	unsigned int GetTotalRun() const;

	unsigned int GetNumPassed() const;

	unsigned int GetNumFailed() const;

	std::string GetResultString() const;

private:
	std::set<std::string> m_passedTests;
	std::set<std::string> m_failedTests;

	std::string m_resultStr;

};

class Function
{
public:
	virtual ~Function() {}
	virtual void execute() = 0;
};

class TestCase
{
public:
	TestCase(const std::string& testName);

	virtual ~TestCase() {}
	virtual void runTest() = 0;

	std::string getTestName() const;
protected:

	std::string m_testName;
};


template<typename T>
void CmpEq(const T& expected, const std::string& expectedStr, const T& actual, const std::string& actualStr, const CheckLocation& testLocation)
{
	CheckResult::Result result = CheckResult::FAIL;

	if (expected == actual)
	{
		result = CheckResult::PASS;
	}

	TestResults::add(CheckResult(testLocation, result, expectedStr, expected, actualStr, actual));
}

namespace ProtonTester
{

void RegisterInitFunction(Function *func);

void RegisterCleanFunction(Function *func);

}

#define RT_CURRENT_CHECK_LOCATION CheckLocation(ProtonTester::GetCurrentlyRunningTestName(), __FILE__, __LINE__)

#define RT_TEST_HARNESS_FUNC(Prefix) class Prefix##Function : public Function { \
public: \
Prefix##Function() { ProtonTester::Register##Prefix##Function(this); } \
virtual ~Prefix##Function() {} \
virtual void execute(); \
}; \
Prefix##Function g_##Prefix##Function; \
void Prefix##Function::execute()


#endif // PROTONTESTER_INTERNAL_H
