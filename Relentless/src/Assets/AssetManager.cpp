#include "AssetManager.h"
#include "Core/Application.h"
#include "Module/AssetToolsModule.h"
#include "Threading/ThreadPool.h"

namespace Relentless
{
	AssetHandle AssetManager::FindAsset(const TypeIndex& aType, const UUID& aUUID) noexcept
	{
		if (auto it = Storages().find(aType); it != Storages().end())
		{
			const AssetHandle handle(aType, aUUID);
			if (it->second->Exists(handle))
				return handle;
		}

		return AssetHandle::INVALID;
	}

	void AssetManager::Shutdown() noexcept
	{
		for (const auto& [id, pStorage] : Storages())
			pStorage->DestroyAll();
	}

	jg::dense_hash_map<TypeIndex, UniquePtr<AssetStorage>>& AssetManager::Storages() noexcept
	{
		static jg::dense_hash_map<TypeIndex, UniquePtr<AssetStorage>> s;
		return s;
	}

	AssetHandle AssetManager::LoadAsset(const String& aFilepath) noexcept
	{
		const String filePath = aFilepath + ".rasset";

		AssetRegistryModule& assetRegistry = ModuleManager::LoadModuleChecked<AssetRegistryModule>();

		const AssetData* pAssetData = assetRegistry.FindAssetByPackagePath(filePath);
		if (!pAssetData)
			return AssetHandle::INVALID;

		if (const AssetHandle handle = FindAsset(pAssetData->Type, pAssetData->Uuid); handle.IsValid())
			return handle;

		AssetToolsModule& assetTools = ModuleManager::LoadModuleChecked<AssetToolsModule>();
		Ref<IFactory> pFactory = assetTools.GetSupportingFactory(pAssetData->Type);
		if (!pFactory || !pFactory->CanCreateNew())
			return AssetHandle::INVALID;

		FactoryCreateResult result = pFactory->CreateNew(pAssetData->Type, pAssetData->Name, pAssetData->Uuid);
		if (!result)
			return AssetHandle::INVALID;

		Ref<IAsset> pNewAsset = result.value();

		Path fullPath = FilepathUtils::Combine(Project::GetProjectDirectory(), filePath);

		LoadArchive coreArchive(fullPath, EArchiveFormat::Binary);
		if (!coreArchive.IsValid())
			return AssetHandle::INVALID;

		AssetFileContent content{};
		if (!coreArchive.Process(content))
			return AssetHandle::INVALID;

		if (!pNewAsset->SerializeCore(coreArchive))
			return AssetHandle::INVALID;

		if (content.BulkDataSize > 0)
		{
			FilepathUtils::SetExtension(fullPath, ".rbulk");
			LoadArchive bulkArchive(fullPath, EArchiveFormat::Binary);
			if (!bulkArchive.IsValid())
				return AssetHandle::INVALID;

			if (!pNewAsset->SerializeBulk(bulkArchive))
				return AssetHandle::INVALID;
		}

		return RegisterAsset(pNewAsset);
	}

	AssetHandle AssetManager::LoadAsset(const AssetData& aAssetData) noexcept
	{
		if (const AssetHandle handle = FindAsset(aAssetData.Type, aAssetData.Uuid); handle.IsValid())
			return handle;

		AssetToolsModule& assetTools = ModuleManager::LoadModuleChecked<AssetToolsModule>();
		Ref<IFactory> pFactory = assetTools.GetSupportingFactory(aAssetData.Type);
		if (!pFactory || !pFactory->CanCreateNew())
			return AssetHandle::INVALID;

		FactoryCreateResult result = pFactory->CreateNew(aAssetData.Type, aAssetData.Name, aAssetData.Uuid);
		if (!result)
			return AssetHandle::INVALID;

		Ref<IAsset> pNewAsset = result.value();

		Path fullPath = FilepathUtils::Combine(Project::GetProjectDirectory(), aAssetData.PackagePath);

		LoadArchive coreArchive(fullPath, EArchiveFormat::Binary);
		if (!coreArchive.IsValid())
			return AssetHandle::INVALID;

		AssetFileContent content{};
		if (!coreArchive.Process(content))
			return AssetHandle::INVALID;

		if (!pNewAsset->SerializeCore(coreArchive))
			return AssetHandle::INVALID;

		if (content.BulkDataSize > 0)
		{
			FilepathUtils::SetExtension(fullPath, ".rbulk");
			LoadArchive bulkArchive(fullPath, EArchiveFormat::Binary);
			if (!bulkArchive.IsValid())
				return AssetHandle::INVALID;

			if (!pNewAsset->SerializeBulk(bulkArchive))
				return AssetHandle::INVALID;
		}

		return RegisterAsset(pNewAsset);
	}

	void AssetManager::LoadAssetAsync(const String& aFilepath, AssetDoneLoadingCallback&& aCallback) noexcept
	{
		RLS_ASSERT(aCallback.IsSet(), "[AssetManager::LoadAssetAsync]: Callback is invalid.");

		ThreadPool& threadPool = Application::Get().GetThreadPool();
		threadPool.Submit([aFilepath, callback = std::move(aCallback)]() mutable 
			{
				AssetHandle handle = LoadAsset(aFilepath);
				callback(handle);
			});
	}

	const UUID& AssetManager::RuntimeTypeToPersistentType(const TypeIndex& aTypeIndex) noexcept
	{
		if (Storages().contains(aTypeIndex))
			return Storages()[aTypeIndex]->GetPersistentType();

		static constexpr UUID nonexisting = NULL_UUID;
		return nonexisting;
	}

	TypeIndex AssetManager::PersistentTypeToRuntimeType(const UUID& aPersistentType) noexcept
	{
		for (const auto& [id, pStorage] : Storages())
		{
			if (pStorage->GetPersistentType() == aPersistentType)
				return pStorage->GetRuntimeType();
		}

		return INVALID_TYPE::StaticType();
	}
}