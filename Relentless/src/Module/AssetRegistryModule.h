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
	enum class EFolderOpError : uint8{ None = 0u, InvalidName, InvalidTarget, NameTaken, SourceNotFound, IOFailure, Blocked };
	enum class ENameStatus : uint8 { Ok = 0u, Empty, Unchanged, Taken, Invalid };

	struct AssetRoot
	{
		String MountName;
		String DisplayName;
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

	struct FolderOpResult
	{
		EFolderOpError Error = EFolderOpError::None;
		String         ResultPath;

		NO_DISCARD bool IsSuccess() const noexcept { return Error == EFolderOpError::None; }

		static FolderOpResult Success(String aPath) { return { EFolderOpError::None, std::move(aPath) }; }
		static FolderOpResult Fail(EFolderOpError e) { return { e, {} }; }
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

		NO_DISCARD String AbsolutePathToVirtualPath(const Path& aAbsolutePath) const;

		bool AddPath(const String& aVirtualPath, String& aOutChildVirtualFolderPath) noexcept;
		void AssetCreated(AssetData aAssetData) noexcept;
		void AssetRemoved(const AssetData& aAssetData) noexcept;

		NO_DISCARD AssetData* FindAsset(const UUID& aUUID) noexcept;
		NO_DISCARD const AssetData* FindAsset(const UUID& aUUID) const noexcept;
		NO_DISCARD const AssetData* FindAssetByPackagePath(const Path& aPath) const noexcept;
		NO_DISCARD const AssetData* FindAssetBySourcePath(const Path& aPath) const noexcept;
		void ForEachAsset(const Callback<bool(const AssetData&)>& aOperation) noexcept;
		void ForEachAssetWithPath(const Path& aPath, const Callback<bool(const AssetData&)>& aOperation, bool aRecursive = false) noexcept;
		void ForEachAssetWithType(const TypeIndex& aType, const Callback<bool(const AssetData&)>& aOperation) noexcept;
		void ForEachChildFolder(const String& aVirtualPath, Callback<bool(const String& aVirtualPath, const String& aDisplayName, EAssetSourceType aSourceType)>&& aCallback) noexcept;
		void ForEachDescendantFolder(const String& aVirtualPath, Callback<bool(const String& aVirtualPath, const String& aDisplayName, EAssetSourceType aSourceType)>&& aCallback) noexcept;
		void ForEachRoot(Callback<bool(const String& aVirtualPath, const String& aDisplayName, EAssetSourceType aSourceType)>&& aCallback) noexcept;

		NO_DISCARD String GenerateUniqueFolderName(const String& aVirtualPath, const String& aBaseName) const noexcept;
		NO_DISCARD std::vector<const AssetData*> GetAllAssetsOfType(const TypeIndex& aTypeIndex) const noexcept;
		NO_DISCARD std::vector<AssetData> GetAssetsUnderPaths(const std::vector<String>& somePaths) const noexcept;

		NO_DISCARD bool IsLoadingAssets() const noexcept;

		FolderOpResult MovePath(const String& aFromPath, const String& aToPath) noexcept;

		NO_DISCARD String ParentOf(const String& aVirtualPath) const noexcept;

		void RegisterRoot(const String& aMountName, const String& aDisplayName, const Path& aBaseDirectory, EAssetSourceType aSourceType) noexcept;
		FolderOpResult RenameFolder(const String& aVirtualPath, const String& aNewName) noexcept;

		void ScanForAssets(const Path& aPath, bool aRecursive = true) noexcept;

		NO_DISCARD ENameStatus ValidateFolderName(const String& aParentVirtualPath, const String& aCurrentName, const String& aNameToValidate) const noexcept;
		NO_DISCARD String VirtualPathToAbsolutePath(const String& aVirtualPath) const noexcept;

		Broadcaster<void(const AssetData& aAssetData)> OnAssetAdded;
		Broadcaster<void(const AssetData& aAssetData)> OnAssetRemoved;
		Broadcaster<void()> OnFileScanDone;
		Broadcaster<void(const String& aVirtualFolderPath)> OnPathAdded;
		Broadcaster<void(const String& aOldVirtualFolderPath, const String& aNewVirtualFolderPath)> OnPathRenamed;
	private:
		NO_DISCARD AssetKeys BuildKeys(const AssetData& aAssetData) const;

		NO_DISCARD const AssetRoot* FindRootFor(const Path& aAbsoluteAssetPath) const;
		NO_DISCARD const AssetRoot* FindRootByMountName(const String& aMountName) const;

		void IndexAdd(AssetIndex aIndex, const AssetKeys& aAssetKeys) noexcept;
		void IndexMove(AssetIndex aFromIndex, AssetIndex aToindex, const AssetKeys& aAssetKeys) noexcept;
		void IndexRemove(AssetIndex aIndex, const AssetKeys& aAssetKeys) noexcept;

		NO_DISCARD String LeafOf(const String& aVirtualPath) const noexcept;

		NO_DISCARD String MakeRootVirtualPath(const AssetRoot& aRoot) const;

		void ProcessAssetFile(const Path& aPath) noexcept;
		void ProcessDirectory(const Path& aPath) noexcept;

		void RenamePathKeys(const String& aOldPath, const String& aNewPath) noexcept;
		NO_DISCARD bool RootExists(const String& aMountName) const noexcept;
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