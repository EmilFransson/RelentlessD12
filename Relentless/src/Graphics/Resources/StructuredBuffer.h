#pragma once
#include "IResource.h"
namespace Relentless
{
	class StructuredBuffer : public IResource
	{
	public:
		explicit StructuredBuffer(uint32_t nrOfElements, size_t byteStride) noexcept;
		virtual ~StructuredBuffer() noexcept override final;

		[[nodiscard]] uint32_t GetFreeIndex() const noexcept;

		size_t m_SizeInBytes;
		uint32_t m_ByteStride;
		uint32_t m_Capacity;
		mutable uint32_t m_NrOfElements;
		DescriptorHandle m_NonVisibleHandle;
		DescriptorHandle m_VisibleHandles[3];
	};
}