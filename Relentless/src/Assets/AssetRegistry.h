#pragma once

namespace Relentless
{
	class AssetRegistry
	{
	public:
		static void MapUUIDToFilepath(const UUID& uuid, const std::string& filepath) noexcept;
		__forceinline static [[nodiscard]] bool IsUUIDMapped(const UUID& uuid) noexcept;
		__forceinline static [[nodiscard]] bool IsFilepathMapped(const std::string& filepath) noexcept;
		__forceinline static [[nodiscard]] bool IsUUIDToFilepathMapped(const UUID& uuid, const std::string& filepath) noexcept;
		static void RemoveUUIDToFilepathMap(const UUID& uuid, const std::string& filepath) noexcept;
		static void Reset() noexcept;
		static void ChangeMappedFilepath(const UUID& uuid, const std::string& newfilepath) noexcept;
		__forceinline static [[nodiscard]] uint32_t GetMappedAssetCount() noexcept;
		__forceinline static [[nodiscard]] const std::string& GetFilepath(const UUID& uuid) noexcept;
		__forceinline static [[nodiscard]] const UUID& GetUUID(const std::string& filepath) noexcept;
		static void ScanDirectoryForGUIDs(const std::filesystem::path& startingDirectory) noexcept;
	};
}