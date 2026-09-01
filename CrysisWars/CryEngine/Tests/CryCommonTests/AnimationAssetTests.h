#pragma once

int RunAnimationAssetFormatCorpus(
	const wchar_t* assetRoot,
	const wchar_t* reportPath);

int RunAnimationAssetCompatibilityPipeline(
	const wchar_t* runtimeDirectory,
	const wchar_t* gameRoot,
	const wchar_t* assetRoot,
	const wchar_t* reportPath);

int RunAnimationAssetRuntimeCorpus(
	const wchar_t* runtimeDirectory,
	const wchar_t* gameRoot,
	const wchar_t* assetRoot,
	const wchar_t* reportPath,
	const wchar_t* checkpointPath);
