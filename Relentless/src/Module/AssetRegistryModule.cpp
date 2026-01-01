#include "Utility/Common.h"
#include "AssetRegistryModule.h"
#include "Core/Application.h"

namespace Relentless
{
	void AssetRegistryModule::AssetCreated(const AssetData& aAssetMetaData) noexcept
	{
		{
			std::unique_lock<std::shared_mutex> lock(m_Mutex);
			AssetDataStorage& storage = m_AssetDataStorages[aAssetMetaData.Type];
			storage.AssetDatas[aAssetMetaData.Uuid] = aAssetMetaData;
		}

		Application::Get().SubmitToMainThread([this, aAssetMetaData]()
			{
				OnAssetAdded(aAssetMetaData);
			});
	}

	void AssetRegistryModule::AssetRemoved(const AssetData& aAssetMetaData) noexcept
	{
		{
			std::unique_lock<std::shared_mutex> lock(m_Mutex);
			AssetDataStorage& storage = m_AssetDataStorages[aAssetMetaData.Type];
			storage.AssetDatas.erase(aAssetMetaData.Uuid);
		}

		Application::Get().SubmitToMainThread([this, aAssetMetaData]()
			{
				OnAssetRemoved(aAssetMetaData);
			});
	}

	AssetData* AssetRegistryModule::FindAsset(const UUID& aUUID) noexcept
	{
		std::shared_lock<std::shared_mutex> lock(m_Mutex);
		for (auto& [type, storage] : m_AssetDataStorages)
		{
			if (auto it = storage.AssetDatas.find(aUUID); it != storage.AssetDatas.end())
				return &it->second;
		}
		
		return nullptr;
	}

	const AssetData* AssetRegistryModule::FindAsset(const UUID& aUUID) const noexcept
	{
		std::shared_lock<std::shared_mutex> lock(m_Mutex);
		for (const auto& [type, storage] : m_AssetDataStorages)
		{
			if (auto it = storage.AssetDatas.find(aUUID); it != storage.AssetDatas.end())
				return &it->second;
		}
		
		return nullptr;
	}

	AssetData* AssetRegistryModule::FindAsset(const UUID& aUUID, const TypeIndex& aTypeIndex) noexcept
	{
		std::shared_lock<std::shared_mutex> lock(m_Mutex);
		if (!m_AssetDataStorages.contains(aTypeIndex))
			return nullptr;

		AssetDataStorage& storage = m_AssetDataStorages.at(aTypeIndex);

		if (auto it = storage.AssetDatas.find(aUUID); it != storage.AssetDatas.end())
			return &it->second;
		
		return nullptr;
	}

	const AssetData* AssetRegistryModule::FindAsset(const UUID& aUUID, const TypeIndex& aTypeIndex) const noexcept
	{
		std::shared_lock<std::shared_mutex> lock(m_Mutex);
		if (!m_AssetDataStorages.contains(aTypeIndex))
			return nullptr;

		const AssetDataStorage& storage = m_AssetDataStorages.at(aTypeIndex);

		if (auto it = storage.AssetDatas.find(aUUID); it != storage.AssetDatas.end())
			return &it->second;

		return nullptr;
	}

	const AssetData* AssetRegistryModule::FindAssetByPackagePath(const Path& aPath) const noexcept
	{
		std::shared_lock<std::shared_mutex> lock(m_Mutex);
		for (const auto& [type, storage] : m_AssetDataStorages)
		{
			for (const auto& [id, metaData] : storage.AssetDatas)
			{
				if (metaData.PackagePath == aPath)
					return &metaData;
			}
		}
		
		return nullptr;
	}

	const AssetData* AssetRegistryModule::FindAssetBySourcePath(const Path& aPath) const noexcept
	{
		std::shared_lock<std::shared_mutex> lock(m_Mutex);
		for (const auto& [type, storage] : m_AssetDataStorages)
		{
			for (const auto& [id, metaData] : storage.AssetDatas)
			{
				if (metaData.SourcePath == aPath)
					return &metaData;
			}
		}
		
		return nullptr;
	}

	std::vector<const AssetData*> AssetRegistryModule::GetAllAssetsOfType(const TypeIndex& aTypeIndex) const noexcept
	{
		std::vector<const AssetData*> assetDatas;

		std::shared_lock<std::shared_mutex> lock(m_Mutex);
		if (m_AssetDataStorages.contains(aTypeIndex))
		{
			const AssetDataStorage& storage = m_AssetDataStorages.at(aTypeIndex);
			assetDatas.reserve(storage.AssetDatas.size());

			for (const auto& [uuid, meta] : storage.AssetDatas)
				assetDatas.push_back(&meta);
		}

		return assetDatas;
	}
}