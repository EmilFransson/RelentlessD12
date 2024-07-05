#pragma once
#include "IResource.h"
namespace Relentless
{
	class VertexBuffer : public IResource
	{
	public:
		VertexBuffer(const std::string& name, uint32_t sizeBytes, uint32_t vertexCount) noexcept;
		virtual ~VertexBuffer() noexcept override final = default;
		[[nodiscard]] uint32_t GetSRVDescriptorHeapIndex() const { return m_SRVDescriptorHandle.Index; }
		[[nodiscard]] const uint32_t GetNrOfVertices() const noexcept { return m_VertexCount; }
		[[nodiscard]] uint32_t GetSizeInBytes() const { return m_SizeInBytes; }
		[[nodiscard]] void* Map(uint32_t offset, uint32_t sizeInBytes) noexcept;
		void Unmap() noexcept;
	private:
		DescriptorHandle m_SRVDescriptorHandle;
		uint32_t m_SizeInBytes = 0u;
		uint32_t m_VertexCount = 0u;
	};
}