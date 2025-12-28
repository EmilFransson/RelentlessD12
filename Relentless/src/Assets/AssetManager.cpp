#include "AssetManager.h"

namespace Relentless
{
	void AssetManager::Shutdown() noexcept
	{
		for (const auto& [id, pStorage] : s_AssetStorages)
			pStorage->DestroyAll();
	}

	bool AssetManager::IsLoaded(const String& aFilepath) noexcept
	{
		std::lock_guard<std::mutex> guard(s_LoadedAssetsMapMutex);
		bool result = s_LoadedAssets.contains(aFilepath);
		return result;
	}

	const AssetHandle& AssetManager::GetDefaultMaterialHandle() noexcept
	{
		return s_DefaultMaterialHandle;
	}

	const AssetHandle& AssetManager::GetInvalidMaterialHandle() noexcept
	{
		return s_InvalidMaterialHandle;
	}

	const AssetHandle& AssetManager::GetInvalidTextureHandle() noexcept
	{
		return s_InvalidTextureHandle;
	}

	void AssetManager::OnFileMoved(const AssetHandle& handle, const std::string& newFilepath) noexcept
	{
		std::lock_guard<std::mutex> guard(s_LoadedAssetsMapMutex);

		auto it = std::find_if(s_LoadedAssets.begin(), s_LoadedAssets.end(),
			[&handle](const std::pair<const std::string, UUID>& element) 
			{
				return element.second == handle.Uuid;
			});

		RLS_ASSERT(it != s_LoadedAssets.end(), "Could not find asset entry.");

		s_LoadedAssets.erase(it);
		s_LoadedAssets[newFilepath] = handle.Uuid;
	}
}