#pragma once
#include "IResource.h"
namespace Relentless
{
	class StructuredBuffer : public IResource
	{
	public:
		StructuredBuffer(const std::string& name, uint32_t nrOfElements, uint32_t byteStride) noexcept;
		void UploadData(void* ptrToData, size_t sizeInBytes, size_t offset) noexcept;
		[[nodiscard]] const DescriptorHandle& GetSRVDescriptorHandle() const noexcept;
		[[nodiscard]] size_t GetSizeInBytes() const noexcept;
		[[nodiscard]] uint32_t GetByteStride() const noexcept;
	private:
		size_t m_SizeInBytes = 0u;
		uint32_t m_ByteStride = 0u;
		uint32_t m_Capacity = 0u;
		mutable uint32_t m_NrOfElements = 0u;
		DescriptorHandle m_SRVDescriptorHandle;

		void* m_pData = nullptr;
	};

	class StructuredBufferSet
	{
	public:
		StructuredBufferSet(const std::string& name, uint32_t nrOfElements, uint32_t byteStride) noexcept;
		[[nodiscard]] uint32_t GetSRVDescriptorIndex(uint32_t bufferIndex) const noexcept;
		[[nodiscard]] StructuredBuffer& operator[](const uint32_t index) noexcept;
		[[nodiscard]] const StructuredBuffer& operator[](const uint32_t index) const noexcept;
		[[nodiscard]] StructuredBuffer& At(uint32_t bufferIndex) noexcept;
		[[nodiscard]] const StructuredBuffer& At(uint32_t bufferIndex) const noexcept;
	private:
		std::string m_Name = "?";
		std::vector<StructuredBuffer> m_StructuredBuffers;
	};
}