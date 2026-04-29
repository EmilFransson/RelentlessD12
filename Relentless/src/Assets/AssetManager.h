#pragma once
#include "Assets/IAsset.h"
#include "AssetMeta.h"
#include "AssetRegistry.h"

#include "Callback/Callback.h"

#include "Core/DLLExport.h"
#include "Core/StaticTypeInfo.h"

#include "Module/AssetRegistryModule.h"
#include "Module/ModuleManager.h"

#include "Project/Project.h"

#include "Utility/FilepathUtils.h"
#include <DenseHashMap/dense_hash_map.hpp>

namespace Relentless
{
	class AssetStorage
	{
	public:
		template<typename AssetType>
		NO_DISCARD __forceinline Ref<AssetType> GetAsset(const AssetHandle& aAssetHandle) const noexcept
		{
			std::shared_lock<std::shared_mutex> lock(m_Mutex);
			Ref<IAsset> base = m_AssetsMap.at(aAssetHandle.Uuid);
			return Ref<AssetType>(static_cast<AssetType*>(base.Get()));
		}

		NO_DISCARD __forceinline Ref<IAsset> GetAsset(const AssetHandle& aAssetHandle) const noexcept
		{
			std::shared_lock<std::shared_mutex> lock(m_Mutex);
			return m_AssetsMap.at(aAssetHandle.Uuid);
		}

		__forceinline void Add(const Ref<IAsset>& aAsset) noexcept
		{
			{
				std::unique_lock<std::shared_mutex> lock(m_Mutex);
				m_AssetsMap.emplace(aAsset->GetUUID(), aAsset);
			}
			OnAssetCreated(AssetHandle(m_RuntimeType, aAsset->GetUUID()));
		}

		__forceinline void Destroy(const AssetHandle& handle) noexcept
		{
			std::unique_lock<std::shared_mutex> lock(m_Mutex);
			m_AssetsMap.erase(handle.Uuid);
		}

		__forceinline void DestroyAll() noexcept
		{
			std::unique_lock<std::shared_mutex> lock(m_Mutex);
			m_AssetsMap.clear();
		}

		NO_DISCARD __forceinline bool Exists(const AssetHandle& aHandle) const noexcept
		{
			std::shared_lock<std::shared_mutex> lock(m_Mutex);
			return m_RuntimeType == aHandle.Type && m_AssetsMap.contains(aHandle.Uuid);
		}

		NO_DISCARD __forceinline uint32 GetNumAssets() const noexcept
		{
			std::shared_lock<std::shared_mutex> lock(m_Mutex);
			return static_cast<uint32>(m_AssetsMap.size());
		}

		const UUID& GetPersistentType() const noexcept
		{
			return m_PersistentType;
		}

		const TypeIndex& GetRuntimeType() const noexcept
		{
			return m_RuntimeType;
		}

		Broadcaster<void(const AssetHandle&)> OnAssetCreated;
	private:
		friend class AssetManager;
		std::unordered_map<UUID, Ref<IAsset>> m_AssetsMap;
		UUID m_PersistentType = NULL_UUID;
		TypeIndex m_RuntimeType = INVALID_TYPE::StaticType();

		mutable std::shared_mutex m_Mutex;
	};

	using AssetDoneLoadingCallback = Callback<void(const AssetHandle&)>;

	class RLS_API AssetManager
	{
	public:
		static AssetHandle FindAsset(const TypeIndex& aType, const UUID& aUUID) noexcept;

		template<typename AssetType>
		static void ForEachAsset(const Callback<bool(AssetType&)>& aOperation) noexcept
		{
			AssetStorage& assetStorage = GetStorage<AssetType>();

			std::unique_lock<std::shared_mutex> guard(assetStorage.m_Mutex);

			for (const auto&[id, asset]: assetStorage.m_AssetsMap)
			{
				if (!aOperation(static_cast<AssetType&>(*asset)))
					break;
			}
		}

		template<typename AssetType>
		static void ForEachAsset(MAYBE_UNUSED std::unique_lock<std::shared_mutex>& aLock, const Callback<bool(AssetType&)>& aOperation) noexcept
		{
			AssetStorage& assetStorage = GetStorage<AssetType>();
			RLS_ASSERT(aLock.mutex() == &assetStorage.m_Mutex && aLock.owns_lock());

			for (const auto& [id, asset] : assetStorage.m_AssetsMap)
			{
				if (!aOperation(static_cast<AssetType&>(*asset)))
					break;
			}
		}

		template<typename AssetType>
		NO_DISCARD static uint32 GetNumAssets() noexcept
		{
			return GetStorage<AssetType>().GetNumAssets();
		}

		template<typename AssetType>
		static std::unique_lock<std::shared_mutex> LockStorage() noexcept
		{
			AssetStorage& storage = GetStorage<AssetType>();
			return std::unique_lock<std::shared_mutex>(storage.m_Mutex);
		}

		static void Shutdown() noexcept;

		template<typename AssetType>
		NO_DISCARD static Ref<AssetType> Get(const AssetHandle& aAssetHandle) noexcept
		{
			RLS_ASSERT(aAssetHandle != AssetHandle::INVALID, "Asset handle is invalid.");
			return GetStorage<AssetType>().template GetAsset<AssetType>(aAssetHandle);
		}

		NO_DISCARD static Ref<IAsset> Get(const AssetHandle& aAssetHandle) noexcept
		{
			RLS_ASSERT(aAssetHandle != AssetHandle::INVALID, "Asset handle is invalid.");
			return Storages().at(aAssetHandle.Type)->GetAsset(aAssetHandle);
		}

		template<typename AssetType>
		static void Destroy(const AssetHandle& aHandle) noexcept
		{
			GetStorage<AssetType>().Destroy(aHandle);
		}

		NO_DISCARD static AssetHandle LoadAsset(const String& aFilepath) noexcept;
		NO_DISCARD static AssetHandle LoadAsset(const AssetData& aAssetData) noexcept;
		static void LoadAssetAsync(const String& aFilepath, AssetDoneLoadingCallback&& aCallback) noexcept;

		template<typename AssetType, typename InstanceType>
		static void ConnectOnAssetCreated(InstanceType* aType, void(InstanceType::*aMethod)(const AssetHandle&)) noexcept
		{
			AssetStorage& storage = GetStorage<AssetType>();
			std::unique_lock<std::shared_mutex> guard(storage.m_Mutex);
			storage.OnAssetCreated.Connect(aType, aMethod);
		}

		template<typename AssetType, typename InstanceType>
		static void ConnectOnAssetCreated(MAYBE_UNUSED std::unique_lock<std::shared_mutex>& aLock, InstanceType* aType, void(InstanceType::* aMethod)(const AssetHandle&)) noexcept
		{
			AssetStorage& storage = GetStorage<AssetType>();
			RLS_ASSERT(aLock.mutex() == &storage.m_Mutex && aLock.owns_lock());
			storage.OnAssetCreated.Connect(aType, aMethod);
		}

		template<typename AssetType, typename InstanceType>
		static void DetachOnAssetCreated(InstanceType* aType) noexcept
		{
			AssetStorage& storage = GetStorage<AssetType>();
			std::unique_lock<std::shared_mutex> guard(storage.m_Mutex);
			storage.OnAssetCreated.Detach(aType);
		}

		template<typename TAsset>
		static AssetHandle RegisterAsset(const Ref<IAsset>& aAsset) noexcept
		{
			GetStorage<TAsset>().Add(aAsset);
			const AssetHandle newHandle(TAsset::StaticType(), aAsset->GetUUID());
			return newHandle;
		}

		static AssetHandle RegisterAsset(const Ref<IAsset>& aAsset) noexcept
		{
			GetStorage(aAsset->GetStaticType()).Add(aAsset);
			const AssetHandle newHandle(aAsset->GetStaticType(), aAsset->GetUUID());
			return newHandle;
		}

		template<typename AssetType>
		static void RegisterStorage() noexcept
		{
			static constexpr TypeIndex id = getTypeIndex<AssetType>();
			if (Storages().contains(id))
				return;

			UniquePtr<AssetStorage> pStorage = MakeUnique<AssetStorage>();
			pStorage->m_PersistentType = AssetType::PersistentType();
			pStorage->m_RuntimeType = AssetType::StaticType();

			Storages()[id] = std::move(pStorage);
		}

		NO_DISCARD static const UUID& RuntimeTypeToPersistentType(const TypeIndex& aTypeIndex) noexcept;
		NO_DISCARD static TypeIndex PersistentTypeToRuntimeType(const UUID& aPersistentType) noexcept;

	private:
		static jg::dense_hash_map<TypeIndex, UniquePtr<AssetStorage>>& Storages() noexcept;

		template<typename AssetType>
		static AssetStorage& GetStorage() noexcept
		{
			static constexpr TypeIndex id = getTypeIndex<AssetType>();
			return *Storages().at(id);
		}

		static AssetStorage& GetStorage(const TypeIndex& aType) noexcept
		{
			return *Storages().at(aType);
		}
	};
}