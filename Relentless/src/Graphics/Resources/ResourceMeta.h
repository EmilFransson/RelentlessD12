#pragma once

namespace Relentless
{
	using ResourceHandle = uint32_t;
	inline constexpr uint32_t NULL_RESOURCE_HANDLE = std::numeric_limits<uint32_t>::max();

	enum class EResourceLockMode : uint8_t
	{
		WriteOnly = 0u
	};
}