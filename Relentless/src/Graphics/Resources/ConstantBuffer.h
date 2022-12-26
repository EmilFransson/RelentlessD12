#pragma once
#include "IResource.h"
#include "../MemoryManager.h"
namespace Relentless
{
	class ConstantBuffer : public IResource
	{
	public:
		explicit ConstantBuffer(uint32_t sizeInBytes) noexcept;
		virtual ~ConstantBuffer() noexcept override final = default;

		uint32_t m_SizeInBytes;
		DescriptorHandle m_NonVisibleHandle;
		DescriptorHandle m_VisibleHandles[3];
	};
}