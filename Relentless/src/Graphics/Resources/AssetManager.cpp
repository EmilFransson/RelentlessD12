#include "AssetManager.h"
#include "../../Utility/SerializeUtilities.h"
#include "TextureSerializer.h"
#include "MaterialSerializer.h"

namespace Relentless
{
	AssetManager::Data AssetManager::s_Data{};

	void AssetManager::Initialize() noexcept
	{
		EstablishGUIDToFilepathMapping(ENGINE_ASSET_DIRECTORY);
		EstablishGUIDToFilepathMapping(EDITOR_ASSET_DIRECTORY);

		s_Data.MaterialManager.Intitialize();
		s_Data.MeshManager.Initialize();
		//s_Data.TextureManager.Initialize();
	}

	MaterialManager& AssetManager::GetMaterialManager() noexcept 
	{ 
		return s_Data.MaterialManager; 
	}

	MeshManager& AssetManager::GetMeshManager() noexcept 
	{ 
		return s_Data.MeshManager; 
	}

	TextureManager& AssetManager::GetTextureManager() noexcept 
	{ 
		return s_Data.TextureManager; 
	}

	const std::string& AssetManager::GetAssetPath(const AssetHandle& assetHandle) noexcept
	{
		RLS_ASSERT(s_Data.UUIDToPathMap.contains(assetHandle.UUID), "Asset handle is invalid.");

		return s_Data.UUIDToPathMap[assetHandle.UUID];
	}

	void AssetManager::MapGUIDToFilepath(const UUID& uuid, const std::string& path) noexcept
	{
		RLS_ASSERT(!s_Data.UUIDToPathMap.contains(uuid), "UUID already has a filepath mapping established");
		//What about when multiple UUIDS lead to same .rasset??

		s_Data.UUIDToPathMap[uuid] = path + ASSET_EXTENSION;
		s_Data.PathToUUIDMap[path + ASSET_EXTENSION] = uuid;
	}

	void AssetManager::DeleteGUIDToFilepathMapping(const UUID& uuid, const std::string& path) noexcept
	{
		RLS_ASSERT(s_Data.UUIDToPathMap.contains(uuid), "UUID to filepath mapping not previously established");

		s_Data.UUIDToPathMap.erase(uuid);
		s_Data.PathToUUIDMap.erase(path + ".rasset");
	}

	[[nodiscard]] static YAML::Node FindGUIDNode(const YAML::Node& node) noexcept
	{
		// Base condition: If it's a scalar, it won't have the "GUID" key.
		if (node.IsScalar()) 
		{
			return YAML::Node();
		}

		// If it's a map and has the "GUID" key, return it.
		if (node.IsMap()) 
		{
			if (node["GUID"]) 
			{
				return node["GUID"];
			}

			// If not, iterate over its children.
			for (const auto& child : node)
			{
				YAML::Node result = FindGUIDNode(child.second);
				if (result) 
				{
					return result;
				}
			}
		}

		// If it's a sequence, iterate over its items.
		if (node.IsSequence()) 
		{
			for (const auto& item : node) 
			{
				YAML::Node result = FindGUIDNode(item);
				if (result) 
				{
					return result;
				}
			}
		}

		return YAML::Node(); // If not found in this branch.
	}

	void AssetManager::EstablishGUIDToFilepathMapping(const std::filesystem::path& startingDirectory) noexcept
	{
		RLS_ASSERT(std::filesystem::exists(startingDirectory), "Path is invalid.");

		constexpr std::string_view rassetExtension = ".rasset";

		for (const auto& entry : std::filesystem::recursive_directory_iterator(startingDirectory))
		{
			if (!std::filesystem::is_regular_file(entry.path()))
			{
				continue;
			}

			std::string currentFileExtension = entry.path().filename().extension().string();
			const bool isRAssetFile = (currentFileExtension == rassetExtension);
			if (!isRAssetFile)
			{
				std::filesystem::path rassetPathToCheck = entry.path();
				rassetPathToCheck.concat(rassetExtension);
				if (std::filesystem::exists(rassetPathToCheck))
				{
					continue;
				}

				if (currentFileExtension == ".jpg" || currentFileExtension == ".png")
				{
					TextureSerializer::SerializeDefault(entry.path().string());
				}
				else if (currentFileExtension == ".rmat")
				{
					MaterialSerializer::SerializeDefault(entry.path().string());
				}

				continue;
			}
			
			YAML::Node root = YAML::LoadFile(entry.path().string());
			YAML::Node guidNode = FindGUIDNode(root);

			UUID uuid = ConvertStringToGUID(guidNode.as<std::string>());
			s_Data.UUIDToPathMap[uuid] = entry.path().string();
			s_Data.PathToUUIDMap[entry.path().string()] = uuid;
		}
	}
}