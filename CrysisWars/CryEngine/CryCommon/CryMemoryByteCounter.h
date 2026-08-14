#pragma once

void CryDebugBreak();

#include <CryAssert.h>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <limits>

class CryMemoryByteCounter
{
public:
	void Add(size_t size) noexcept
	{
		m_bytes.fetch_add(static_cast<uint64_t>(size), std::memory_order_relaxed);
	}

	void Remove(size_t size) noexcept
	{
		const bool removed = TryRemove(size);
		CRY_ASSERT_MESSAGE(removed, "Memory byte accounting underflow");
		(void)removed;
	}

	bool TryRemove(size_t size) noexcept
	{
		const uint64_t amount = static_cast<uint64_t>(size);
		uint64_t current = m_bytes.load(std::memory_order_relaxed);

		while (current >= amount)
		{
			if (m_bytes.compare_exchange_weak(
					current,
					current - amount,
					std::memory_order_relaxed,
					std::memory_order_relaxed))
			{
				return true;
			}
		}

		return false;
	}

	void Reset() noexcept
	{
		m_bytes.store(0, std::memory_order_relaxed);
	}

	uint64_t Load() const noexcept
	{
		return m_bytes.load(std::memory_order_relaxed);
	}

	int LoadLegacyInt() const noexcept
	{
		const uint64_t bytes = Load();
		const uint64_t maximum = static_cast<uint64_t>(std::numeric_limits<int>::max());
		return bytes > maximum ? std::numeric_limits<int>::max() : static_cast<int>(bytes);
	}

private:
	std::atomic<uint64_t> m_bytes{0};
};
