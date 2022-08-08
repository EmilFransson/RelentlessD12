#pragma once
#include "DescriptorHeap.h"
#include "Resources/Texture.h"
namespace Relentless
{
	class MemoryManager
	{
	public:
		static MemoryManager& Get() noexcept;
		void Initialize() noexcept;
		[[nodiscard]] const DescriptorHandle CreateDescriptorHandle(DescriptorHandleType descriptorHandleType) noexcept;
		void DestroyDescriptorHandle(const DescriptorHandle& descriptorHandle) noexcept;
		//void DestroyResource(Microsoft::WRL::ComPtr<ID3D12Resource> resource) noexcept;
		void DestroyResource(std::shared_ptr<IResource> pResource) noexcept;
		void PerformDeferredDeletion() noexcept;
	private:
		MemoryManager() noexcept = default;
		~MemoryManager() noexcept = default;
	private:
		static MemoryManager s_Instance;
		std::unique_ptr<DescriptorHeap> m_pRTVDescriptorHeap;
		std::unique_ptr<DescriptorHeap> m_pShaderBindablesDescriptorHeapNV;
		std::unique_ptr<std::vector<DescriptorHandle>[]> m_pDeferredFreeLists;
		//std::unique_ptr<std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>[]> m_pDeferredFreeListsResources;
		std::unique_ptr<std::vector<std::shared_ptr<IResource>>[]> m_pDeferredFreeListsResources;
	};
}