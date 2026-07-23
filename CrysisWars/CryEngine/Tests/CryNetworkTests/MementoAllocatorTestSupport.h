#pragma once

#include <cstddef>

class CMementoMemoryManager;

struct SMementoAllocatorTestStats
{
	std::size_t allocationCalls;
	std::size_t deallocationCalls;
	std::size_t liveAllocationCount;
	std::size_t liveBytes;
	std::size_t invalidDeallocationCalls;
	bool alive;
	bool destroyedWithOutstandingAllocations;
};

SMementoAllocatorTestStats GetMementoAllocatorTestStats(
	const CMementoMemoryManager* manager);
void SetCurrentMementoManagerForTest(CMementoMemoryManager* manager);
