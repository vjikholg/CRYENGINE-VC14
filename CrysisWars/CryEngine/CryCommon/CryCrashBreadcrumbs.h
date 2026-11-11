#pragma once

#include <stdint.h>

#if defined(_WIN32)
#include <atomic>
#include <windows.h>
#endif

// This order is part of the CryRecordCrashBreadcrumbV1 ABI. Append new events only.
// AabbSplit callerAndMode uses the low two bits for transformed, local or swept (0, 1, 2).
// GridUnlink25's cell is packed X/Y when the thunk was first in a cell, or -1 otherwise.
#define CRY_CRASH_BREADCRUMB_EVENTS(X) \
	X(AabbSplit, tree, nodes, parent, child, nodeCount, scratch, callerAndMode) \
	X(GridRead, world, pool, cell, index, next, entity, capacity) \
	X(GridLink25, world, pool, cell, index, next, entity, capacity) \
	X(GridUnlink25, world, pool, cell, index, next, entity, capacity) \
	X(PhysicsStream, owner, object, operation, foreignId, flags, unused1, unused2) \
	X(OctreeWalk, node, object, childSlot, operation, unused1, unused2, unused3) \
	X(OctreeLink, node, object, head, tail, owner, unused1, unused2) \
	X(OctreeUnlink, node, object, head, tail, unused1, unused2, unused3) \
	X(OctreeFree, node, parent, head, unused1, unused2, unused3, unused4) \
	X(RenderNodeOwner, object, node, operation, unused1, unused2, unused3, unused4) \
	X(TerrainErrors, node, errors, count, operation, lod, unused1, unused2) \
	X(NetworkPacket, channel, memento, slot, phase, inSync, channelId, lockOwner) \
	X(NetworkPing, channel, memento, slot, phase, serializer, channelId, lockOwner) \
	X(NetworkMementoMissing, allocator, unused1, unused2, unused3, unused4, unused5, unused6) \
	X(NetworkChannel, channel, slot, phase, endpoint, channelId, lockOwner, unused1) \
	X(NetworkObject, context, slot, salt, entity, phase, storage, count) \
	X(NetworkObjectMiss, context, slot, salt, storage, count, unused1, unused2) \
	X(NetworkDispatch, view, context, slot, salt, sequence, unused1, unused2) \
	X(AnimationCache, animation, controllers, count, cache, cacheCount, phase, pathCrc) \
	X(AnimationLookup, animation, array, count, controllerId, pathCrc, unused1, unused2) \
	X(Skinning, mesh, character, attachment, lod, poseFrame, remapCount, joints) \
	X(SkinningStreams, mesh, renderMesh, positions, weights, tangents, stride, vertices) \
	X(SkinningRemap, mesh, character, attachment, index, source, jointCount, capacity) \
	X(FlashLoad, player, movie, definition, pathHash, phase, unused1, unused2) \
	X(FlashUse, player, movie, operation, unused1, unused2, unused3, unused4) \
	X(FlashRelease, player, movie, definition, phase, unused1, unused2, unused3) \
	X(FmodUpdate, device, events, phase, result, loops, unused1, unused2) \
	X(FmodRelease, device, events, phase, result, unused1, unused2, unused3) \
	X(FmodCallback, event, soundId, type, phase, result, unused1, unused2) \
	X(AabbLoad, tree, mesh, parent, child, nodeCount, triangles, version)

namespace CryCrashBreadcrumbs
{
	enum Event : uint32_t
	{
#define CRY_CRASH_EVENT(name, ...) name,
		CRY_CRASH_BREADCRUMB_EVENTS(CRY_CRASH_EVENT)
#undef CRY_CRASH_EVENT
		EventCount
	};

#if defined(_WIN32)
	using Writer = void (*)(uint32_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t);

	inline std::atomic<Writer>& WriterSlot()
	{
		static std::atomic<Writer> writer{nullptr};
		return writer;
	}

	inline void Bind()
	{
		HMODULE system = GetModuleHandleW(L"CrySystem.dll");
		if (system)
			WriterSlot().store(reinterpret_cast<Writer>(GetProcAddress(system, "CryRecordCrashBreadcrumbV1")), std::memory_order_release);
	}

	template<class T> inline uintptr_t Word(T value) { return static_cast<uintptr_t>(value); }
	template<class T> inline uintptr_t Word(T* value) { return reinterpret_cast<uintptr_t>(value); }

	template<class A, class B = int, class C = int, class D = int, class E = int, class F = int, class G = int>
	inline void Record(Event event, A a, B b = 0, C c = 0, D d = 0, E e = 0, F f = 0, G g = 0)
	{
		if (Writer writer = WriterSlot().load(std::memory_order_acquire))
			writer(event, Word(a), Word(b), Word(c), Word(d), Word(e), Word(f), Word(g));
	}
#else
	inline void Bind() {}
	template<class... Args> inline void Record(Event, Args...) {}
#endif
}
