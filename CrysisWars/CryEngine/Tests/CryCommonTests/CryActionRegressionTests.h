#pragma once

int RunCryActionRegressionPipeline(const wchar_t* runtimeDirectory, const wchar_t* gameRoot,
    const wchar_t* reportPath, const char* family = nullptr);
int RunCryActionRegressionCase(const wchar_t* runtimeDirectory, const wchar_t* gameRoot, const char* caseName, const wchar_t* detailPath);
