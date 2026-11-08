#pragma once

#include <cstdint>
#include <limits>

struct IRenderNode;

inline bool CryHasRenderNode(const IRenderNode* node)
{
	// Sprite rendering passes a non-null placeholder rather than a render node.
	constexpr auto legacySpritePlaceholder = (std::numeric_limits<std::uint32_t>::max)();
	constexpr auto nativeSpritePlaceholder = (std::numeric_limits<std::uintptr_t>::max)();
	const auto address = reinterpret_cast<std::uintptr_t>(node);
	return node && address != legacySpritePlaceholder && address != nativeSpritePlaceholder;
}
