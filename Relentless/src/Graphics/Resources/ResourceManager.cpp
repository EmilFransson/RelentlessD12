#include "ResourceManager.h"
#include "Utility/Common.h"

namespace Relentless
{
	ResourceHandle ResourceManager::CreateConstantBufferSet(const std::string& name, uint32_t sizeInBytes) noexcept
	{
		return m_ConstantBufferSetPool.Add(std::make_shared<ConstantBufferSet>(name, sizeInBytes));
	}

	bool ResourceManager::UploadConstantBufferData(ResourceHandle handle, void* ptrToData, uint32_t sizeInBytes, uint32_t bufferIndex, uint32_t offset) noexcept
	{
		const std::shared_ptr<ConstantBufferSet>& ref = m_ConstantBufferSetPool.GetRef(handle);
		if (!ref)
			return false;

		ref->At(bufferIndex).UploadData(ptrToData, sizeInBytes, offset);
		return true;
	}

	uint32_t ResourceManager::GetConstantBufferViewDescriptorIndex(ResourceHandle handle, uint32_t bufferIndex) noexcept
	{
		const std::shared_ptr<ConstantBufferSet>& ref = m_ConstantBufferSetPool.GetRef(handle);
		if (!ref)
			return NULL_RESOURCE_HANDLE;

		return ref->GetCBVDescriptorIndex(bufferIndex);
	}

	ResourceHandle ResourceManager::CreateStructuredBufferSet(const std::string& name, uint32_t nrOfElements, uint32_t byteStride) noexcept
	{
		return m_StructuredBufferSetPool.Add(std::make_shared<StructuredBufferSet>(name, nrOfElements, byteStride));
	}

	bool ResourceManager::UploadStructuredBufferData(ResourceHandle handle, void* ptrToData, uint32_t sizeInBytes, uint32_t bufferIndex, uint32_t offset) noexcept
	{
		const std::shared_ptr<StructuredBufferSet>& ref = m_StructuredBufferSetPool.GetRef(handle);
		if (!ref)
			return false;

		ref->At(bufferIndex).UploadData(ptrToData, sizeInBytes, offset);
		return true;
	}

	uint32_t ResourceManager::GetStructuredBufferShaderResourceViewDescriptorIndex(ResourceHandle handle, uint32_t bufferIndex) noexcept
	{
		const std::shared_ptr<StructuredBufferSet>& ref = m_StructuredBufferSetPool.GetRef(handle);
		if (!ref)
			return NULL_RESOURCE_HANDLE;

		return ref->GetSRVDescriptorIndex(bufferIndex);
	}

	ResourceHandle ResourceManager::CreateVertexBuffer(const std::string& name, uint32_t sizeInBytes, uint32_t vertexCount) noexcept
	{
		return m_VertexBufferPool.Add(std::make_shared<VertexBuffer>(name, sizeInBytes, vertexCount));
	}

	void* ResourceManager::LockVertexBuffer(ResourceHandle handle, uint32_t offset, uint32_t sizeInBytes) noexcept
	{
		const std::shared_ptr<VertexBuffer>& ref = m_VertexBufferPool.GetRef(handle);
		if (!ref)
			return nullptr;

		return ref->Map(offset, sizeInBytes);
	}

	void ResourceManager::UnlockVertexBuffer(ResourceHandle handle) noexcept
	{
		const std::shared_ptr<VertexBuffer>& ref = m_VertexBufferPool.GetRef(handle);
		if (ref)
			ref->Unmap();
	}

	uint32_t ResourceManager::GetVertexBufferShaderResourceViewDescriptorIndex(ResourceHandle handle) noexcept
	{
		const std::shared_ptr<VertexBuffer> ref = m_VertexBufferPool.Get(handle);
		if (!ref)
			return NULL_RESOURCE_HANDLE;

		return ref->GetSRVDescriptorHeapIndex();
	}

	std::shared_ptr<VertexBuffer> ResourceManager::GetVertexBuffer(ResourceHandle handle)
	{
		std::shared_ptr<VertexBuffer> pVertexBuffer = m_VertexBufferPool.Get(handle);
		if (!pVertexBuffer)
			return nullptr;

		return pVertexBuffer;
	}

	ResourceHandle ResourceManager::CreateIndexBuffer(const std::string& name, uint32_t sizeInBytes, uint32_t indexCount) noexcept
	{
		return m_IndexBufferPool.Add(std::make_shared<IndexBuffer>(name, sizeInBytes, indexCount));
	}

	void* ResourceManager::LockIndexBuffer(ResourceHandle handle, uint32_t offset, uint32_t sizeInBytes) noexcept
	{
		const std::shared_ptr<IndexBuffer>& ref = m_IndexBufferPool.GetRef(handle);
		if (!ref)
			return nullptr;

		return ref->Map(offset, sizeInBytes);
	}

	void ResourceManager::UnlockIndexBuffer(ResourceHandle handle) noexcept
	{
		const std::shared_ptr<IndexBuffer>& ref = m_IndexBufferPool.GetRef(handle);
		if (ref)
			ref->Unmap();
	}

	uint32_t ResourceManager::GetIndexBufferShaderResourceViewDescriptorIndex(ResourceHandle handle) noexcept
	{
		const std::shared_ptr<IndexBuffer> ref = m_IndexBufferPool.Get(handle);
		if (!ref)
			return NULL_RESOURCE_HANDLE;

		return ref->GetSRVDescriptorHeapIndex();
	}

	std::shared_ptr<IndexBuffer> ResourceManager::GetIndexBuffer(ResourceHandle handle)
	{
		std::shared_ptr<IndexBuffer> pIndexBuffer = m_IndexBufferPool.Get(handle);
		if (!pIndexBuffer)
			return nullptr;

		return pIndexBuffer;
	}
}