#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#include <platform.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <processtopologyapi.h>

#include "MM03BenchmarkEnvironment.h"

#include <limits>
#include <vector>

struct MM03ProcessorCandidate
{
	DWORD_PTR affinityMask;
	BYTE efficiencyClass;
	BYTE logicalProcessorIndex;
	bool valid;
};

static BYTE MM03FindLogicalProcessorIndex(DWORD_PTR affinityMask)
{
	for (BYTE index = 0; index < sizeof(DWORD_PTR) * 8; ++index)
	{
		if (affinityMask & (DWORD_PTR{ 1 } << index))
			return index;
	}

	return (std::numeric_limits<BYTE>::max)();
}

static DWORD_PTR MM03ChooseLogicalProcessor(DWORD_PTR availableMask)
{
	if (availableMask & ~DWORD_PTR{ 1 })
		availableMask &= ~DWORD_PTR{ 1 };

	for (size_t index = sizeof(DWORD_PTR) * 8; index > 0; --index)
	{
		const DWORD_PTR candidate = DWORD_PTR{ 1 } << (index - 1);
		if (availableMask & candidate)
			return candidate;
	}

	return 0;
}

static MM03ProcessorCandidate MM03SelectProcessorCandidate(
	WORD processorGroup,
	DWORD_PTR availableMask)
{
	MM03ProcessorCandidate selected = {};
	DWORD bufferSize = 0;
	if (GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &bufferSize) ||
		GetLastError() != ERROR_INSUFFICIENT_BUFFER)
	{
		return selected;
	}

	std::vector<unsigned char> buffer(bufferSize);
	auto* const first = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(
		buffer.data());
	if (!GetLogicalProcessorInformationEx(RelationProcessorCore, first, &bufferSize))
		return selected;

	DWORD offset = 0;
	while (offset < bufferSize)
	{
		auto* const information =
			reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(
				buffer.data() + offset);
		const PROCESSOR_RELATIONSHIP& processor = information->Processor;
		for (WORD groupIndex = 0; groupIndex < processor.GroupCount; ++groupIndex)
		{
			const GROUP_AFFINITY& group = processor.GroupMask[groupIndex];
			if (group.Group != processorGroup)
				continue;

			const DWORD_PTR candidateMask =
				MM03ChooseLogicalProcessor(group.Mask & availableMask);
			if (!candidateMask)
				continue;

			const BYTE logicalProcessorIndex =
				MM03FindLogicalProcessorIndex(candidateMask);
			if (!selected.valid ||
				processor.EfficiencyClass > selected.efficiencyClass ||
				(processor.EfficiencyClass == selected.efficiencyClass &&
				 logicalProcessorIndex > selected.logicalProcessorIndex))
			{
				selected.affinityMask = candidateMask;
				selected.efficiencyClass = processor.EfficiencyClass;
				selected.logicalProcessorIndex = logicalProcessorIndex;
				selected.valid = true;
			}
		}

		if (!information->Size)
			break;
		offset += information->Size;
	}

	return selected;
}

MM03BenchmarkEnvironment::MM03BenchmarkEnvironment()
	: m_previousAffinityMask(0)
	, m_previousPriorityClass(0)
	, m_previousThreadPriority(THREAD_PRIORITY_NORMAL)
	, m_previousPriorityBoostDisabled(FALSE)
	, m_previousProcessorGroup(0)
	, m_selectedProcessorGroup(0)
	, m_selectedLogicalProcessor((std::numeric_limits<std::uint8_t>::max)())
	, m_affinityLocked(false)
	, m_processPriorityRaised(false)
	, m_threadPriorityRaised(false)
	, m_priorityBoostChanged(false)
{
	HANDLE const process = GetCurrentProcess();
	HANDLE const thread = GetCurrentThread();

	GROUP_AFFINITY previousGroupAffinity = {};
	if (GetThreadGroupAffinity(thread, &previousGroupAffinity))
	{
		m_previousAffinityMask = previousGroupAffinity.Mask;
		m_previousProcessorGroup = previousGroupAffinity.Group;
		const MM03ProcessorCandidate candidate = MM03SelectProcessorCandidate(
			previousGroupAffinity.Group,
			previousGroupAffinity.Mask);
		const DWORD_PTR selectedMask = candidate.valid
			? candidate.affinityMask
			: MM03ChooseLogicalProcessor(previousGroupAffinity.Mask);

		if (selectedMask)
		{
			GROUP_AFFINITY selectedAffinity = previousGroupAffinity;
			selectedAffinity.Mask = selectedMask;
			if (SetThreadGroupAffinity(thread, &selectedAffinity, nullptr))
			{
				m_affinityLocked = true;
				m_selectedProcessorGroup = selectedAffinity.Group;
				m_selectedLogicalProcessor = MM03FindLogicalProcessorIndex(selectedMask);
			}
		}
	}

	m_previousPriorityClass = GetPriorityClass(process);
	if (m_previousPriorityClass && SetPriorityClass(process, HIGH_PRIORITY_CLASS))
		m_processPriorityRaised = true;

	m_previousThreadPriority = GetThreadPriority(thread);
	if (m_previousThreadPriority != THREAD_PRIORITY_ERROR_RETURN &&
		SetThreadPriority(thread, THREAD_PRIORITY_HIGHEST))
	{
		m_threadPriorityRaised = true;
	}

	BOOL priorityBoostDisabled = FALSE;
	if (GetThreadPriorityBoost(thread, &priorityBoostDisabled))
	{
		m_previousPriorityBoostDisabled = priorityBoostDisabled;
		m_priorityBoostChanged = SetThreadPriorityBoost(thread, TRUE) != FALSE;
	}
}

MM03BenchmarkEnvironment::~MM03BenchmarkEnvironment()
{
	HANDLE const process = GetCurrentProcess();
	HANDLE const thread = GetCurrentThread();

	if (m_priorityBoostChanged)
		SetThreadPriorityBoost(thread, m_previousPriorityBoostDisabled);
	if (m_threadPriorityRaised)
		SetThreadPriority(thread, m_previousThreadPriority);
	if (m_processPriorityRaised)
		SetPriorityClass(process, m_previousPriorityClass);
	if (m_affinityLocked)
	{
		GROUP_AFFINITY previousAffinity = {};
		previousAffinity.Mask = m_previousAffinityMask;
		previousAffinity.Group = m_previousProcessorGroup;
		SetThreadGroupAffinity(thread, &previousAffinity, nullptr);
	}
}

bool MM03BenchmarkEnvironment::IsReady() const
{
	return m_affinityLocked &&
		m_processPriorityRaised &&
		m_threadPriorityRaised &&
		m_priorityBoostChanged;
}

bool MM03BenchmarkEnvironment::IsAffinityLocked() const
{
	return m_affinityLocked;
}

bool MM03BenchmarkEnvironment::IsProcessPriorityRaised() const
{
	return m_processPriorityRaised;
}

bool MM03BenchmarkEnvironment::IsThreadPriorityRaised() const
{
	return m_threadPriorityRaised;
}

bool MM03BenchmarkEnvironment::IsPriorityBoostDisabled() const
{
	return m_priorityBoostChanged;
}

std::uint16_t MM03BenchmarkEnvironment::ProcessorGroup() const
{
	return m_selectedProcessorGroup;
}

std::uint8_t MM03BenchmarkEnvironment::LogicalProcessorIndex() const
{
	return m_selectedLogicalProcessor;
}
