#pragma once
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "../MemoryManager.h"
#include "Material.h"
//namespace std
//{
//	template<>
//	struct hash<UUID>
//	{
//		std::size_t operator()(const UUID& gUid) const
//		{
//			const uint64_t* half = reinterpret_cast<const uint64_t*>(&gUid);
//			return half[0] ^ half[1];
//		}
//	};
//}

namespace Relentless
{
	inline static std::mutex g_LoadMutex;

	class AssetManager
	{
	public:
		AssetManager() noexcept = default;
		~AssetManager() noexcept = default;

		[[nodiscard]] static constexpr AssetManager& Get() noexcept { return s_instance; }

		[[nodiscard]] bool HasLoaded(const std::string& assetPath) const noexcept;

		[[nodiscard]] MaterialManager& GetMaterialManager() noexcept { return m_MaterialManager; }

		template<typename AssetType>
		void Upload(const ResourceID& resourceID) noexcept
		{
			if constexpr (std::is_same_v<AssetType, Material>)
			{
				m_MaterialManager.Upload(resourceID);
			}
		}

		template<typename AssetType>
		__forceinline [[nodiscard]] const ResourceID Create(const std::string& assetName, const AssetType& assetType = AssetType())
		{
			if constexpr (std::is_same_v<AssetType, Material>)
			{
				return m_MaterialManager.Create(assetName, assetType);
			}
		}

		template<typename AssetType>
		requires std::is_base_of_v<IResource, AssetType>
		const ResourceID& Load(const std::string& contextName, void* pSpecification = nullptr) noexcept
		{
			const std::lock_guard<std::mutex> lock(g_LoadMutex);
			if (!m_PathToResourceIDMap.contains(contextName))
			{
				UUID uuID;
				#if defined RLS_DEBUG
				RLS_ASSERT(::UuidCreate(&uuID) == RPC_S_OK, "Failed to generate UUID.");
				#else
				::UuidCreate(&uuID);
				#endif
				m_PathToResourceIDMap[contextName] = uuID;
				if (pSpecification)
					LoadInternal<AssetType>(uuID, pSpecification);
				else
					LoadInternal<AssetType>(uuID, (void*)contextName.c_str());

				MemoryManager::Get().GetUploadBuffer()->Upload();
			}
			return m_PathToResourceIDMap[contextName];
		}

		template<typename AssetType>
		[[nodiscard]] AssetType& Get(const ResourceID assetID) noexcept
		{
			if constexpr (std::is_same_v<AssetType, Material>)
			{
				return m_MaterialManager.Get(assetID);
			}
		}

		template<typename AssetType>
		requires std::is_base_of_v<IResource, AssetType>
		[[nodiscard]] AssetType* GetAsset(const ResourceID& assetID) noexcept
		{
			RLS_ASSERT(m_Assets.contains(assetID), "Asset has not been loaded.");
			return static_cast<AssetType*>(m_Assets[assetID].get());
		}

		[[nodiscard]] bool Exists(const ResourceID& uuID) const noexcept
		{
			return m_Assets.contains(uuID);
		}

		[[nodiscard]] const std::string& GetAssetName(const ResourceID& uuID) const noexcept
		{
			RLS_ASSERT(m_Assets.contains(uuID), "Resource does not exist.");
			const std::string* toReturn{nullptr};
			for (const auto& [name, ID] : m_PathToResourceIDMap)
			{
				if (ID == uuID)
				{
					toReturn = &name;
					break;
				}
			}
			return *toReturn;
		}
	private:
		template<typename AssetType>
		requires std::is_base_of_v<IResource, AssetType>
		void LoadInternal(const ResourceID& uuID, void* pSpecification)
		{
			RLS_ASSERT(false, "Resource not yet supported.");
		}

		template<>
		void LoadInternal<VertexBuffer>(const ResourceID& uuID, void* pSpecification)
		{
			VertexBuffer::Specification* pVertexBufferSpecification = static_cast<VertexBuffer::Specification*>(pSpecification);
			m_Assets[uuID] = std::move(std::make_shared<VertexBuffer>(pVertexBufferSpecification));
		}

		template<>
		void LoadInternal<IndexBuffer>(const ResourceID& uuID, void* pSpecification)
		{
			IndexBuffer::Specification* pIndexBufferSpecification = static_cast<IndexBuffer::Specification*>(pSpecification);
			m_Assets[uuID] = std::move(std::make_shared<IndexBuffer>(pIndexBufferSpecification));
		}

		template<>
		void LoadInternal<Texture2D>(const ResourceID& uuID, void* fileName)
		{
			RLS_ASSERT(fileName, "FileName is NULL");
			m_Assets[uuID] = std::move(std::make_shared<Texture2D>((char*)fileName));
		}

	private:
		static AssetManager s_instance;
		std::unordered_map<ResourceID, std::shared_ptr<IResource>> m_Assets;
		std::unordered_map<std::string, ResourceID> m_PathToResourceIDMap;
		MaterialManager m_MaterialManager;
	};
}