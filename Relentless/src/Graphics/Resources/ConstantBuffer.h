#pragma once
#include "IResource.h"
#include "../MemoryManager.h"
namespace Relentless
{
	class ConstantBuffer : public IResource
	{
	public:
		explicit ConstantBuffer(size_t sizeInBytes) noexcept;
		virtual ~ConstantBuffer() noexcept override final = default;

		size_t m_SizeInBytes;
		DescriptorHandle m_NonVisibleHandle;
		DescriptorHandle m_VisibleHandles[3];
	};
}