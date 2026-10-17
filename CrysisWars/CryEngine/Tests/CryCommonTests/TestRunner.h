#pragma once

class CryTestRunner
{
public:
	typedef void (*TestFunction)(CryTestRunner& runner);

	CryTestRunner();

	void Run(const char* testName, TestFunction testFunction);
	template<typename TestContext>
	void Run(
		const char* testName,
		void (*testFunction)(CryTestRunner& runner, TestContext& context),
		TestContext& context)
	{
		const int failuresBeforeTest = BeginTest();
		testFunction(*this, context);
		EndTest(testName, failuresBeforeTest);
	}
	void Check(bool condition, const char* expression, const char* file, int line);
	int TestCount() const;
	int FailureCount() const;
	int ExitCode() const;

private:
	int BeginTest();
	void EndTest(const char* testName, int failuresBeforeTest);

	int m_testCount;
	int m_failureCount;
};

#define CRY_TEST_CHECK(runner, expression) \
	(runner).Check(static_cast<bool>(expression), #expression, __FILE__, __LINE__)
