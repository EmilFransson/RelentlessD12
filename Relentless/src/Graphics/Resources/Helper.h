#pragma once

namespace std
{
	template<>
	struct hash<UUID>
	{
		std::size_t operator()(const UUID& gUid) const
		{
			const uint64_t* half = reinterpret_cast<const uint64_t*>(&gUid);
			return half[0] ^ half[1];
		}
	};
}