#include "TestRunner.h"

#include <cstdio>

CryTestRunner::CryTestRunner()
	: m_testCount(0)
	, m_failureCount(0)
{
}

void CryTestRunner::Run(const char* testName, TestFunction testFunction)
{
	const int failuresBeforeTest = m_failureCount;
	++m_testCount;

	testFunction(*this);

	if (m_failureCount == failuresBeforeTest)
	{
		std::printf("[PASS] %s\n", testName);
	}
	else
	{
		std::printf("[FAIL] %s\n", testName);
	}
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

int CryTestRunner::ExitCode() const
{
	std::printf(
		"\n%d test(s), %d failure(s)\n",
		m_testCount,
		m_failureCount);
	return m_failureCount == 0 ? 0 : 1;
}
