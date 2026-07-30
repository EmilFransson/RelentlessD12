#pragma once

namespace Relentless::HashUtils
{
	NO_DISCARD inline uint64 HashBytes(const void* aData, uint64 aSize) noexcept
	{
		const auto* pBytes = static_cast<const std::byte*>(aData);
		uint64 hash = 14695981039346656037ull;
		for (uint64 i = 0; i < aSize; ++i)
			hash = (hash ^ static_cast<uint64>(pBytes[i])) * 1099511628211ull;
		return hash;
	}
}