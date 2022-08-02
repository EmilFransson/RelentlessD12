#pragma once
#include "DescriptorHeap.h"
namespace Relentless
{
	class MemoryManager
	{
	public:
		static MemoryManager& Get() noexcept;
		void Initialize() noexcept;
		[[nodiscard]] const DescriptorHandle CreateDescriptorHandle(DescriptorHandleType descriptorHandleType) noexcept;
		void DestroyDescriptorHandle(const DescriptorHandle& descriptorHandle) noexcept;
		void PerformDeferredDeletion() noexcept;
	private:
		MemoryManager() noexcept = default;
		~MemoryManager() noexcept = default;
	private:
		static MemoryManager s_Instance;
		std::unique_ptr<DescriptorHeap> m_pRTVDescriptorHeap;
		std::unique_ptr<DescriptorHeap> m_pShaderBindablesDescriptorHeapNV;
		std::unique_ptr<std::vector<DescriptorHandle>[]> m_pDeferredFreeLists;
	};
}