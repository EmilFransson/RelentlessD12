#include "AssetManager.h"

namespace Relentless
{
	AssetManager::AssetStorages AssetManager::s_AssetStorages{};
	std::unordered_map<std::string, UUID> AssetManager::s_LoadedAssets;
	std::unordered_map<UUID, AssetHandle> AssetManager::s_LoadedAssets2;
	std::unordered_map<UUID, std::string> AssetManager::s_UUIDToSrcPathMap;
	std::unordered_map<std::string, EAssetStatus> AssetManager::s_AssetPathToLoadingStateMap;
	std::unordered_map<std::string, std::condition_variable> AssetManager::s_AssetPathToConditionVariableMap;
	AssetHandle AssetManager::m_DefaultMaterialHandle{NULL_HANDLE};
	AssetHandle AssetManager::m_InvalidMaterialHandle{ NULL_HANDLE };
	AssetHandle AssetManager::m_InvalidTextureHandle{ NULL_HANDLE };

	std::mutex AssetManager::m_LoadedAssetsMapMutex{};

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
		case AssetType::TextureEx:
			s_AssetStorages.TextureStorage.IncreaseReferenceCount(assetHandle);
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
		case AssetType::TextureEx:
			s_AssetStorages.TextureStorage.DecreaseReferenceCount(assetHandle);
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
		const std::filesystem::path invalidTexturePath = FilepathUtils::Combine(ENGINE_ASSET_DIRECTORY, "Textures\\invalidtexture.rasset");
		const bool loaded = RequestLoadAsset(invalidTexturePath, m_InvalidTextureHandle);
		RLS_VERIFY(loaded, "Core engine asset 'invalidtextrue.rasset' missing.");

		//m_DefaultMaterialHandle = CreateNew<Material>();
		//Ref<Material> defaultMaterial = Get<Material>(m_DefaultMaterialHandle);
		//defaultMaterial->m_AlbedoColor = DirectX::XMFLOAT4(DirectX::Colors::White);
		//defaultMaterial->SetName("M_DefaultMaterial");
		//
		//m_InvalidMaterialHandle = CreateNew<Material>();
		//Ref<Material> invalidMaterial = Get<Material>(m_InvalidMaterialHandle);
		//invalidMaterial->m_AlbedoColor = DirectX::XMFLOAT4(DirectX::Colors::Magenta);
		//invalidMaterial->SetName("M_InvalidMaterial");
	}

	std::future<void> AssetManager::RequestAsyncLoadAsset(const std::filesystem::path& filepathToAsset, DelegateToCall&& delegateToCall) noexcept
	{
		return Application::Get().GetThreadPool().Submit([filepathToAsset, Delegate = std::move(delegateToCall)]()
			{
				bool shouldWait = false;
				bool returnDirectly = false;
				{
					std::lock_guard<std::mutex> lock(m_LoadedAssetsMapMutex);
					if (s_LoadedAssets.contains(filepathToAsset.string()))
						returnDirectly = true;
					else if (s_AssetPathToLoadingStateMap[filepathToAsset.string()] == EAssetStatus::Failed)
					{
						Delegate(NULL_HANDLE);
						return;
					}
					else if (s_AssetPathToLoadingStateMap[filepathToAsset.string()] == EAssetStatus::Loading)
						shouldWait = true;
					else
						s_AssetPathToLoadingStateMap[filepathToAsset.string()] = EAssetStatus::Loading;
				}

				if (returnDirectly)
				{
					Delegate(AssetManager::GetHandleByPath(filepathToAsset));
					return;
				}
				else if (shouldWait)
				{
					bool shouldReturnByPath = false;
					{
						std::unique_lock<std::mutex> lock(m_LoadedAssetsMapMutex);
						s_AssetPathToConditionVariableMap[filepathToAsset.string()].wait(lock, [filepathToAsset]()
							{
								return s_AssetPathToLoadingStateMap[filepathToAsset.string()] != EAssetStatus::Loading;
							});

						if (s_AssetPathToLoadingStateMap[filepathToAsset.string()] == EAssetStatus::Loaded)
							shouldReturnByPath = true;
						else
							Delegate(NULL_HANDLE);
					}
					
					if (shouldReturnByPath)
						Delegate(GetHandleByPath(filepathToAsset));
					
					return;
				}

				AssetHandle handle = NULL_HANDLE;
				Serializer::Deserialize(filepathToAsset, handle);
				{
					std::lock_guard<std::mutex> lock(m_LoadedAssetsMapMutex);

					if (handle != NULL_HANDLE) 
						s_AssetPathToLoadingStateMap[filepathToAsset.string()] = EAssetStatus::Loaded;
					else
						s_AssetPathToLoadingStateMap[filepathToAsset.string()] = EAssetStatus::Failed;

					s_AssetPathToConditionVariableMap[filepathToAsset.string()].notify_all();
				}

				Delegate(handle);
			});
	}

	bool AssetManager::RequestLoadAsset(const std::filesystem::path& filepathToAsset, AssetHandle& outHandle) noexcept
	{
		bool shouldWait = false;
		bool returnDirectly = false;
		{
			std::lock_guard<std::mutex> lock(m_LoadedAssetsMapMutex);
			if (s_LoadedAssets.contains(filepathToAsset.string()))
				returnDirectly = true;
			else if (s_AssetPathToLoadingStateMap[filepathToAsset.string()] == EAssetStatus::Failed)
			{
				outHandle = NULL_HANDLE;
				return false;
			}
			else if (s_AssetPathToLoadingStateMap[filepathToAsset.string()] == EAssetStatus::Loading)
				shouldWait = true;
			else
				s_AssetPathToLoadingStateMap[filepathToAsset.string()] = EAssetStatus::Loading;
		}

		if (returnDirectly)
		{
			outHandle = AssetManager::GetHandleByPath(filepathToAsset);
			return true;
		}
		else if (shouldWait)
		{
			bool shouldReturnByPath = false;
			{
				std::unique_lock<std::mutex> lock(m_LoadedAssetsMapMutex);
				s_AssetPathToConditionVariableMap[filepathToAsset.string()].wait(lock, [filepathToAsset]()
					{
						return s_AssetPathToLoadingStateMap[filepathToAsset.string()] != EAssetStatus::Loading;
					});

				if (s_AssetPathToLoadingStateMap[filepathToAsset.string()] == EAssetStatus::Loaded)
					shouldReturnByPath = true;
			}
			
			if (shouldReturnByPath)
			{
				outHandle = GetHandleByPath(filepathToAsset);
				return true;
			}
			else
			{
				outHandle = NULL_HANDLE;
				return false;
			}
		}

		bool succeeded = Serializer::Deserialize(filepathToAsset, outHandle);

		{
			std::lock_guard<std::mutex> lock(m_LoadedAssetsMapMutex);

			if (succeeded)
				s_AssetPathToLoadingStateMap[filepathToAsset.string()] = EAssetStatus::Loaded;
			else
				s_AssetPathToLoadingStateMap[filepathToAsset.string()] = EAssetStatus::Failed;

			s_AssetPathToConditionVariableMap[filepathToAsset.string()].notify_all();
		}

		return succeeded;
	}

	bool AssetManager::IsLoaded(const std::string& filepath) noexcept
	{
		std::lock_guard<std::mutex> guard(m_LoadedAssetsMapMutex);
		bool result = s_LoadedAssets.contains(filepath);
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
		std::lock_guard<std::mutex> guard(m_LoadedAssetsMapMutex);

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
}