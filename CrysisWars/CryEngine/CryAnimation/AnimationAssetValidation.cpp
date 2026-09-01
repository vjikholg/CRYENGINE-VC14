#include "stdafx.h"
#include "AnimationAssetValidation.h"
#include "LoaderCAF.h"

bool CryValidateAnimationAsset(
	const char* fileName,
	char* errorMessage,
	size_t errorMessageCapacity)
{
	if (errorMessage != nullptr && errorMessageCapacity != 0)
		errorMessage[0] = '\0';
	if (fileName == nullptr || fileName[0] == '\0')
	{
		if (errorMessage != nullptr && errorMessageCapacity != 0)
			_snprintf_s(errorMessage, errorMessageCapacity, _TRUNCATE, "Asset path is empty");
		return false;
	}

	I3DEngine* engine3D = g_pISystem != nullptr
		? g_pISystem->GetI3DEngine()
		: nullptr;
	if (engine3D == nullptr)
	{
		if (errorMessage != nullptr && errorMessageCapacity != 0)
			_snprintf_s(errorMessage, errorMessageCapacity, _TRUNCATE, "Cry3DEngine is not initialized");
		return false;
	}
	IChunkFile* chunkFile = engine3D->CreateChunkFile(true);
	if (chunkFile == nullptr)
	{
		if (errorMessage != nullptr && errorMessageCapacity != 0)
			_snprintf_s(errorMessage, errorMessageCapacity, _TRUNCATE, "Cry3DEngine could not create a chunk reader");
		return false;
	}

	CLoaderCAF loader;
	const bool loaded = loader.LoadCAF(
		fileName,
		chunkFile,
		static_cast<ILoaderCAFListener*>(nullptr)) != nullptr;
	chunkFile->Release();
	if (loaded)
		return true;

	if (errorMessage != nullptr && errorMessageCapacity != 0)
	{
		const char* loaderError = loader.GetLastError();
		_snprintf_s(
			errorMessage,
			errorMessageCapacity,
			_TRUNCATE,
			"%s",
			loaderError != nullptr && loaderError[0] != '\0'
				? loaderError
				: "CryAnimation chunk loader rejected the asset");
	}
	return false;
}
