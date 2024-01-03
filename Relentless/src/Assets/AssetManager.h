#pragma once
#include "AssetMeta.h"
#include "AssetRegistry.h"
#include "Graphics/Resources/Texture.h"
#include "Graphics/Resources/Material.h"
#include "Importer.h"
#include "ImportSettings.h"
#include "Mesh/Mesh.h"
#include "Serializer.h"
#include "Utility/Common.h"

namespace Relentless
{
	typedef uint32_t ReferenceCount;

	template<typename T>
	struct AssetTypeTrait
	{
		static constexpr AssetType value = AssetType::Undefined;
	};

	template<>
	struct AssetTypeTrait<Texture2D>
	{
		static constexpr AssetType value = AssetType::Texture2D;
	};

	template<>
	struct AssetTypeTrait<Mesh>
	{
		static constexpr AssetType value = AssetType::Mesh;
	};

	template<>
	struct AssetTypeTrait<Material>
	{
		static constexpr AssetType value = AssetType::Material;
	};

	template<typename AssetType>
	struct AssetStorage
	{
		__forceinline [[nodiscard]] AssetType& GetAsset(const AssetHandle& assetHandle) noexcept
		{
			RLS_ASSERT(assetHandle.Type == AssetTypeTrait<AssetType>::value, "Asset type mismatch detected.");
			
			const uint32_t physicalIndex = SparseArray[assetHandle.Index];
			RLS_ASSERT(!(physicalIndex > Assets.size()), "Index out of bounds - No asset corresponds to asset handle.");

			return Assets[physicalIndex];
		}

		__forceinline [[nodiscard]] uint32_t Add(AssetType assetType) noexcept 
		{
			uint32_t sparseIndex = NULL_INDEX;
			const uint32_t physicalIndex = static_cast<uint32_t>(Assets.size());

			if (!FreeList.empty())
			{
				sparseIndex = FreeList.front();
				FreeList.pop();

				SparseArray[sparseIndex] = physicalIndex;
			}
			else
			{
				sparseIndex = static_cast<uint32_t>(Assets.size());
				SparseArray.push_back(sparseIndex);
			}

			Assets.push_back(std::move(assetType));
			ReverseLookup.push_back(sparseIndex);

			if (RefCounts.size() > physicalIndex)
			{
				RefCounts[physicalIndex] = 0u;
			}
			else
			{
				RefCounts.push_back(0u);
			}

			return sparseIndex;
		}

		__forceinline void Destroy(const AssetHandle& handle) noexcept
		{
			RLS_ASSERT(handle.Type == AssetTypeTrait<AssetType>::value, "Asset type mismatch detected.");

			const uint32_t sparseIndex = handle.Index;
			const uint32_t assetIndex = SparseArray[sparseIndex];

			RLS_ASSERT(assetIndex < Assets.size(), "Asset index is invalid.");
			if (assetIndex >= Assets.size())
				return;

			const uint32_t assetsBackIndex = static_cast<uint32_t>(Assets.size() - 1);
			if (assetIndex != assetsBackIndex)
			{
				std::swap(Assets[assetIndex], Assets[assetsBackIndex]);

				const uint32_t swappedSparseIndex = ReverseLookup[assetsBackIndex];
				SparseArray[swappedSparseIndex] = assetIndex;
				ReverseLookup[assetIndex] = swappedSparseIndex;
			}

			Assets.pop_back();
			ReverseLookup.pop_back();

			SparseArray[sparseIndex] = std::numeric_limits<uint32_t>::max();
			FreeList.push(sparseIndex);
		}

		__forceinline void IncreaseReferenceCount(const AssetHandle& assetHandle) noexcept
		{
			RLS_ASSERT(assetHandle.Type == AssetTypeTrait<AssetType>::value, "Asset type mismatch detected.");
			
			const uint32_t physicalIndex = SparseArray[assetHandle.Index];
			RLS_ASSERT(physicalIndex < RefCounts.size(), "Index out of bounds - No asset corresponds to asset handle.");

			RefCounts[physicalIndex]++;
		}

		__forceinline void DecreaseReferenceCount(const AssetHandle& assetHandle) noexcept
		{
			RLS_ASSERT(assetHandle.Type == AssetTypeTrait<AssetType>::value, "Asset type mismatch detected.");
			
			const uint32_t physicalIndex = SparseArray[assetHandle.Index];

			if (physicalIndex < RefCounts.size())
			{
				RLS_ASSERT(RefCounts[physicalIndex] > 0, "Reference count is already at 0.");
				RefCounts[physicalIndex]--;
			}
		}

	public:
		std::vector<uint32_t> SparseArray;
		std::vector<uint32_t> ReverseLookup;
		std::vector<AssetType> Assets;
		std::vector<ReferenceCount> RefCounts;
		std::queue<uint32_t> FreeList;
	};

	class AssetManager
	{
	private:
		template<typename>
		struct always_false : std::false_type {};
	public:
		static void Initialize() noexcept;

		template<typename AssetType>
		static [[nodiscard]] AssetType& Get(const AssetHandle& assetHandle) noexcept
		{
			RLS_ASSERT(assetHandle != NULL_HANDLE, "Asset handle is not valid.");
			return GetStorage<AssetType>().GetAsset(assetHandle);
		}

		template<typename AssetType>
		static [[nodiscard]] AssetType& GetFromPath(const std::filesystem::path& fullPath) noexcept
		{
			RLS_ASSERT(s_LoadedAssets.contains(fullPath.string()), "Path to asset is invalid.");
			return GetStorage<AssetType>().GetAsset(s_LoadedAssets[fullPath.string()]);
		}

		static [[nodiscard]] AssetHandle GetHandleByPath(const std::filesystem::path& fullPath) noexcept
		{
			RLS_ASSERT(s_LoadedAssets.contains(fullPath.string()), "Path to asset is invalid.");
			return s_LoadedAssets2[s_LoadedAssets[fullPath.string()]];
		}

		template<typename AssetType>
		[[nodiscard]] static typename std::enable_if<
			std::conjunction<
			std::negation<std::is_same<AssetType, Mesh>>, 
			std::negation<std::is_same<AssetType, Material>>>::value, AssetHandle>::type
		LoadFromFile(const std::string& filepath, const typename AssetTypeInfo<AssetType>::Settings& importSettings = {}, const UUID& uuid = CreateUUID()) noexcept
		{
			RLS_ASSERT(std::filesystem::exists(filepath), "Filepath is invalid.");
			if (IsLoaded(filepath))
			{
				return s_LoadedAssets2[s_LoadedAssets[filepath]];
			}
			else
			{
				uint32_t handleIndex = NULL_INDEX;

				if (AssetRegistry::IsFilepathMapped(filepath))
				{
					//handleIndex = GetStorage<AssetType>().Add(std::move(Serializer::Deserialize<AssetType>(filepath)));
				}
				else
				{
					//The asset is neither loaded nor known. This means it is a freshly added asset that should 
					//both be loaded and mapped.
					handleIndex = GetStorage<AssetType>().Add(std::move(Importer::Import<AssetType>(filepath, importSettings)));

					AssetRegistry::MapAssetToFilepath({ uuid, AssetTypeTrait<AssetType>::value }, filepath);
				}

				s_LoadedAssets[filepath] = uuid;
				auto [it, inserted] = s_LoadedAssets2.emplace
				(
					std::piecewise_construct,
					std::forward_as_tuple(uuid),
					std::forward_as_tuple(AssetTypeTrait<AssetType>::value, uuid, handleIndex)
				);

				return it->second;
			}
		}

		template<typename AssetType>
		[[nodiscard]] static typename std::enable_if<std::is_same<AssetType, Mesh>::value, void>::type
			LoadFromFile(const std::string& filepath, const typename AssetTypeInfo<AssetType>::Settings& importSettings = {}) noexcept
		{
			Importer::Import<Mesh>(filepath, importSettings);
		}

		template<typename AssetType>
		[[nodiscard]] static typename std::enable_if<std::disjunction<
		std::is_same<AssetType, Material>,
		std::is_same<AssetType, Mesh>
		>::value, AssetHandle>::type
		CreateNew(const UUID& uuid = CreateUUID(), const std::string& pathToFile = "") noexcept
		{
			const uint32_t index = GetStorage<AssetType>().Add(AssetType());

			auto [it, inserted] = s_LoadedAssets2.emplace
			(
				std::piecewise_construct,
				std::forward_as_tuple(uuid),
				std::forward_as_tuple(AssetTypeTrait<AssetType>::value, uuid, index)
			);
			RLS_ASSERT(inserted, "Failed to insert new handle to asset.");

			if (!pathToFile.empty())
			{
				s_LoadedAssets[pathToFile] = uuid;
			}

			return it->second;
		}

		template<typename AssetType>
		[[nodiscard]] static typename std::enable_if<std::is_same<AssetType, Texture2D>::value, AssetHandle>::type
			CreateNew(const Texture2DSpecification& specification, const UUID& uuid = CreateUUID(), const std::string& pathToFile = "") noexcept
		{
			const uint32_t index = GetStorage<AssetType>().Add(AssetType(specification));

			auto [it, inserted] = s_LoadedAssets2.emplace
			(
				std::piecewise_construct,
				std::forward_as_tuple(uuid),
				std::forward_as_tuple(AssetTypeTrait<AssetType>::value, uuid, index)
			);
			RLS_ASSERT(inserted, "Failed to insert new handle to asset.");

			if (!pathToFile.empty())
			{
				s_LoadedAssets[pathToFile] = uuid;
			}

			return it->second;
		}

		template<typename AssetType>
		static void Destroy(const AssetHandle& handle) noexcept
		{
			GetStorage<AssetType>().Destroy(handle);
		}

		static [[nodiscard]] bool IsLoaded(const std::string& filepath) noexcept;
		static [[nodiscard]] const AssetHandle& GetDefaultMaterialHandle() noexcept;
		static void OnFileMoved(const AssetHandle& handle, const std::string& newFilepath) noexcept;
		static void Link(const std::string& path, const UUID& uuid) noexcept;
	private:
		static void IncreaseReferenceCount(const AssetHandle& assetHandle) noexcept;
		static void DecreaseReferenceCount(const AssetHandle& assetHandle) noexcept;
		
		template<typename AssetType>
		static AssetStorage<AssetType>& GetStorage() noexcept
		{
			static_assert(always_false<AssetType>::value, "This operation is not supported by the type, or the type does not exist.");
		}
	private:
		struct AssetStorages
		{
			AssetStorage<Mesh> MeshStorage;
			AssetStorage<Texture2D> Texture2DStorage;
			AssetStorage<Material> MaterialStorage;
		};

		static AssetStorages s_AssetStorages;
		
		static std::unordered_map<std::string, UUID> s_LoadedAssets;
		static std::unordered_map<UUID, AssetHandle> s_LoadedAssets2;

		friend struct AssetHandle;

		static AssetHandle m_DefaultMaterialHandle;
	};

	template<>
	inline AssetStorage<Texture2D>& AssetManager::GetStorage<Texture2D>() noexcept { return s_AssetStorages.Texture2DStorage; }

	template<>
	inline AssetStorage<Mesh>& AssetManager::GetStorage<Mesh>() noexcept { return s_AssetStorages.MeshStorage; }

	template<>
	inline AssetStorage<Material>& AssetManager::GetStorage<Material>() noexcept { return s_AssetStorages.MaterialStorage; }

}