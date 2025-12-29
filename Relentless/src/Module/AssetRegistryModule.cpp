#include "Utility/Common.h"
#include "AssetRegistryModule.h"
#include "Core/Application.h"

namespace Relentless
{
	void AssetRegistryModule::AssetCreated(const AssetMetaData& aAssetMetaData) noexcept
	{
		{
			std::unique_lock<std::shared_mutex> lock(m_Mutex);
			AssetMetaStorage& storage = m_AssetMetadataStorages[aAssetMetaData.Type];
			storage.AssetMetaDatas[aAssetMetaData.Uuid] = aAssetMetaData;
		}

		Application::Get().SubmitToMainThread([this, aAssetMetaData]()
			{
				OnAssetAdded(aAssetMetaData);
			});
	}

	void AssetRegistryModule::AssetRemoved(const AssetMetaData& aAssetMetaData) noexcept
	{
		{
			std::unique_lock<std::shared_mutex> lock(m_Mutex);
			AssetMetaStorage& storage = m_AssetMetadataStorages[aAssetMetaData.Type];
			storage.AssetMetaDatas.erase(aAssetMetaData.Uuid);
		}

		Application::Get().SubmitToMainThread([this, aAssetMetaData]()
			{
				OnAssetRemoved(aAssetMetaData);
			});
	}

	AssetMetaData* AssetRegistryModule::FindAsset(const UUID& aUUID) noexcept
	{
		std::shared_lock<std::shared_mutex> lock(m_Mutex);
		for (auto& [type, storage] : m_AssetMetadataStorages)
		{
			if (auto it = storage.AssetMetaDatas.find(aUUID); it != storage.AssetMetaDatas.end())
				return &it->second;
		}
		return nullptr;
	}

	const AssetMetaData* AssetRegistryModule::FindAsset(const UUID& aUUID) const noexcept
	{
		std::shared_lock<std::shared_mutex> lock(m_Mutex);
		for (const auto& [type, storage] : m_AssetMetadataStorages)
		{
			if (auto it = storage.AssetMetaDatas.find(aUUID); it != storage.AssetMetaDatas.end())
				return &it->second;
		}
		return nullptr;
	}

	AssetMetaData* AssetRegistryModule::FindAsset(const UUID& aUUID, const TypeIndex& aTypeIndex) noexcept
	{
		std::shared_lock<std::shared_mutex> lock(m_Mutex);
		if (!m_AssetMetadataStorages.contains(aTypeIndex))
			return nullptr;

		AssetMetaStorage& storage = m_AssetMetadataStorages.at(aTypeIndex);

		if (auto it = storage.AssetMetaDatas.find(aUUID); it != storage.AssetMetaDatas.end())
			return &it->second;
		
		return nullptr;
	}

	const AssetMetaData* AssetRegistryModule::FindAsset(const UUID& aUUID, const TypeIndex& aTypeIndex) const noexcept
	{
		std::shared_lock<std::shared_mutex> lock(m_Mutex);
		if (!m_AssetMetadataStorages.contains(aTypeIndex))
			return nullptr;

		const AssetMetaStorage& storage = m_AssetMetadataStorages.at(aTypeIndex);

		if (auto it = storage.AssetMetaDatas.find(aUUID); it != storage.AssetMetaDatas.end())
			return &it->second;

		return nullptr;
	}

	const AssetMetaData* AssetRegistryModule::FindAssetByPackagePath(const Path& aPath) const noexcept
	{
		std::shared_lock<std::shared_mutex> lock(m_Mutex);
		for (const auto& [type, storage] : m_AssetMetadataStorages)
		{
			for (const auto& [id, metaData] : storage.AssetMetaDatas)
			{
				if (metaData.PackagePath == aPath)
					return &metaData;
			}
		}
		return nullptr;
	}

	const AssetMetaData* AssetRegistryModule::FindAssetBySourcePath(const Path& aPath) const noexcept
	{
		std::shared_lock<std::shared_mutex> lock(m_Mutex);
		for (const auto& [type, storage] : m_AssetMetadataStorages)
		{
			for (const auto& [id, metaData] : storage.AssetMetaDatas)
			{
				if (metaData.SourcePath == aPath)
					return &metaData;
			}
		}
		return nullptr;
	}

	std::vector<const AssetMetaData*> AssetRegistryModule::GetAllAssetsOfType(const TypeIndex& aTypeIndex) const noexcept
	{
		std::vector<const AssetMetaData*> assetMetaDatas;

		std::shared_lock<std::shared_mutex> lock(m_Mutex);
		if (m_AssetMetadataStorages.contains(aTypeIndex))
		{
			const AssetMetaStorage& storage = m_AssetMetadataStorages.at(aTypeIndex);
			assetMetaDatas.reserve(storage.AssetMetaDatas.size());

			for (const auto& [uuid, meta] : storage.AssetMetaDatas)
				assetMetaDatas.push_back(&meta);
		}

		return assetMetaDatas;
	}
}