#pragma once

#include "CryMemoryManager.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <limits>
#include <memory>
#include <type_traits>

struct alignas(std::max_align_t) SSafeAlignedAllocHeader
{
	TPFAlloc allocator;
	void* allocationBase;
	size_t requestedSize;
	size_t alignment;
};

static_assert(std::is_trivially_copyable_v<SSafeAlignedAllocHeader>);
static_assert(
	sizeof(SSafeAlignedAllocHeader) % alignof(SSafeAlignedAllocHeader) == 0,
	"SafeAlloc metadata must preserve its own alignment.");

constexpr size_t SafeAllocAlignUp(size_t value, size_t alignment)
{
	return (value + alignment - 1) / alignment * alignment;
}

struct SSafeAllocStructMetadata
{
	static constexpr size_t MetadataAlignment = alignof(SSafeAlignedAllocHeader);
	static constexpr size_t MetadataSize = sizeof(SSafeAlignedAllocHeader);

	static void Store(
		void* metadataAddress,
		TPFAlloc allocator,
		void* allocationBase,
		size_t requestedSize,
		size_t alignment)
	{
		auto* const header = static_cast<SSafeAlignedAllocHeader*>(metadataAddress);
		std::construct_at(
			header,
			SSafeAlignedAllocHeader{ allocator, allocationBase, requestedSize, alignment });
	}

	static void Load(
		void* metadataAddress,
		TPFAlloc& allocator,
		void*& allocationBase,
		size_t& requestedSize,
		size_t& alignment)
	{
		const auto* const header =
			static_cast<const SSafeAlignedAllocHeader*>(metadataAddress);
		allocator = header->allocator;
		allocationBase = header->allocationBase;
		requestedSize = header->requestedSize;
		alignment = header->alignment;
	}
};

struct SSafeAllocConstexprMetadataPolicy
{
	static constexpr size_t MetadataAlignment = alignof(std::max_align_t);
	static constexpr size_t AllocatorOffset = 0;
	static constexpr size_t AllocationBaseOffset = SafeAllocAlignUp(
		AllocatorOffset + sizeof(TPFAlloc), alignof(void*));
	static constexpr size_t RequestedSizeOffset = SafeAllocAlignUp(
		AllocationBaseOffset + sizeof(void*), alignof(size_t));
	static constexpr size_t AlignmentOffset = SafeAllocAlignUp(
		RequestedSizeOffset + sizeof(size_t), alignof(size_t));
	static constexpr size_t MetadataSize = SafeAllocAlignUp(
		AlignmentOffset + sizeof(size_t), MetadataAlignment);

	template<class T>
	static void StoreValue(void* metadataAddress, size_t offset, const T& value)
	{
		auto* const destination = static_cast<unsigned char*>(metadataAddress) + offset;
		std::memcpy(destination, &value, sizeof(value));
	}

	template<class T>
	static void LoadValue(void* metadataAddress, size_t offset, T& value)
	{
		const auto* const source =
			static_cast<const unsigned char*>(metadataAddress) + offset;
		std::memcpy(&value, source, sizeof(value));
	}

	static void Store(
		void* metadataAddress,
		TPFAlloc allocator,
		void* allocationBase,
		size_t requestedSize,
		size_t alignment)
	{
		StoreValue(metadataAddress, AllocatorOffset, allocator);
		StoreValue(metadataAddress, AllocationBaseOffset, allocationBase);
		StoreValue(metadataAddress, RequestedSizeOffset, requestedSize);
		StoreValue(metadataAddress, AlignmentOffset, alignment);
	}

	static void Load(
		void* metadataAddress,
		TPFAlloc& allocator,
		void*& allocationBase,
		size_t& requestedSize,
		size_t& alignment)
	{
		LoadValue(metadataAddress, AllocatorOffset, allocator);
		LoadValue(metadataAddress, AllocationBaseOffset, allocationBase);
		LoadValue(metadataAddress, RequestedSizeOffset, requestedSize);
		LoadValue(metadataAddress, AlignmentOffset, alignment);
	}
};

static_assert(
	SSafeAllocConstexprMetadataPolicy::MetadataSize ==
		SSafeAllocStructMetadata::MetadataSize,
	"SafeAlloc benchmark policies must reserve equivalent metadata.");

static bool SafeAllocIsPowerOfTwo(size_t value)
{
	return value != 0 && (value & (value - 1)) == 0;
}

template<class MetadataPolicy>
static bool TryCalculateSafeAlignedAllocation( size_t requestedSize, size_t requestedAlignment, size_t& effectiveAlignment, size_t& allocationSize)
{
	effectiveAlignment = 0;
	allocationSize = 0;
	if (!SafeAllocIsPowerOfTwo(requestedAlignment))
		return false;

	effectiveAlignment = (std::max)(
		requestedAlignment,
		(std::max)(MetadataPolicy::MetadataAlignment, alignof(std::max_align_t)));

	constexpr size_t maximumSize = (std::numeric_limits<size_t>::max)();
	if (requestedSize > maximumSize - MetadataPolicy::MetadataSize)
		return false;

	const size_t sizeWithMetadata = requestedSize + MetadataPolicy::MetadataSize;
	const size_t alignmentSlack = effectiveAlignment - 1;
	if (sizeWithMetadata > maximumSize - alignmentSlack)
		return false;

	allocationSize = sizeWithMetadata + alignmentSlack;
	return true;
}

template<class MetadataPolicy>
static void* SafeAllocNew( TPFAlloc allocator, size_t requestedSize, size_t requestedAlignment)
{
	size_t effectiveAlignment = 0;
	size_t allocationSize = 0;
	if (!TryCalculateSafeAlignedAllocation<MetadataPolicy>(
			requestedSize,
			requestedAlignment,
			effectiveAlignment,
			allocationSize))
	{
		return nullptr;
	}

	void* const allocationBase = allocator(nullptr, allocationSize);
	if (!allocationBase)
		return nullptr;

	void* userMemory =
		static_cast<unsigned char*>(allocationBase) + MetadataPolicy::MetadataSize;
	size_t availableSize = allocationSize - MetadataPolicy::MetadataSize;
	if (!std::align(effectiveAlignment, requestedSize, userMemory, availableSize))
	{
		allocator(allocationBase, 0);
		return nullptr;
	}

	void* const metadataAddress =
		static_cast<unsigned char*>(userMemory) - MetadataPolicy::MetadataSize;
	MetadataPolicy::Store(
		metadataAddress,
		allocator,
		allocationBase,
		requestedSize,
		effectiveAlignment);
	return userMemory;
}

template<class MetadataPolicy>
static void* SafeAllocWithMetadataPolicy( TPFAlloc allocator, void* userMemory, size_t requestedSize, size_t requestedAlignment)
{
	if (!userMemory)
	{
		return requestedSize
			? SafeAllocNew<MetadataPolicy>(allocator, requestedSize, requestedAlignment)
			: nullptr;
	}

	void* const metadataAddress =
		static_cast<unsigned char*>(userMemory) - MetadataPolicy::MetadataSize;
	TPFAlloc originalAllocator = nullptr;
	void* allocationBase = nullptr;
	size_t oldRequestedSize = 0;
	size_t oldAlignment = 0;
	MetadataPolicy::Load(
		metadataAddress,
		originalAllocator,
		allocationBase,
		oldRequestedSize,
		oldAlignment);

	if (!requestedSize)
	{
		originalAllocator(allocationBase, 0);
		return nullptr;
	}

	void* const replacement = SafeAllocNew<MetadataPolicy>(
		originalAllocator,
		requestedSize,
		oldAlignment);
	if (!replacement)
		return nullptr;

	std::memcpy(replacement, userMemory, (std::min)(oldRequestedSize, requestedSize));
	originalAllocator(allocationBase, 0);
	return replacement;
}

static void* SafeAllocUsingStructMetadata( TPFAlloc allocator, void* userMemory, size_t requestedSize, size_t requestedAlignment )
{
	return SafeAllocWithMetadataPolicy<SSafeAllocStructMetadata>(
		allocator, userMemory, requestedSize, requestedAlignment);
}

static void* SafeAllocUsingConstexprMetadata( TPFAlloc allocator, void* userMemory, size_t requestedSize, size_t requestedAlignment )
{
	return SafeAllocWithMetadataPolicy<SSafeAllocConstexprMetadataPolicy>(
		allocator, userMemory, requestedSize, requestedAlignment);
}

void* SafeAlloc( TPFAlloc allocator, void* userMemory, size_t requestedSize, size_t requestedAlignment )
{
	// The typed header is the active runtime metadata layout.
	return SafeAllocWithMetadataPolicy<SSafeAllocStructMetadata>(allocator, userMemory, requestedSize, requestedAlignment);
}
