#pragma once
#include "AssetMeta.h"
#include "AssetRegistry.h"
#include "Callback/Callback.h"
#include "Core/IAsset.h"

#include <StaticTypeInfo/type_index.h>
#include "../../vendor/includes/DenseHashMap/dense_hash_map.hpp"

namespace Relentless
{
	using namespace static_type_info;
	using TypeIndex = static_type_info::TypeIndex;

	class AssetStorage
	{
	public:
		template<typename AssetType>
		__forceinline NO_DISCARD Ref<AssetType> GetAsset(const AssetHandle& aAssetHandle) noexcept
		{
			std::lock_guard<std::mutex> guard(m_Mutex);

			const uint32 physicalIndex = m_SparseArray[aAssetHandle.Index];
			RLS_ASSERT(physicalIndex < m_Assets.size(), "Index out of bounds - No asset corresponds to asset handle.");

			Ref<IAsset> base = m_Assets[physicalIndex];

			return Ref<AssetType>(static_cast<AssetType*>(base.Get()));
		}

		__forceinline NO_DISCARD uint32 Add(const Ref<IAsset>& aAsset) noexcept
		{
			uint32 sparseIndex = NULL_INDEX;

			std::lock_guard<std::mutex> guard(m_Mutex);

			const uint32 physicalIndex = static_cast<uint32>(m_Assets.size());

			if (!m_FreeList.empty())
			{
				sparseIndex = m_FreeList.front();
				m_FreeList.pop();

				m_SparseArray[sparseIndex] = physicalIndex;
			}
			else
			{
				sparseIndex = static_cast<uint32>(m_Assets.size());
				m_SparseArray.push_back(sparseIndex);
			}

			m_Assets.push_back(aAsset);
			m_ReverseLookup.push_back(sparseIndex);

			return sparseIndex;
		}

		__forceinline void Destroy(const AssetHandle& handle) noexcept
		{
			const uint32 sparseIndex = handle.Index;

			std::lock_guard<std::mutex> guard(m_Mutex);

			const uint32 assetIndex = m_SparseArray[sparseIndex];

			RLS_ASSERT(assetIndex < m_Assets.size(), "Asset index is invalid.");
			if (assetIndex >= m_Assets.size())
				return;

			const uint32 assetsBackIndex = static_cast<uint32>(m_Assets.size() - 1);
			if (assetIndex != assetsBackIndex)
			{
				std::swap(m_Assets[assetIndex], m_Assets[assetsBackIndex]);

				const uint32 swappedSparseIndex = m_ReverseLookup[assetsBackIndex];
				m_SparseArray[swappedSparseIndex] = assetIndex;
				m_ReverseLookup[assetIndex] = swappedSparseIndex;
			}

			m_Assets.pop_back();
			m_ReverseLookup.pop_back();

			m_SparseArray[sparseIndex] = std::numeric_limits<uint32>::max();
			m_FreeList.push(sparseIndex);
		}

		__forceinline void DestroyAll() noexcept
		{
			std::lock_guard<std::mutex> guard(m_Mutex);
			m_Assets.clear();
			m_SparseArray.clear();
			m_ReverseLookup.clear();
		}

		__forceinline NO_DISCARD uint32 GetNumAssets() noexcept
		{
			std::lock_guard<std::mutex> guard(m_Mutex);
			return static_cast<uint32>(m_Assets.size());
		}

		__forceinline NO_DISCARD uint32 GetPhysicalIndex(const AssetHandle& aHandle) noexcept
		{
			std::lock_guard<std::mutex> guard(m_Mutex);
			return m_SparseArray[aHandle.Index];
		}

	private:
		friend class AssetManager;

		std::vector<uint32> m_SparseArray;
		std::vector<uint32> m_ReverseLookup;
		std::vector<Ref<IAsset>> m_Assets;
		std::queue<uint32> m_FreeList;

		std::mutex m_Mutex;
	};

	class AssetManager
	{
	public:
		template<typename AssetType>
		static void ForEachAsset(const Callback<bool(AssetType&)>& aOperation) noexcept
		{
			AssetStorage& assetStorage = GetStorage<AssetType>();

			std::lock_guard<std::mutex> guard(assetStorage.m_Mutex);

			for (const Ref<IAsset>& pAsset : assetStorage.m_Assets)
			{
				if (!aOperation(static_cast<AssetType&>(*pAsset)))
					break;
			}
		}

		template<typename AssetType>
		static NO_DISCARD uint32 GetNumAssets() noexcept
		{
			return GetStorage<AssetType>().GetNumAssets();
		}

		template<typename AssetType>
		static NO_DISCARD uint32 GetPhysicalIndex(const AssetHandle& aAssetHandle) noexcept
		{
			RLS_ASSERT(aAssetHandle != AssetHandle::INVALID, "Asset handle is invalid.");
			return GetStorage<AssetType>().GetPhysicalIndex(aAssetHandle);
		}

		static void Shutdown() noexcept;

		template<typename AssetType>
		static NO_DISCARD Ref<AssetType> Get(const AssetHandle& aAssetHandle) noexcept
		{
			RLS_ASSERT(aAssetHandle != AssetHandle::INVALID, "Asset handle is invalid.");
			return GetStorage<AssetType>().GetAsset<AssetType>(aAssetHandle);
		}

		static NO_DISCARD AssetHandle GetHandleByPath(const Path& aFullPath) noexcept
		{
			std::lock_guard<std::mutex> guard(s_LoadedAssetsMapMutex);

			RLS_ASSERT(s_LoadedAssets.contains(aFullPath.string()), "Path to asset is invalid.");
			return s_LoadedAssets2[s_LoadedAssets[aFullPath.string()]];
		}

		template<typename AssetType>
		static void Destroy(const AssetHandle& aHandle) noexcept
		{
			GetStorage<AssetType>().Destroy(aHandle);
		}

		static NO_DISCARD bool IsLoaded(const String& aFilepath) noexcept;
		static NO_DISCARD const AssetHandle& GetDefaultMaterialHandle() noexcept;
		static NO_DISCARD const AssetHandle& GetInvalidMaterialHandle() noexcept;
		static NO_DISCARD const AssetHandle& GetInvalidTextureHandle() noexcept;
		static void OnFileMoved(const AssetHandle& aHandle, const String& aNewFilepath) noexcept;

		template<typename TAsset>
		static AssetHandle RegisterAsset(const Ref<IAsset>& aAsset) noexcept
		{
			const uint32 sparseIndex = GetStorage<TAsset>().Add(aAsset);
			const AssetHandle newHandle(TAsset::StaticType(), aAsset->GetUUID(), sparseIndex);
			return newHandle;
		}

		template<typename AssetType>
		static void RegisterStorage() noexcept
		{
			static constexpr TypeIndex id = getTypeIndex<AssetType>();
			if (s_AssetStorages.contains(id))
				return;

			s_AssetStorages[id] = std::move(MakeUnique<AssetStorage>());
		}

	private:
		template<typename AssetType>
		static AssetStorage& GetStorage() noexcept
		{
			static constexpr TypeIndex id = getTypeIndex<AssetType>();
			return *s_AssetStorages.at(id);
		}

	private:
		inline static jg::dense_hash_map<TypeIndex, UniquePtr<AssetStorage>> s_AssetStorages;

		inline static std::unordered_map<String, UUID> s_LoadedAssets;
		inline static std::unordered_map<UUID, AssetHandle> s_LoadedAssets2;

		inline static AssetHandle s_DefaultMaterialHandle;
		inline static AssetHandle s_InvalidMaterialHandle;
		inline static AssetHandle s_InvalidTextureHandle;

		inline static std::mutex s_LoadedAssetsMapMutex;
	};
}