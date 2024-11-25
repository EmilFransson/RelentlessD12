#pragma once
#include "graphics/DescriptorHeap.h"
#include "IResource.h"
namespace Relentless
{
	class ConstantBuffer2 : public IResource
	{
	public:
		ConstantBuffer2(const std::string& name, uint32_t sizeInBytes) noexcept;
		[[nodiscard]] const DescriptorHandle& GetCBVDescriptorHandle() const noexcept;
		[[nodiscard]] size_t GetSizeInBytes() const noexcept;
		void UploadData(const void* ptrToData, size_t sizeInBytes, size_t offset = 0);
	private:
		uint32_t m_SizeInBytes;
		DescriptorHandle m_CBVDescriptorHandle;
		void* m_pData = nullptr;
	};

	class ConstantBufferSet
	{
	public:
		explicit ConstantBufferSet(const std::string& name, uint32_t sizeInBytes) noexcept;
		[[nodiscard]] uint32_t GetCBVDescriptorIndex(uint32_t bufferIndex) const noexcept;
		const ConstantBuffer2& operator[](uint32_t bufferIndex) const noexcept;
		ConstantBuffer2& operator[](uint32_t bufferIndex) noexcept;
		[[nodiscard]] ConstantBuffer2& At(uint32_t bufferIndex) noexcept;
		[[nodiscard]] const ConstantBuffer2& At(uint32_t bufferIndex) const noexcept;
	private:
		std::string m_Name = "?";
		uint32_t m_SizeInBytes = 0u;
		std::vector<ConstantBuffer2> m_ConstantBuffers;
	};
}