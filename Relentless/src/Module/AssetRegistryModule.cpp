#include "Utility/Common.h"
#include "AssetRegistryModule.h"

#include "Assets/AssetManager.h"
#include "Assets/AssetMeta.h"
#include "Core/Application.h"
#include "Project/Project.h"
#include "Threading/ThreadPool.h"
#include "Utility/FilepathUtils.h"
#include "Utility/StringUtils.h"

namespace Relentless
{
	AssetRegistryModule::AssetRegistryModule() noexcept
	{
	}

	AssetRegistryModule::~AssetRegistryModule() noexcept
	{
	}

	void AssetRegistryModule::AssetCreated(AssetData aAssetData) noexcept
	{
		const AssetKeys keys = BuildKeys(aAssetData);
		UUID uid = aAssetData.Uuid;

		{
			std::unique_lock<std::shared_mutex> lock(m_Mutex);
			RLS_ASSERT(!m_UUIDToAssetIndex.contains(aAssetData.Uuid), "[AssetRegistryModule::AssetCreated]: Asset already exists.");

			const AssetIndex index = static_cast<AssetIndex>(m_AssetDatas.size());
			m_AssetDatas.push_back(std::move(aAssetData));
			IndexAdd(index, keys);
		}

		Application::Get().SubmitToMainThread([this, uid = std::move(uid)]()
			{ 
				if (const AssetData* pData = FindAsset(uid))
					OnAssetAdded(*pData);
			});
	}

	void AssetRegistryModule::AssetRemoved(const AssetData& aAssetData) noexcept
	{
		const AssetKeys toRemoveKeys = BuildKeys(aAssetData);

		{
			std::unique_lock<std::shared_mutex> lock(m_Mutex);
			RLS_ASSERT(m_UUIDToAssetIndex.contains(aAssetData.Uuid), "[AssetRegistryModule::AssetRemoved]: Asset does not exist.");

			const AssetIndex toRemoveIndex = m_UUIDToAssetIndex[aAssetData.Uuid];
			const AssetIndex lastIndex = m_AssetDatas.size() - 1;

			IndexRemove(toRemoveIndex, toRemoveKeys);

			if (toRemoveIndex != lastIndex)
			{
				std::swap(m_AssetDatas[toRemoveIndex], m_AssetDatas[lastIndex]);
				const AssetKeys toMoveKeys = BuildKeys(m_AssetDatas[toRemoveIndex]);
				IndexMove(lastIndex, toRemoveIndex, toMoveKeys);
			}

			m_AssetDatas.pop_back();
		}

		Application::Get().SubmitToMainThread([this, aAssetData]() { OnAssetRemoved(aAssetData); });
	}

	AssetData* AssetRegistryModule::FindAsset(const UUID& aUUID) noexcept
	{
		std::shared_lock<std::shared_mutex> lock(m_Mutex);
		if (auto it = m_UUIDToAssetIndex.find(aUUID); it != m_UUIDToAssetIndex.end())
			return &m_AssetDatas[it->second];

		return nullptr;
	}

	const AssetData* AssetRegistryModule::FindAsset(const UUID& aUUID) const noexcept
	{
		std::shared_lock<std::shared_mutex> lock(m_Mutex);
		if (auto it = m_UUIDToAssetIndex.find(aUUID); it != m_UUIDToAssetIndex.end())
			return &m_AssetDatas[it->second];
		
		return nullptr;
	}

	const AssetData* AssetRegistryModule::FindAssetByPackagePath(const Path& aPath) const noexcept
	{
		std::shared_lock<std::shared_mutex> lock(m_Mutex);
		if (auto it = m_FullPathToAssetIndex.find(aPath); it != m_FullPathToAssetIndex.end())
			return &m_AssetDatas[it->second];

		return nullptr;
	}

	const AssetData* AssetRegistryModule::FindAssetBySourcePath(const Path& aPath) const noexcept
	{
		std::shared_lock<std::shared_mutex> lock(m_Mutex);
		for (const AssetData& assetData : m_AssetDatas)
		{
			if (assetData.SourcePath == aPath)
				return &assetData;
		}
		
		return nullptr;
	}

	void AssetRegistryModule::ForEachAsset(const Callback<bool(const AssetData&)>& aOperation) noexcept
	{
		std::shared_lock<std::shared_mutex> lock(m_Mutex);
		for (const AssetData& assetData : m_AssetDatas)
		{
			if (!aOperation(assetData))
				break;
		}
	}

	void AssetRegistryModule::ForEachAssetWithPath(const Path& aPath, const Callback<bool(const AssetData&)>& aOperation, bool aRecursive) noexcept
	{
		std::shared_lock<std::shared_mutex> lock(m_Mutex);
		
		const String start = aPath.string();

		std::queue<String> toProcess;
		toProcess.push(start);

		while (!toProcess.empty())
		{
			const String current = toProcess.front();
			toProcess.pop();

			if (auto it = m_PathToAssetIndexes.find(current); it != m_PathToAssetIndexes.end())
			{
				for (AssetIndex index : it->second.Dense())
				{
					if (!aOperation(m_AssetDatas[index]))
						return;
				}
			}

			if (!aRecursive)
				return;

			if (auto it = m_PathToFolders.find(current); it != m_PathToFolders.end())
			{
				for (const String& child : it->second)
					toProcess.push(current.empty() ? (child + "/") : (current + child + "/"));
			}
		}
	}

	void AssetRegistryModule::ForEachAssetWithType(const TypeIndex& aType, const Callback<bool(const AssetData&)>& aOperation) noexcept
	{
		std::shared_lock<std::shared_mutex> lock(m_Mutex);
		if (auto it = m_TypeToAssetIndexes.find(aType); it != m_TypeToAssetIndexes.end())
		{
			for (AssetIndex idx : it->second.Dense())
			{
				if (!aOperation(m_AssetDatas[idx]))
					return;
			}
		}
	}

	std::vector<const AssetData*> AssetRegistryModule::GetAllAssetsOfType(const TypeIndex& aTypeIndex) const noexcept
	{
		std::vector<const AssetData*> assetDatas;

		std::shared_lock<std::shared_mutex> lock(m_Mutex);
		if (auto it = m_TypeToAssetIndexes.find(aTypeIndex); it != m_TypeToAssetIndexes.end())
		{
			assetDatas.reserve(it->second.Size());

			for (AssetIndex index : it->second.Dense())
				assetDatas.push_back(&m_AssetDatas[index]);
		}

		return assetDatas;
	}

	bool AssetRegistryModule::IsLoadingAssets() const noexcept
	{
		return m_IsLoadingAssets;
	}

	void AssetRegistryModule::ScanForAssets(const Path& aPath, bool aRecursive) noexcept
	{
		namespace fs = std::filesystem;

		ThreadPool& threadPool = Application::Get().GetThreadPool();

		threadPool.Submit([this, aPath, aRecursive]()
			{
				m_IsLoadingAssets = true;
				if (aRecursive)
				{
					for (const auto& entry : fs::recursive_directory_iterator(aPath))
					{
						if (entry.is_regular_file() && FilepathUtils::ExtractExtension(entry.path()) == ".rasset")
							ProcessAssetFile(entry.path());
						else if (entry.is_directory())
							ProcessDirectory(entry.path());
					}
				}
				else
				{
					for (const auto& entry : fs::directory_iterator(aPath))
					{
						if (entry.is_regular_file() && FilepathUtils::ExtractExtension(entry.path()) == ".rasset")
							ProcessAssetFile(entry.path());
						else if (entry.is_directory())
							ProcessDirectory(entry.path());
					}
				}
				m_IsLoadingAssets = false;
				
				Application::Get().SubmitToMainThread([this]() { OnFileScanDone(); });
			});
	}

	AssetKeys AssetRegistryModule::BuildKeys(const AssetData& aAssetData) const
	{
		const AssetRoot* pRoot = FindRootFor(aAssetData.PackagePath);
		RLS_ASSERT(pRoot, "[AssetRegistryModule::BuildKeys]: Asset path doesn't belong to any registered root.");

		const Path absFull = std::filesystem::absolute(aAssetData.PackagePath).lexically_normal();
		const Path absRoot = std::filesystem::absolute(pRoot->BaseDirectory).lexically_normal();
		const Path relative = std::filesystem::relative(absFull, absRoot);

		String folder = relative.string() + "/";
		StringUtils::ReplaceCharacters(folder, '\\', '/');

		const String prefix = (pRoot->SourceType == EAssetSourceType::Engine) ? "Engine/" : "Game/";

		return AssetKeys
		{
			.FolderKey = prefix + folder,
			.FileKey = prefix + folder + aAssetData.Name + ".rasset",
			.Type = aAssetData.Type,
			.Uuid = aAssetData.Uuid
		};
	}

	const AssetRoot* AssetRegistryModule::FindRootFor(const Path& aAbsoluteAssetPath) const
	{
		const Path normalized = std::filesystem::absolute(aAbsoluteAssetPath).lexically_normal();
		for (const auto& root : m_Roots)
		{
			const Path absRoot = std::filesystem::absolute(root.BaseDirectory).lexically_normal();
			// Check if normalized starts with absRoot
			auto rel = std::filesystem::relative(normalized, absRoot);
			if (!rel.empty() && rel.native()[0] != '.') // didn't traverse upward
				return &root;
		}
		
		return nullptr;
	}

	void AssetRegistryModule::IndexAdd(AssetIndex aIndex, const AssetKeys& aAssetKeys) noexcept
	{
		RLS_ASSERT(!m_UUIDToAssetIndex.contains(aAssetKeys.Uuid), "[AssetRegistryModule::IndexAdd]: Asset data already exists.");
		m_UUIDToAssetIndex[aAssetKeys.Uuid] = aIndex;

		RLS_ASSERT(!m_FullPathToAssetIndex.contains(aAssetKeys.FileKey), "[AssetRegistryModule::IndexAdd]: Asset data already exists.");
		m_FullPathToAssetIndex[aAssetKeys.FileKey] = aIndex;

		RLS_DEBUG_ONLY(bool inserted = ) m_PathToAssetIndexes[aAssetKeys.FolderKey].Insert(aIndex);
		RLS_ASSERT(inserted, "[AssetRegistryModule::IndexAdd]: Asset data already exists in path index.");

		RLS_DEBUG_ONLY(inserted = ) m_TypeToAssetIndexes[aAssetKeys.Type].Insert(aIndex);
		RLS_ASSERT(inserted, "[AssetRegistryModule::IndexAdd]: Asset data already exists in type index.");
	}

	void AssetRegistryModule::IndexMove(AssetIndex aFromIndex, AssetIndex aToIndex, const AssetKeys& aAssetKeys) noexcept
	{
		if (aFromIndex == aToIndex)
			return;

		m_UUIDToAssetIndex[aAssetKeys.Uuid] = aToIndex;
		m_FullPathToAssetIndex[aAssetKeys.FileKey] = aToIndex;

		{
			auto it = m_PathToAssetIndexes.find(aAssetKeys.FolderKey);
			RLS_ASSERT(it != m_PathToAssetIndexes.end(), "[IndexMove] Folder bucket missing.");

			RLS_DEBUG_ONLY(const bool ok = ) it->second.Replace(aFromIndex, aToIndex);
			RLS_ASSERT(ok, "[IndexMove] Replace failed in folder bucket (missing from or to already existed).");
		}

		{
			auto it = m_TypeToAssetIndexes.find(aAssetKeys.Type);
			RLS_ASSERT(it != m_TypeToAssetIndexes.end(), "[IndexMove] Type bucket missing.");

			RLS_DEBUG_ONLY(const bool ok = ) it->second.Replace(aFromIndex, aToIndex);
			RLS_ASSERT(ok, "[IndexMove] Replace failed in type bucket (missing from or to already existed).");
		}
	}

	void AssetRegistryModule::IndexRemove(AssetIndex aIndex, const AssetKeys& aAssetKeys) noexcept
	{
		m_UUIDToAssetIndex.erase(aAssetKeys.Uuid);
		m_FullPathToAssetIndex.erase(aAssetKeys.FileKey);

		if (auto it = m_PathToAssetIndexes.find(aAssetKeys.FolderKey); it != m_PathToAssetIndexes.end())
		{
			RLS_DEBUG_ONLY(const bool erased = ) it->second.Erase(aIndex);
			RLS_ASSERT(erased, "[AssetRegistryModule::IndexRemove]: Asset data does not exist in folder index.");

			if (it->second.Empty())
				m_PathToAssetIndexes.erase(it);
		}

		if (auto it = m_TypeToAssetIndexes.find(aAssetKeys.Type); it != m_TypeToAssetIndexes.end())
		{
			RLS_DEBUG_ONLY(const bool erased = ) it->second.Erase(aIndex);
			RLS_ASSERT(erased, "[IndexRemove] Index missing in type bucket.");

			if (it->second.Empty())
				m_TypeToAssetIndexes.erase(it);
		}
	}

	void AssetRegistryModule::ProcessAssetFile(const Path& aPath) noexcept
	{
		LoadArchive archive(aPath, EArchiveFormat::Binary);
		if (!archive.IsValid())
		{
			RLS_CORE_WARN("[AssetRegistryModule::ProcessAssetFile]: Unable to load file with path: '{0}'.", aPath.string().c_str());
			return;
		}

		AssetFileContent content{};
		if (!archive.Process(content))
		{
			RLS_CORE_WARN("[AssetRegistryModule::ProcessAssetFile]: Unable to process asset file content for file with path: '{0}'.", aPath.string().c_str());
			return;
		}

		if (content.Magic != ASSET_FILE_MAGIC_NUMBER)
		{
			RLS_CORE_WARN("[AssetRegistryModule::ProcessAssetFile]: Asset file signature is invalid for file with path: '{0}'.", aPath.string().c_str());
			return;
		}

		if (FindAsset(content.AssetUUID))
			return;

		AssetData assetData{};
		assetData.Name = FilepathUtils::ExtractFilenameWithoutExtension(aPath);
		assetData.Uuid = content.AssetUUID;
		assetData.SourcePath = Path(content.SourcePath);
		assetData.PackagePath = aPath.parent_path();
		assetData.Type = AssetManager::PersistentTypeToRuntimeType(content.PersistentID);
		assetData.ModificationDateAndTime = content.ModificationDateAndTime;
		assetData.Flags = 0;

		const AssetRoot* root = FindRootFor(aPath);
		assetData.Source = root ? root->SourceType : EAssetSourceType::Project;

		AssetCreated(std::move(assetData));
	}

	void AssetRegistryModule::ProcessDirectory(const Path& aPath) noexcept
	{
		const AssetRoot* pRoot = FindRootFor(aPath);
		if (!pRoot)
		{
			RLS_CORE_WARN("[AssetRegistryModule::ProcessDirectory]: Directory doesn't belong to any registered root: '{}'", aPath.string());
			return;
		}

		const Path absFull = std::filesystem::absolute(aPath).lexically_normal();
		const Path absRoot = std::filesystem::absolute(pRoot->BaseDirectory).lexically_normal();
		const Path relative = std::filesystem::relative(absFull, absRoot);

		String pathString = relative.string();
		StringUtils::ReplaceCharacters(pathString, '\\', '/');

		const String prefix = (pRoot->SourceType == EAssetSourceType::Engine) ? "Engine/" : "Game/";

		// Tokenize and build parent path + child folder
		const std::vector<String> tokens = StringUtils::Split(pathString, '/');
		if (tokens.empty())
			return;

		String parentPath = prefix;
		for (size_t i = 0; i + 1 < tokens.size(); ++i)
			parentPath += (tokens[i] + "/");

		std::unique_lock<std::shared_mutex> lock(m_Mutex);
		m_PathToFolders[parentPath].insert(tokens.back());
	}

	void AssetRegistryModule::RegisterRoot(const Path& aBaseDirectory, EAssetSourceType aSourceType) noexcept
	{
		std::unique_lock<std::shared_mutex> lock(m_Mutex);

		const Path normalized = std::filesystem::absolute(aBaseDirectory).lexically_normal();

		for (const auto& root : m_Roots)
		{
			if (root.BaseDirectory == normalized)
			{
				RLS_CORE_WARN("[AssetRegistryModule::RegisterRoot]: Already registered: '{}'", normalized.string());
				return;
			}
		}

		m_Roots.push_back(AssetRoot{ .BaseDirectory = normalized, .SourceType = aSourceType });
	}
}