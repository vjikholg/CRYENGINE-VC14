#pragma once

int RunPhysicsRegressionPipeline(
	const wchar_t* runtimeDirectory,
	const wchar_t* gameRoot,
	const wchar_t* reportPath);

int RunPhysicsRegressionCase(
	const wchar_t* runtimeDirectory,
	const wchar_t* gameRoot,
	const char* caseName,
	const wchar_t* reportPath);
