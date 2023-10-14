#pragma once
#include "AssetMeta.h"
#include "AssetRegistry.h"
#include "Serializer.h"
#include "../Graphics/Resources/Helper.h"

namespace Relentless
{
	typedef uint32_t ReferenceCount;

	template<typename AssetType>
	struct AssetStorage
	{
		__forceinline [[nodiscard]] AssetType& GetAsset(const AssetHandle_EX& assetHandle) noexcept
		{
			RLS_ASSERT(!(assetHandle.Index > Assets.size()), "Index out of bounds - No asset corresponds to asset handle.");
			return Assets[assetHandle.Index];
		}

		__forceinline [[nodiscard]] uint32_t Add(AssetType assetType) noexcept 
		{
			uint32_t index = NULL_INDEX_EX;
			if (!FreeList.empty())
			{
				index = FreeList.front();
				FreeList.pop();
				Assets[index] = std::move(assetType);
				RefCounts[index] = 0;
			}
			else
			{
				index = static_cast<uint32_t>(Assets.size());
				Assets.push_back(std::move(assetType));
				RefCounts.push_back(0);
			}
			return index;
		}

		__forceinline void IncreaseReferenceCount(const AssetHandle_EX& assetHandle) noexcept
		{
			RLS_ASSERT(!(assetHandle.Index > RefCounts.size()), "Index out of bounds - No asset corresponds to asset handle.");
			RefCounts[assetHandle.Index]++;
		}

		__forceinline void DecreaseReferenceCount(const AssetHandle_EX& assetHandle) noexcept
		{
			RLS_ASSERT(!(assetHandle.Index > RefCounts.size()), "Index out of bounds - No asset corresponds to asset handle.");
			RLS_ASSERT(!(RefCounts[assetHandle.Index] == 0), "Index out of bounds - No asset corresponds to asset handle.");
			RefCounts[assetHandle.Index]--;
		}

	public:
		std::vector<AssetType> Assets;
		std::vector<ReferenceCount> RefCounts;
		std::queue<uint32_t> FreeList;
	};

	class AssetManagerEx
	{
	private:
		template<typename>
		struct always_false : std::false_type {};

		template<typename T>
		struct AssetTypeTrait
		{
			static constexpr AssetType value = AssetType::Undefined;
		};

	public:
		

		template<typename AssetType>
		static [[nodiscard]] AssetType& GetAsset(const AssetHandle_EX& assetHandle) noexcept
		{
			RLS_ASSERT(assetHandle != NULL_HANDLE_EX, "Asset handle is not valid.");
			return GetStorage<AssetType>().GetAsset(assetHandle);
		}

		template<typename AssetType>
		static [[nodiscard]] AssetHandle_EX LoadFromFile(const std::string& filepath) noexcept
		{
			RLS_ASSERT(std::filesystem::exists(filepath), "Filepath is invalid.");
			if (IsLoaded(filepath))
			{
				return s_LoadedAssets[filepath];
			}
			else
			{
				AssetHandle_EX assetHandle = NULL_HANDLE_EX;
				assetHandle.Type = AssetTypeTrait<AssetType>::value;
				assetHandle.Uuid = CreateUUID();

				if (AssetRegistry::IsFilepathMapped(filepath))
				{
					assetHandle.Index = GetStorage<AssetType>().Add(std::move(Serializer::Deserialize<AssetType>(filepath)));
				}
				else
				{
					//The asset is neither loaded nor known. This means it is a freshly added asset that should 
					//both be loaded and mapped.
					assetHandle.Index = GetStorage<AssetType>().Add(std::move(AssetType::LoadFromFile(filepath)));
					AssetRegistry::MapUUIDToFilepath(assetHandle.Uuid, filepath);
				}

				s_LoadedAssets[filepath] = assetHandle;

				return assetHandle;
			}
		}

		static [[nodiscard]] bool IsLoaded(const std::string& filepath) noexcept;
	private:
		static void IncreaseReferenceCount(const AssetHandle_EX& assetHandle) noexcept;
		static void DecreaseReferenceCount(const AssetHandle_EX& assetHandle) noexcept;
		
		template<typename AssetType>
		static AssetStorage<AssetType>& GetStorage() noexcept
		{
			static_assert(always_false<AssetType>::value, "This operation is not supported by the type, or the type does not exist.");
		}
	private:
		struct AssetStorages
		{
			//AssetStorage<Material> MaterialStorage;
			AssetStorage<Texture2D> Texture2DStorage;
			//AssetStorage<Scene> SceneStorage;
		};

		static AssetStorages s_AssetStorages;
		
		static std::unordered_map<std::string, AssetHandle_EX> s_LoadedAssets;

		friend struct AssetHandle_EX;
	};

	template<>
	inline AssetStorage<Texture2D>& AssetManagerEx::GetStorage<Texture2D>() noexcept { return s_AssetStorages.Texture2DStorage; }

	template<>
	struct AssetManagerEx::AssetTypeTrait<Texture2D>
	{
		static constexpr AssetType value = AssetType::Texture2D;
	};
}