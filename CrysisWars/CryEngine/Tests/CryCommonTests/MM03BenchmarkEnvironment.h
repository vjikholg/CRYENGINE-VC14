#pragma once

#include <cstddef>
#include <cstdint>

class MM03BenchmarkEnvironment
{
public:
	MM03BenchmarkEnvironment();
	~MM03BenchmarkEnvironment();

	MM03BenchmarkEnvironment(const MM03BenchmarkEnvironment&) = delete;
	MM03BenchmarkEnvironment& operator=(const MM03BenchmarkEnvironment&) = delete;

	bool IsReady() const;
	bool IsAffinityLocked() const;
	bool IsProcessPriorityRaised() const;
	bool IsThreadPriorityRaised() const;
	bool IsPriorityBoostDisabled() const;
	std::uint16_t ProcessorGroup() const;
	std::uint8_t LogicalProcessorIndex() const;

private:
	std::uintptr_t m_previousAffinityMask;
	std::uint32_t m_previousPriorityClass;
	int m_previousThreadPriority;
	int m_previousPriorityBoostDisabled;
	std::uint16_t m_previousProcessorGroup;
	std::uint16_t m_selectedProcessorGroup;
	std::uint8_t m_selectedLogicalProcessor;
	bool m_affinityLocked;
	bool m_processPriorityRaised;
	bool m_threadPriorityRaised;
	bool m_priorityBoostChanged;
};
