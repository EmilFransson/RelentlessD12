#include "AssetManager.h"

namespace Relentless
{
	AssetManager::AssetStorages AssetManager::s_AssetStorages{};
	std::unordered_map<std::string, UUID> AssetManager::s_LoadedAssets;
	std::unordered_map<UUID, AssetHandle> AssetManager::s_LoadedAssets2;
	AssetHandle AssetManager::m_DefaultMaterialHandle{NULL_HANDLE};

	void AssetManager::IncreaseReferenceCount(const AssetHandle& assetHandle) noexcept
	{
		if (!assetHandle.IsValid())
		{
			return;
		}

		switch (assetHandle.Type)
		{
		case AssetType::Material:
			s_AssetStorages.MaterialStorage.IncreaseReferenceCount(assetHandle);
			break;
		case AssetType::Texture2D:
			s_AssetStorages.Texture2DStorage.IncreaseReferenceCount(assetHandle);
			break;
		case AssetType::Mesh:
			s_AssetStorages.MeshStorage.IncreaseReferenceCount(assetHandle);
			break;
		case AssetType::Scene:
			//s_AssetStorages.SceneStorage.IncreaseReferenceCount(assetHandle);
			break;
		default:
			RLS_ASSERT(false, "Unknown asset type encountered");
			break;
		}
	}

	void AssetManager::DecreaseReferenceCount(const AssetHandle& assetHandle) noexcept
	{
		if (!assetHandle.IsValid())
		{
			return;
		}

		switch (assetHandle.Type)
		{
		case AssetType::Material:
			s_AssetStorages.MaterialStorage.DecreaseReferenceCount(assetHandle);
			break;
		case AssetType::Texture2D:
			s_AssetStorages.Texture2DStorage.DecreaseReferenceCount(assetHandle);
			break;
		case AssetType::Mesh:
			s_AssetStorages.MeshStorage.DecreaseReferenceCount(assetHandle);
			break;
		case AssetType::Scene:
			//s_AssetStorages.SceneStorage.DecreaseReferenceCount(assetHandle);
			break;
		default:
			RLS_ASSERT(false, "Unknown asset type encountered");
			break;
		}
	}

	void AssetManager::Initialize() noexcept
	{
		m_DefaultMaterialHandle = Serializer::Deserialize<Material>(std::string(ENGINE_ASSET_DIRECTORY) + "Materials\\M_DefaultMaterial.rasset");
	}

	bool AssetManager::IsLoaded(const std::string& filepath) noexcept
	{
		return s_LoadedAssets.contains(filepath) && s_LoadedAssets2.contains(s_LoadedAssets[filepath]);
	}

	const AssetHandle& AssetManager::GetDefaultMaterialHandle() noexcept
	{
		return m_DefaultMaterialHandle;
	}

	void AssetManager::OnFileMoved(const AssetHandle& handle, const std::string& newFilepath) noexcept
	{
		auto it = std::find_if(s_LoadedAssets.begin(), s_LoadedAssets.end(),
			[&handle](const std::pair<const std::string, UUID>& element) 
			{
				return element.second == handle.Uuid;
			});

		RLS_ASSERT(it != s_LoadedAssets.end(), "Could not find asset entry.");

		s_LoadedAssets.erase(it);
		s_LoadedAssets[newFilepath] = handle.Uuid;
	}

	void AssetManager::Link(const std::string& path, const UUID& uuid) noexcept
	{
		s_LoadedAssets[path] = uuid;
	}
}