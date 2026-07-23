#pragma once

class CryTestRunner
{
public:
	typedef void (*TestFunction)(CryTestRunner& runner);

	CryTestRunner();

	void Run(const char* testName, TestFunction testFunction);
	void Check(bool condition, const char* expression, const char* file, int line);
	int ExitCode() const;

private:
	int m_testCount;
	int m_failureCount;
};

#define CRY_TEST_CHECK(runner, expression) \
	(runner).Check(static_cast<bool>(expression), #expression, __FILE__, __LINE__)
