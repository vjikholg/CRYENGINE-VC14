#include "TestRunner.h"

#include <cstdio>

CryTestRunner::CryTestRunner()
	: m_testCount(0)
	, m_failureCount(0)
{
}

void CryTestRunner::Run(const char* testName, TestFunction testFunction)
{
	const int failuresBeforeTest = BeginTest();
	testFunction(*this);
	EndTest(testName, failuresBeforeTest);
}

int CryTestRunner::BeginTest()
{
	++m_testCount;
	return m_failureCount;
}

void CryTestRunner::EndTest(const char* testName, int failuresBeforeTest)
{
	if (m_failureCount == failuresBeforeTest)
	{
		std::printf("[PASS] %s\n", testName);
	}
	else
	{
		std::printf("[FAIL] %s\n", testName);
	}
	std::fflush(stdout);
}

void CryTestRunner::Check(bool condition, const char* expression, const char* file, int line)
{
	if (condition)
	{
		return;
	}

	++m_failureCount;
	std::printf("  %s(%d): check failed: %s\n", file, line, expression);
}

int CryTestRunner::TestCount() const
{
	return m_testCount;
}

int CryTestRunner::FailureCount() const
{
	return m_failureCount;
}

int CryTestRunner::ExitCode() const
{
	std::printf(
		"\n%d test(s), %d failure(s)\n",
		m_testCount,
		m_failureCount);
	return m_failureCount == 0 ? 0 : 1;
}
