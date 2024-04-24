#pragma once

namespace Relentless
{
	struct AssetMetaData;
	enum class AssetType : uint8_t;

	struct AssetHandle;

	class AssetRegistry
	{
	public:
		static void MapAssetToFilepath(const AssetMetaData& metaData, const std::string& filepath) noexcept;
		static [[nodiscard]] bool IsUUIDMapped(const UUID& uuid) noexcept;
		static [[nodiscard]] bool IsFilepathMapped(const std::string& filepath) noexcept;
		static [[nodiscard]] bool IsUUIDToFilepathMapped(const UUID& uuid, const std::string& filepath) noexcept;
		static void RemoveUUIDToFilepathMap(const UUID& uuid, const std::string& filepath) noexcept;
		static void Reset() noexcept;
		static void ChangeMappedFilepath(const UUID& uuid, const std::string& newfilepath) noexcept;
		static [[nodiscard]] uint32_t GetMappedAssetCount() noexcept;
		static [[nodiscard]] const std::string& GetFilepath(const UUID& uuid) noexcept;
		static [[nodiscard]] const UUID& GetUUID(const std::string& filepath) noexcept;
		static void RecursiveScanDirectoryForAssets(const std::filesystem::path& startingDirectory) noexcept;
		static [[nodiscard]] const AssetType GetAssetTypeFromPath(const std::filesystem::path& filepath);
		static [[nodiscard]] const AssetMetaData& GetMetaData(const std::filesystem::path& filepath) noexcept;
		static [[nodiscard]] const AssetMetaData& GetMetaData(const AssetHandle& handle) noexcept;

	private:
		static std::mutex m_Mutex;
	};
}