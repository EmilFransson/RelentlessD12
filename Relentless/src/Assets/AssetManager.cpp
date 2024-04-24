#include "AssetManager.h"

namespace Relentless
{
	AssetManager::AssetStorages AssetManager::s_AssetStorages{};
	std::unordered_map<std::string, UUID> AssetManager::s_LoadedAssets;
	std::unordered_map<UUID, AssetHandle> AssetManager::s_LoadedAssets2;
	std::unordered_map<UUID, std::string> AssetManager::s_UUIDToSrcPathMap;
	AssetHandle AssetManager::m_DefaultMaterialHandle{NULL_HANDLE};
	AssetHandle AssetManager::m_InvalidMaterialHandle{ NULL_HANDLE };
	AssetHandle AssetManager::m_InvalidTextureHandle{ NULL_HANDLE };

	std::mutex AssetManager::m_LoadedAssetsMapMutex{};

	std::unordered_set<std::string> AssetManager::m_AssetsCurrentlyBeingLoaded{};
	std::unordered_map<std::string, std::condition_variable> AssetManager::m_FilepathToConditionVariableMap{};
	std::mutex AssetManager::m_AssetsBeginLoadedMutex{};

	std::unordered_set<std::string> AssetManager::m_InvalidAssets{};

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


	void AssetManager::AddAssetFileAsCurrentlyBeingLoaded(const std::string& filepath) noexcept
	{
		RLS_ASSERT(!IsAssetFileCurrentlyBeingLoaded(filepath), "Asset file is already being loaded.");
		m_AssetsCurrentlyBeingLoaded.insert(filepath);
	}

	void AssetManager::RemoveAssetFileAsCurrentlyBeingLoaded(const std::string& filepath) noexcept
	{

		RLS_ASSERT(IsAssetFileCurrentlyBeingLoaded(filepath), "Asset file is not already being loaded.");
		m_AssetsCurrentlyBeingLoaded.erase(filepath);
	}

	bool AssetManager::IsAssetFileCurrentlyBeingLoaded(const std::string& filepath) noexcept
	{
		return m_AssetsCurrentlyBeingLoaded.contains(filepath);
	}

	void AssetManager::Initialize() noexcept
	{
		Serializer::Deserialize<Material>(std::string(ENGINE_ASSET_DIRECTORY) + "Materials\\M_DefaultMaterial.rasset", m_DefaultMaterialHandle);
		RLS_ASSERT(m_DefaultMaterialHandle != NULL_HANDLE, "[AssetManager]: Unable to deserialize default material.");

		Serializer::Deserialize<Material>(std::string(ENGINE_ASSET_DIRECTORY) + "Materials\\M_InvalidMaterial.rasset", m_InvalidMaterialHandle);
		RLS_ASSERT(m_InvalidMaterialHandle != NULL_HANDLE, "[AssetManager]: Unable to deserialize invalid material.");
	}

	bool AssetManager::IsLoaded(const std::string& filepath) noexcept
	{
		std::lock_guard<std::mutex> guard(m_LoadedAssetsMapMutex);
		bool result = s_LoadedAssets.contains(filepath) && s_LoadedAssets2.contains(s_LoadedAssets[filepath]);
		RLS_CORE_INFO("Thread {0} called AssetManager::IsLoaded - returned value is {1}", std::this_thread::get_id(), result);
		return result;
	}

	const AssetHandle AssetManager::GetDefaultMaterialHandle() noexcept
	{
		return m_DefaultMaterialHandle;
	}

	const AssetHandle AssetManager::GetInvalidMaterialHandle() noexcept
	{
		return m_InvalidMaterialHandle;
	}

	const AssetHandle AssetManager::GetInvalidTextureHandle() noexcept
	{
		return m_InvalidTextureHandle;
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
		std::lock_guard<std::mutex> guard(m_LoadedAssetsMapMutex);
		s_LoadedAssets[path] = uuid;
	}

	void AssetManager::Unlink(const std::string& path) noexcept
	{
		std::lock_guard<std::mutex> guard(m_LoadedAssetsMapMutex);
		if (s_LoadedAssets.contains(path))
			s_LoadedAssets.erase(path);
	}

	void AssetManager::SetInvalid(const std::string& path) noexcept
	{
		std::lock_guard<std::mutex> guard(m_LoadedAssetsMapMutex);
		m_InvalidAssets.insert(path);
	}

	void AssetManager::SetValid(const std::string& path) noexcept
	{
		std::lock_guard<std::mutex> guard(m_LoadedAssetsMapMutex);
		m_InvalidAssets.erase(path);
	}

	[[nodiscard]] bool AssetManager::IsValid(const std::string& path) noexcept
	{
		std::lock_guard<std::mutex> guard(m_LoadedAssetsMapMutex);
		return !m_InvalidAssets.contains(path);
	}

}