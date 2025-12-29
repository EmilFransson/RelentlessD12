#pragma once
#include "Callback/Broadcaster.h"
#include "IModule.h"
#include <StaticTypeInfo/type_index.h>

namespace Relentless
{
	using TypeIndex = static_type_info::TypeIndex;

	struct AssetMetaData
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

	struct AssetMetaStorage
	{
		std::unordered_map<UUID, AssetMetaData> AssetMetaDatas;
	};

	class AssetRegistryModule : public IModule
	{
	public:
		void AssetCreated(const AssetMetaData& aAssetMetaData) noexcept;
		void AssetRemoved(const AssetMetaData& aAssetMetaData) noexcept;

		NO_DISCARD AssetMetaData* FindAsset(const UUID& aUUID) noexcept;
		NO_DISCARD AssetMetaData* FindAsset(const UUID& aUUID, const TypeIndex& aTypeIndex) noexcept;
		NO_DISCARD const AssetMetaData* FindAsset(const UUID& aUUID) const noexcept;
		NO_DISCARD const AssetMetaData* FindAsset(const UUID& aUUID, const TypeIndex& aTypeIndex) const noexcept;
		NO_DISCARD const AssetMetaData* FindAssetByPackagePath(const Path& aPath) const noexcept;
		NO_DISCARD const AssetMetaData* FindAssetBySourcePath(const Path& aPath) const noexcept;

		NO_DISCARD std::vector<const AssetMetaData*> GetAllAssetsOfType(const TypeIndex& aTypeIndex) const noexcept;

		Broadcaster<void(const AssetMetaData& aMetaData)> OnAssetAdded;
		Broadcaster<void(const AssetMetaData& aMetaData)> OnAssetRemoved;
	private:
		std::unordered_map<TypeIndex, AssetMetaStorage> m_AssetMetadataStorages;
		mutable std::shared_mutex m_Mutex;
	};
}