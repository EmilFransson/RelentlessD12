#pragma once
#include "Callback/Broadcaster.h"
#include "IModule.h"
#include <StaticTypeInfo/type_index.h>

namespace Relentless
{
	using TypeIndex = static_type_info::TypeIndex;

	struct AssetData
	{
		String Name							= "\0";
		UUID Uuid							= UUID{ 0 };
		Path SourcePath						= "\0";
		Path PackagePath					= "\0";
		std::vector<String> Tags			= {};
		TypeIndex Type						= TypeIndex{};
		uint64 ModificationDateAndTime		= 0u;
		uint32 Flags						= 0u;
	};

	struct AssetDataStorage
	{
		std::unordered_map<UUID, AssetData> AssetDatas;
	};

	class AssetRegistryModule : public IModule
	{
	public:
		void AssetCreated(const AssetData& aAssetData) noexcept;
		void AssetRemoved(const AssetData& aAssetData) noexcept;

		NO_DISCARD AssetData* FindAsset(const UUID& aUUID) noexcept;
		NO_DISCARD AssetData* FindAsset(const UUID& aUUID, const TypeIndex& aTypeIndex) noexcept;
		NO_DISCARD const AssetData* FindAsset(const UUID& aUUID) const noexcept;
		NO_DISCARD const AssetData* FindAsset(const UUID& aUUID, const TypeIndex& aTypeIndex) const noexcept;
		NO_DISCARD const AssetData* FindAssetByPackagePath(const Path& aPath) const noexcept;
		NO_DISCARD const AssetData* FindAssetBySourcePath(const Path& aPath) const noexcept;

		NO_DISCARD std::vector<const AssetData*> GetAllAssetsOfType(const TypeIndex& aTypeIndex) const noexcept;

		Broadcaster<void(const AssetData& aAssetData)> OnAssetAdded;
		Broadcaster<void(const AssetData& aAssetData)> OnAssetRemoved;
	private:
		std::unordered_map<TypeIndex, AssetDataStorage> m_AssetDataStorages;
		mutable std::shared_mutex m_Mutex;
	};
}