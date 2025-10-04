#pragma once 

namespace Relentless
{
	[[nodiscard]] UUID ConvertStringToGUID(const std::string& guidString) noexcept;
	[[nodiscard]] std::string ConvertUUIDToString(const UUID& uuid) noexcept;
	[[nodiscard]] UUID CreateUUID() noexcept;

	struct ScopedFlag 
	{
		bool& flag;
		explicit ScopedFlag(bool& f) noexcept : flag(f) { flag = true; }
		~ScopedFlag() noexcept { flag = false; }
	};
}

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