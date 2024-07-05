#pragma once
#include "IResource.h"
namespace Relentless
{
	class IndexBuffer : public IResource
	{
	public:
		IndexBuffer(const std::string& name, uint32_t sizeInBytes, uint32_t indexCount) noexcept;
		virtual ~IndexBuffer() noexcept override final = default;
		[[nodiscard]] uint32_t GetSRVDescriptorHeapIndex() const { return m_SRVDescriptorHandle.Index; }
		[[nodiscard]] uint32_t GetSizeInBytes() const { return m_SizeInBytes; }
		[[nodiscard]] const uint32_t GetNrOfIndices() const noexcept { return m_IndexCount; }
		[[nodiscard]] void* Map(uint32_t offset, uint32_t sizeInBytes) noexcept;
		void Unmap() noexcept;
	private:
		DescriptorHandle m_SRVDescriptorHandle;
		uint32_t m_SizeInBytes = 0u;
		uint32_t m_IndexCount = 0u;
	};
}