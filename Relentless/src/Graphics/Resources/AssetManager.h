#pragma once
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "../MemoryManager.h"
namespace std
{
	template<>
	struct hash<UUID>
	{
		std::size_t operator()(const UUID& gUid) const
		{
			const uint64_t* half = reinterpret_cast<const uint64_t*>(&gUid);
			return half[0] ^ half[1];
		}
	};
}

namespace Relentless
{
	typedef UUID ResourceID;
	class AssetManager
	{
	public:
		AssetManager() noexcept = default;
		~AssetManager() noexcept = default;

		[[nodiscard]] static constexpr AssetManager& Get() noexcept { return s_instance; }

		[[nodiscard]] bool HasLoaded(const std::string& assetPath) const noexcept;

		template<typename AssetType>
		requires std::is_base_of_v<IResource, AssetType>
		const ResourceID& Load(const std::string& contextName, void* pSpecification) noexcept
		{
			if (!m_PathToResourceIDMap.contains(contextName))
			{
				UUID uuID;
				#if defined RLS_DEBUG
				RLS_ASSERT(::UuidCreate(&uuID) == RPC_S_OK, "Failed to generate UUID.");
				#else
				::UuidCreate(&uuID);
				#endif
				m_PathToResourceIDMap[contextName] = uuID;
				LoadInternal<AssetType>(uuID, pSpecification);
				MemoryManager::Get().GetUploadBuffer()->Upload();
			}
			return m_PathToResourceIDMap[contextName];
		}

		template<typename AssetType>
		requires std::is_base_of_v<IResource, AssetType>
		[[nodiscard]] AssetType* GetAsset(const ResourceID assetID) noexcept
		{
			RLS_ASSERT(m_Assets.contains(assetID), "Asset has not been loaded.");
			return static_cast<AssetType*>(m_Assets[assetID].get());
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
	private:
		static AssetManager s_instance;
		std::unordered_map<ResourceID, std::shared_ptr<IResource>> m_Assets;
		std::unordered_map<std::string, ResourceID> m_PathToResourceIDMap;
	};
}