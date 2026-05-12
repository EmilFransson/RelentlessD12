#pragma once
#include "Assets/AssetMeta.h"

#include "Callback/Broadcaster.h"
#include "Callback/Callback.h"

#include "Core/DLLExport.h"
#include "Core/StaticTypeInfo.h"

#include "DataStructure/DenseSet.h"

#include "IModule.h"

namespace Relentless
{
	enum class EAssetSourceType : uint8 { Engine = 0u, Project };

	struct AssetRoot
	{
		Path BaseDirectory;
		EAssetSourceType SourceType;
	};

	struct AssetData
	{
		String Name						= "";
		UUID Uuid						= NULL_UUID;
		Path SourcePath					= "";
		Path PackagePath				= "";
		TypeIndex Type					= TypeIndex{};
		uint64 ModificationDateAndTime	= 0u;
		uint32 Flags					= 0u;
		EAssetSourceType Source			= EAssetSourceType::Project;

		NO_DISCARD bool operator==(const AssetData& aOther) const noexcept
		{
			return Uuid == aOther.Uuid;
		}
	};

	struct AssetKeys
	{
		String FolderKey	= "";
		String FileKey		= "";
		TypeIndex Type		= {};
		UUID Uuid			= NULL_UUID;
	};

	class RLS_API AssetRegistryModule : public IModule
	{
	public:
		using AssetIndex = uint32;

		AssetRegistryModule() noexcept;
		virtual ~AssetRegistryModule() noexcept override;

		void AssetCreated(AssetData aAssetData) noexcept;
		void AssetRemoved(const AssetData& aAssetData) noexcept;

		NO_DISCARD AssetData* FindAsset(const UUID& aUUID) noexcept;
		NO_DISCARD const AssetData* FindAsset(const UUID& aUUID) const noexcept;
		NO_DISCARD const AssetData* FindAssetByPackagePath(const Path& aPath) const noexcept;
		NO_DISCARD const AssetData* FindAssetBySourcePath(const Path& aPath) const noexcept;
		void ForEachAsset(const Callback<bool(const AssetData&)>& aOperation) noexcept;
		void ForEachAssetWithPath(const Path& aPath, const Callback<bool(const AssetData&)>& aOperation, bool aRecursive = false) noexcept;
		void ForEachAssetWithType(const TypeIndex& aType, const Callback<bool(const AssetData&)>& aOperation) noexcept;

		NO_DISCARD std::vector<const AssetData*> GetAllAssetsOfType(const TypeIndex& aTypeIndex) const noexcept;

		NO_DISCARD bool IsLoadingAssets() const noexcept;

		void RegisterRoot(const Path& aBaseDirectory, EAssetSourceType aSourceType) noexcept;

		void ScanForAssets(const Path& aPath, bool aRecursive = true) noexcept;

		Broadcaster<void(const AssetData& aAssetData)> OnAssetAdded;
		Broadcaster<void(const AssetData& aAssetData)> OnAssetRemoved;
		Broadcaster<void()> OnFileScanDone;
	private:
		NO_DISCARD AssetKeys BuildKeys(const AssetData& aAssetData) const;

		NO_DISCARD const AssetRoot* FindRootFor(const Path& aAbsoluteAssetPath) const;

		void IndexAdd(AssetIndex aIndex, const AssetKeys& aAssetKeys) noexcept;
		void IndexMove(AssetIndex aFromIndex, AssetIndex aToindex, const AssetKeys& aAssetKeys) noexcept;
		void IndexRemove(AssetIndex aIndex, const AssetKeys& aAssetKeys) noexcept;

		void ProcessAssetFile(const Path& aPath) noexcept;
		void ProcessDirectory(const Path& aPath) noexcept;
	private:
		using AssetBucket = DenseSet<AssetIndex>;

		std::vector<AssetData> m_AssetDatas;
		std::vector<AssetRoot> m_Roots;
		std::unordered_map<UUID, AssetIndex> m_UUIDToAssetIndex;
		std::unordered_map<Path, AssetIndex> m_FullPathToAssetIndex;
		std::unordered_map<TypeIndex, AssetBucket> m_TypeToAssetIndexes;
		std::unordered_map<String, AssetBucket> m_PathToAssetIndexes;
		
		std::unordered_map<String, std::unordered_set<String>> m_PathToFolders;

		mutable std::shared_mutex m_Mutex;
		std::atomic<bool> m_IsLoadingAssets = false;
	};
}