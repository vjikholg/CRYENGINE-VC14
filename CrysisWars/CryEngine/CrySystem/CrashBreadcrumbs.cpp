#include "StdAfx.h"
#include "CrashBreadcrumbs.h"
#include <CryCrashBreadcrumbs.h>

#if defined(_WIN32)

class CCrashBreadcrumbStore
{
public:
	void Record(uint32_t event, const uintptr_t (&fields)[7]);
	unsigned long Capture(const char** text);

private:
	static constexpr unsigned LaneCount = 4;
	static constexpr unsigned History = 4;
	static constexpr unsigned FieldCount = 7;
	static_assert(ATOMIC_POINTER_LOCK_FREE == 2 && ATOMIC_INT_LOCK_FREE == 2, "Breadcrumb writes must be lock-free");

	struct Slot
	{
		// An odd version means a writer owns this slot. A zero version is unused.
		std::atomic<uint32_t> version{0};
		std::atomic<uintptr_t> thread{0};
		std::atomic<uintptr_t> tickLow{0};
		std::atomic<uintptr_t> tickHigh{0};
		std::atomic<uintptr_t> fields[FieldCount]{};
	};

	struct Lane
	{
		std::atomic<uint32_t> next{0};
		Slot slots[History];
	};

	Lane m_events[CryCrashBreadcrumbs::EventCount][LaneCount];
	std::atomic<unsigned> m_captureState{0};
	unsigned long m_textSize = 0;
	char m_text[128 * 1024]{};
};

static CCrashBreadcrumbStore g_crashBreadcrumbs;

void CCrashBreadcrumbStore::Record(uint32_t event, const uintptr_t (&fields)[7])
{
	if (event >= CryCrashBreadcrumbs::EventCount || m_captureState.load(std::memory_order_relaxed))
		return;

	const DWORD thread = GetCurrentThreadId();
	// Windows thread IDs typically have their low two bits clear.
	Lane& lane = m_events[event][(thread >> 2) & (LaneCount - 1)];
	const uint32_t ticket = lane.next.fetch_add(1, std::memory_order_relaxed) + 1;
	Slot& slot = lane.slots[ticket & (History - 1)];
	uint32_t version = slot.version.load(std::memory_order_relaxed);
	if ((version & 1) || (version && static_cast<int32_t>(ticket * 2 - version) <= 0) ||
		!slot.version.compare_exchange_strong(version, ticket * 2 - 1, std::memory_order_acq_rel))
		return;

	LARGE_INTEGER counter{};
	QueryPerformanceCounter(&counter);
	slot.thread.store(thread, std::memory_order_relaxed);
	slot.tickLow.store(counter.LowPart, std::memory_order_relaxed);
	slot.tickHigh.store(static_cast<uint32_t>(counter.HighPart), std::memory_order_relaxed);
	for (unsigned i = 0; i != FieldCount; ++i)
		slot.fields[i].store(fields[i], std::memory_order_relaxed);
	slot.version.store(ticket * 2, std::memory_order_release);
}

unsigned long CCrashBreadcrumbStore::Capture(const char** text)
{
	if (!text)
		return 0;
	*text = nullptr;

	unsigned state = 0;
	if (!m_captureState.compare_exchange_strong(state, 1, std::memory_order_acq_rel))
	{
		if (state != 2)
			return 0;
		*text = m_text;
		return m_textSize;
	}

	static const char* const names[][FieldCount + 1] = {
#define CRY_CRASH_NAMES(name, a, b, c, d, e, f, g) {#name, #a, #b, #c, #d, #e, #f, #g},
		CRY_CRASH_BREADCRUMB_EVENTS(CRY_CRASH_NAMES)
#undef CRY_CRASH_NAMES
	};
	const DWORD faultThread = GetCurrentThreadId();
	LARGE_INTEGER frequency{};
	QueryPerformanceFrequency(&frequency);
	size_t used = static_cast<size_t>(_snprintf_s(m_text, sizeof(m_text), _TRUNCATE,
		"\nCryEngine crash breadcrumbs v1 (bounded, most recent per event/lane)\nfault_thread=%lu qpc_hz=%llu\n",
		faultThread, static_cast<unsigned long long>(frequency.QuadPart)));

	for (unsigned event = 0; event != CryCrashBreadcrumbs::EventCount; ++event)
		for (const Lane& lane : m_events[event])
			for (const Slot& slot : lane.slots)
			{
				const uint32_t first = slot.version.load(std::memory_order_acquire);
				if (!first || (first & 1))
					continue;

				const uintptr_t thread = slot.thread.load(std::memory_order_relaxed);
				const uint64_t low = slot.tickLow.load(std::memory_order_relaxed);
				const uint64_t high = slot.tickHigh.load(std::memory_order_relaxed);
				uintptr_t fields[FieldCount];
				for (unsigned i = 0; i != FieldCount; ++i)
					fields[i] = slot.fields[i].load(std::memory_order_relaxed);
				std::atomic_thread_fence(std::memory_order_acq_rel);
				if (slot.version.load(std::memory_order_relaxed) != first)
					continue;

				// Reserve enough room for a complete record, including the longest field names.
				if (sizeof(m_text) - used < 400)
					continue;
				used += _snprintf_s(m_text + used, sizeof(m_text) - used, _TRUNCATE,
					"%s tid=%lu qpc=%llu", names[event][0], static_cast<unsigned long>(thread),
					static_cast<unsigned long long>((high << 32) | low));
				for (unsigned i = 0; i != FieldCount; ++i)
					if (strncmp(names[event][i + 1], "unused", 6) != 0)
						used += _snprintf_s(m_text + used, sizeof(m_text) - used, _TRUNCATE,
							" %s=0x%llx", names[event][i + 1], static_cast<unsigned long long>(fields[i]));
				m_text[used++] = '\n';
				m_text[used] = '\0';
			}

	m_textSize = static_cast<unsigned long>(used + 1);
	*text = m_text;
	m_captureState.store(2, std::memory_order_release);
	return m_textSize;
}

extern "C" DLL_EXPORT void CryRecordCrashBreadcrumbV1(uint32_t event, uintptr_t a, uintptr_t b,
	uintptr_t c, uintptr_t d, uintptr_t e, uintptr_t f, uintptr_t g)
{
	const uintptr_t fields[] = {a, b, c, d, e, f, g};
	g_crashBreadcrumbs.Record(event, fields);
}

extern "C" DLL_EXPORT unsigned long CryCaptureCrashBreadcrumbs(const char** text)
{
	return g_crashBreadcrumbs.Capture(text);
}

void WriteCrashBreadcrumbsSidecar()
{
	const char* text = nullptr;
	const DWORD size = CryCaptureCrashBreadcrumbs(&text);
	if (size <= 1)
		return;

	static std::atomic_flag writtenOnce = ATOMIC_FLAG_INIT;
	if (writtenOnce.test_and_set(std::memory_order_relaxed))
		return;

	WCHAR path[MAX_PATH];
	const DWORD length = GetTempPathW(ARRAYSIZE(path), path);
	if (!length || length >= ARRAYSIZE(path) ||
		_snwprintf_s(path + length, ARRAYSIZE(path) - length, _TRUNCATE,
			L"CryEngineCrash-%lu.breadcrumbs.txt", GetCurrentProcessId()) < 0)
		return;

	const HANDLE file = CreateFileW(path, GENERIC_WRITE, FILE_SHARE_READ, nullptr,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file != INVALID_HANDLE_VALUE)
	{
		DWORD written;
		WriteFile(file, text, size - 1, &written, nullptr);
		CloseHandle(file);
	}
}

#endif
