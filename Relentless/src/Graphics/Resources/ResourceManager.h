#pragma once
#include "ResourceMeta.h"
#include "ConstantBufferSet.h"
#include "StructuredBufferSet.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "../../vendor/includes/DenseHashMap/dense_hash_map.hpp"
namespace Relentless
{
	template<typename ResourceType>
	struct ResourcePool
	{
	public:
		using Pool = jg::dense_hash_map<ResourceHandle, std::shared_ptr<ResourceType>>;
		
		[[nodiscard]] std::shared_ptr<ResourceType> Get(ResourceHandle handle) noexcept
		{
			const std::lock_guard<std::mutex> guard(m_AccessMutex);

			if (m_Resources.contains(handle))
				return m_Resources[handle];
			else
				return nullptr;
		}

		[[nodiscard]] const std::shared_ptr<ResourceType>& GetRef(ResourceHandle handle) noexcept
		{
			const std::lock_guard<std::mutex> guard(m_AccessMutex);

			if (m_Resources.contains(handle))
				return m_Resources[handle];
			else
				return nullptr;
		}

		[[nodiscard]] ResourceHandle Add(const std::shared_ptr<ResourceType>& pResource) noexcept
		{
			const std::lock_guard<std::mutex> guard(m_AccessMutex);

			RLS_ASSERT(pResource, "[ResourcePool]: Resource to add is invalid.");

			const ResourceHandle handle = RequestHandle();
			m_Resources[handle] = std::move(pResource);
			return handle;
		}

		[[nodiscard]] bool Exists(ResourceHandle resourceHandle) const
		{
			const std::lock_guard<std::mutex> guard(m_AccessMutex);

			return m_Resources.contains(resourceHandle);
		}

	private:
		[[nodiscard]] ResourceHandle RequestHandle() noexcept
		{
			const ResourceHandle currentHandle = m_NextHandle;
			m_NextHandle++;

			return currentHandle;
		}
	private:
		Pool m_Resources;
		ResourceHandle m_NextHandle = 0u;
		std::mutex m_AccessMutex;
	};

	class ResourceManager
	{
	public:
		//Constant Buffer:
		[[nodiscard]] ResourceHandle CreateConstantBufferSet(const std::string& name, uint32_t sizeInBytes) noexcept;
		bool UploadConstantBufferData(ResourceHandle handle, void* ptrToData, uint32_t sizeInBytes, uint32_t bufferIndex, uint32_t offset = 0u) noexcept;
		[[nodiscard]] uint32_t GetConstantBufferViewDescriptorIndex(ResourceHandle handle, uint32_t bufferIndex) noexcept;

		//Stuctured Buffer:
		[[nodiscard]] ResourceHandle CreateStructuredBufferSet(const std::string& name, uint32_t nrOfElements, uint32_t byteStride) noexcept;
		bool UploadStructuredBufferData(ResourceHandle handle, void* ptrToData, uint32_t sizeInBytes, uint32_t bufferIndex, uint32_t offset) noexcept;
		[[nodiscard]] uint32_t GetStructuredBufferShaderResourceViewDescriptorIndex(ResourceHandle handle, uint32_t bufferIndex) noexcept;

		//Vertex Buffer:
		[[nodiscard]] ResourceHandle CreateVertexBuffer(const std::string& name, uint32_t sizeInBytes, uint32_t vertexCount) noexcept;
		[[nodiscard]] void* LockVertexBuffer(ResourceHandle handle, uint32_t offset, uint32_t sizeInBytes) noexcept;
		void UnlockVertexBuffer(ResourceHandle handle) noexcept;
		[[nodiscard]] uint32_t GetVertexBufferShaderResourceViewDescriptorIndex(ResourceHandle handle) noexcept;
		[[nodiscard]] std::shared_ptr<VertexBuffer> GetVertexBuffer(ResourceHandle handle);

		//Index Buffer:
		[[nodiscard]] ResourceHandle CreateIndexBuffer(const std::string& name, uint32_t sizeInBytes, uint32_t indexCount) noexcept;
		[[nodiscard]] void* LockIndexBuffer(ResourceHandle handle, uint32_t offset, uint32_t sizeInBytes) noexcept;
		void UnlockIndexBuffer(ResourceHandle handle) noexcept;
		[[nodiscard]] uint32_t GetIndexBufferShaderResourceViewDescriptorIndex(ResourceHandle handle) noexcept;
		[[nodiscard]] std::shared_ptr<IndexBuffer> GetIndexBuffer(ResourceHandle handle);

	private:
		ResourcePool<ConstantBufferSet> m_ConstantBufferSetPool;
		ResourcePool<StructuredBufferSet> m_StructuredBufferSetPool;
		ResourcePool<VertexBuffer> m_VertexBufferPool;
		ResourcePool<IndexBuffer> m_IndexBufferPool;
	};
}