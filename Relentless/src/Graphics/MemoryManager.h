#pragma once
#include "DescriptorHeap.h"
#include "Resources/Texture.h"
#include "Resources/UploadBuffer.h"
namespace Relentless
{
	class MemoryManager
	{
	public:
		static MemoryManager& Get() noexcept;
		void Initialize() noexcept;
		[[nodiscard]] const DescriptorHandle CreateDescriptorHandle(DescriptorHandleType descriptorHandleType) noexcept;
		void DestroyDescriptorHandle(const DescriptorHandle& descriptorHandle) noexcept;
		void DestroyResource(std::shared_ptr<IResource> pResource) noexcept;
		void PerformDeferredDeletion() noexcept;
		[[nodiscard]] constexpr const std::unique_ptr<UploadBuffer>& GetUploadBuffer() const { return m_pUploadBuffer; }
		[[nodiscard]] constexpr const std::unique_ptr<DescriptorHeap>& GetShaderBindableDescriptorHeap() const { return m_pShaderBindablesDescriptorHeap; }
	private:
		MemoryManager() noexcept = default;
		~MemoryManager() noexcept = default;
	private:
		static MemoryManager s_Instance;
		std::unique_ptr<UploadBuffer> m_pUploadBuffer;
		std::unique_ptr<DescriptorHeap> m_pRTVDescriptorHeap;
		std::unique_ptr<DescriptorHeap> m_pDSVDescriptorHeap;
		std::unique_ptr<DescriptorHeap> m_pShaderBindablesDescriptorHeapNV;
		std::unique_ptr<DescriptorHeap> m_pShaderBindablesDescriptorHeap;
		std::unique_ptr<std::vector<DescriptorHandle>[]> m_pDeferredFreeLists;
		std::unique_ptr<std::vector<std::shared_ptr<IResource>>[]> m_pDeferredFreeListsResources;
	};
}