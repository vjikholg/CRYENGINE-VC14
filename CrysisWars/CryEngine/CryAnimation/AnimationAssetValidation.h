#pragma once

extern "C" CRYANIMATION_API bool CryValidateAnimationAsset(
	const char* fileName,
	char* errorMessage,
	size_t errorMessageCapacity);
