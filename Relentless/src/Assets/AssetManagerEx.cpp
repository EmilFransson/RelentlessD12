#include "AssetManagerEx.h"

namespace Relentless
{
	AssetManagerEx::AssetStorages AssetManagerEx::s_AssetStorages{};
	std::unordered_map<std::string, AssetHandle_EX> AssetManagerEx::s_LoadedAssets;

	void AssetManagerEx::IncreaseReferenceCount(const AssetHandle_EX& assetHandle) noexcept
	{
		if (assetHandle == NULL_HANDLE_EX)
		{
			return;
		}

		switch (assetHandle.Type)
		{
		case AssetType::Material:
			//s_AssetStorages.MaterialStorage.IncreaseReferenceCount(assetHandle);
			break;
		case AssetType::Texture2D:
			s_AssetStorages.Texture2DStorage.IncreaseReferenceCount(assetHandle);
			break;
		case AssetType::Scene:
			//s_AssetStorages.SceneStorage.IncreaseReferenceCount(assetHandle);
			break;
		default:
			RLS_ASSERT(false, "Unknown asset type encountered");
			break;
		}
	}

	void AssetManagerEx::DecreaseReferenceCount(const AssetHandle_EX& assetHandle) noexcept
	{
		if (assetHandle == NULL_HANDLE_EX)
		{
			return;
		}

		switch (assetHandle.Type)
		{
		case AssetType::Material:
			//s_AssetStorages.MaterialStorage.DecreaseReferenceCount(assetHandle);
			break;
		case AssetType::Texture2D:
			s_AssetStorages.Texture2DStorage.DecreaseReferenceCount(assetHandle);
			break;
		case AssetType::Scene:
			//s_AssetStorages.SceneStorage.DecreaseReferenceCount(assetHandle);
			break;
		default:
			RLS_ASSERT(false, "Unknown asset type encountered");
			break;
		}
	}

	bool AssetManagerEx::IsLoaded(const std::string& filepath) noexcept
	{
		return s_LoadedAssets.contains(filepath);
	}
}