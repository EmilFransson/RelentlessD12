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
		if (!pNewAsset->Load())
			return AssetHandle::INVALID;

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
		if (!pNewAsset->Load())
			return AssetHandle::INVALID;

		return RegisterAsset(pNewAsset);
	}

	bool AssetManager::LoadAsset(const AssetHandle& aAssetHandle) noexcept
	{
		if (const AssetHandle handle = FindAsset(aAssetHandle.Type, aAssetHandle.Uuid); handle.IsValid())
			return true;

		AssetRegistryModule& assetRegistry = ModuleManager::LoadModuleChecked<AssetRegistryModule>();

		const AssetData* pAssetData = assetRegistry.FindAsset(aAssetHandle.Uuid);
		if (!pAssetData)
			return false;

		AssetToolsModule& assetTools = ModuleManager::LoadModuleChecked<AssetToolsModule>();
		Ref<IFactory> pFactory = assetTools.GetSupportingFactory(pAssetData->Type);
		if (!pFactory || !pFactory->CanCreateNew())
			return false;

		FactoryCreateResult result = pFactory->CreateNew(pAssetData->Type, pAssetData->Name, pAssetData->Uuid);
		if (!result)
			return false;

		Ref<IAsset> pNewAsset = result.value();
		if (!pNewAsset->Load())
			return false;

		RegisterAsset(pNewAsset);

		return true;
	}

	void AssetManager::LoadAssetAsync(const String& aFilepath, AssetDoneLoadingCallback&& aCallback) noexcept
	{
		RLS_ASSERT(aCallback.IsSet(), "[AssetManager::LoadAssetAsync]: Callback is invalid.");

		ThreadPool& threadPool = Application::Get().GetThreadPool();
		threadPool.Submit([aFilepath, callback = std::move(aCallback)]() mutable 
			{
				AssetHandle handle = LoadAsset(aFilepath);
				Application::Get().SubmitToMainThread([assetHandle = std::move(handle), cb = std::move(callback)]()
					{
						cb(assetHandle);
					});
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