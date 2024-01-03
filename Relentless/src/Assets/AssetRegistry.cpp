#include "AssetRegistry.h"
#include "AssetMeta.h"
#include "Utility/Common.h"

namespace Relentless
{
	static [[nodiscard]] bool IsFilepathValid(const std::filesystem::path& filepath) noexcept
	{
		return std::filesystem::exists(filepath);
	}

	struct BiDirectionalMap
	{
		std::unordered_map<UUID, std::string> UUIDToPathMap;
		std::unordered_map<std::string, AssetMetaData> PathToAssetMetaMap;
	};
	static BiDirectionalMap s_Data;

	void AssetRegistry::MapAssetToFilepath(const AssetMetaData& metaData, const std::string& filepath) noexcept
	{
		RLS_ASSERT(!IsUUIDMapped(metaData.Uuid), "UUID is already mapped.");
		RLS_ASSERT(!IsFilepathMapped(filepath), "Filepath is already mapped.");
		RLS_ASSERT(IsFilepathValid(filepath), "Filepath is not valid.");

		s_Data.UUIDToPathMap[metaData.Uuid] = filepath;
		s_Data.PathToAssetMetaMap[filepath] = metaData;
	}
	
	bool AssetRegistry::IsUUIDMapped(const UUID& uuid) noexcept
	{
		return s_Data.UUIDToPathMap.contains(uuid);
	}
	
	bool AssetRegistry::IsFilepathMapped(const std::string& filepath) noexcept
	{
		return s_Data.PathToAssetMetaMap.contains(filepath);
	}
	
	bool AssetRegistry::IsUUIDToFilepathMapped(const UUID& uuid, const std::string& filepath) noexcept
	{
		return s_Data.UUIDToPathMap.contains(uuid) 
			&& s_Data.UUIDToPathMap[uuid] == filepath 
			&& s_Data.PathToAssetMetaMap.contains(filepath)
			&& s_Data.PathToAssetMetaMap[filepath].Uuid == uuid;
	}
	
	void AssetRegistry::RemoveUUIDToFilepathMap(const UUID& uuid, const std::string& filepath) noexcept
	{
		RLS_ASSERT(IsUUIDToFilepathMapped(uuid, filepath), "UUID and filepath is not properly mapped..");

		s_Data.UUIDToPathMap.erase(uuid);
		s_Data.PathToAssetMetaMap.erase(filepath);
	}
	
	void AssetRegistry::Reset() noexcept
	{
		s_Data.PathToAssetMetaMap.clear();
		s_Data.UUIDToPathMap.clear();
	}
	
	void AssetRegistry::ChangeMappedFilepath(const UUID& uuid, const std::string& newfilepath) noexcept
	{
		RLS_ASSERT(IsUUIDMapped(uuid), "UUID is not mapped to any filepath");
		const std::string currentFilepath = s_Data.UUIDToPathMap[uuid];
		const AssetMetaData data = s_Data.PathToAssetMetaMap[currentFilepath];

		s_Data.UUIDToPathMap.erase(uuid);
		s_Data.PathToAssetMetaMap.erase(currentFilepath);

		MapAssetToFilepath({data.Uuid, data.AssetType}, newfilepath);
	}
	
	//Checking one is sufficient as it acts as a bidirectional map
	uint32_t AssetRegistry::GetMappedAssetCount() noexcept
	{
		return static_cast<uint32_t>(s_Data.UUIDToPathMap.size());
	}
	
	const std::string& AssetRegistry::GetFilepath(const UUID& uuid) noexcept
	{
		RLS_ASSERT(IsUUIDMapped(uuid), "UUID is not mapped to any filepath.");
	
		return s_Data.UUIDToPathMap[uuid];
	}
	
	const UUID& AssetRegistry::GetUUID(const std::string& filepath) noexcept
	{
		RLS_ASSERT(IsFilepathMapped(filepath), "filepath is not mapped to any UUID.");
	
		return s_Data.PathToAssetMetaMap[filepath].Uuid;
	}
	
	void AssetRegistry::RecursiveScanDirectoryForAssets(const std::filesystem::path& startingDirectory) noexcept
	{
		RLS_ASSERT(std::filesystem::exists(startingDirectory), "Path is invalid.");
		
		for (const auto& entry : std::filesystem::recursive_directory_iterator(startingDirectory))
		{
			if (!std::filesystem::is_regular_file(entry.path()))
			{
				continue;
			}
		
			const std::string currentFileExtension = entry.path().filename().extension().string();
			const bool isRassetFile = (currentFileExtension == ASSET_EXTENSION);
			if (isRassetFile)
			{
				std::ifstream inFile(entry.path().string(), std::ios_base::binary);
				RLS_ASSERT(inFile.is_open(), "Unable to open rasset file.");
		
				RassetHeader readHeader;
				inFile.read(reinterpret_cast<char*>(&readHeader), sizeof(readHeader));
		
				if (!IsUUIDToFilepathMapped(readHeader.UUID, entry.path().string()))
				{
					MapAssetToFilepath({ readHeader.UUID, readHeader.AssetType }, entry.path().string());
				}
		
				inFile.close();
			}
		}
	}

	const AssetType AssetRegistry::GetAssetTypeFromPath(const std::filesystem::path& filepath)
	{
		RLS_ASSERT(std::filesystem::exists(filepath), "[AssetRegistry]: Filepath is invalid.");
		RLS_ASSERT(IsFilepathMapped(filepath.string()), "[AssetRegistry]: Filepath is not mapped.");
		return s_Data.PathToAssetMetaMap[filepath.string()].AssetType;
	}

	const AssetMetaData& AssetRegistry::GetMetaData(const std::filesystem::path& filepath) noexcept
	{
		RLS_ASSERT(std::filesystem::exists(filepath), "[AssetRegistry]: Filepath is invalid.");
		RLS_ASSERT(IsFilepathMapped(filepath.string()), "[AssetRegistry]: Filepath is not mapped.");
		return s_Data.PathToAssetMetaMap[filepath.string()];
	}
}