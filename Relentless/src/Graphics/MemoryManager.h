#pragma once
#include "Assets/AssetMeta.h"
#include "DescriptorHeap.h"
#include "Renderer/MasterRenderer.h"
#include "Resources/Texture.h"
#include "Resources/UploadBuffer.h"
#include "Utility/Common.h"
namespace Relentless
{
	constexpr size_t INVALID_CONSTANT_BUFFER_ID = static_cast<size_t>(-1);

	class MemoryManager
	{
	public:
		MemoryManager() noexcept = default;
		~MemoryManager() noexcept = default;
		void Initialize() noexcept;
		[[nodiscard]] const DescriptorHandle CreateDescriptorHandle(DescriptorHandleType descriptorHandleType) noexcept;
		void DestroyDescriptorHandle(const DescriptorHandle& descriptorHandle) noexcept;
		void DestroyResource(std::shared_ptr<IResource> pResource) noexcept;
		void PerformDeferredDeletion() noexcept;
		[[nodiscard]] constexpr const std::unique_ptr<UploadBuffer>& GetUploadBuffer() const { return m_pUploadBuffer; }
		[[nodiscard]] constexpr const std::unique_ptr<DescriptorHeap>& GetShaderBindableDescriptorHeap() const { return m_pShaderBindablesDescriptorHeap; }
		[[nodiscard]] constexpr const std::unique_ptr<DescriptorHeap>& GetCBVSRVUAVDescriptorHeap() const { return m_pShaderBindablesDescriptorHeapNV; }
		[[nodiscard]] constexpr const std::unique_ptr<DescriptorHeap>& GetRTVDescriptorHeap() const { return m_pRTVDescriptorHeap; }
		[[nodiscard]] constexpr const std::unique_ptr<DescriptorHeap>& GetDSVDescriptorHeap() const { return m_pDSVDescriptorHeap; }
		void SetDirtyMaterial(const AssetHandle& handle) noexcept;
		void UpdateDirtyMaterials();
	private:
		std::unique_ptr<UploadBuffer> m_pUploadBuffer;
		std::unique_ptr<DescriptorHeap> m_pRTVDescriptorHeap;
		std::unique_ptr<DescriptorHeap> m_pDSVDescriptorHeap;
		std::unique_ptr<DescriptorHeap> m_pShaderBindablesDescriptorHeapNV;
		std::unique_ptr<DescriptorHeap> m_pShaderBindablesDescriptorHeap;
		std::unique_ptr<std::vector<DescriptorHandle>[]> m_pDeferredFreeLists;
		std::unique_ptr<std::vector<std::shared_ptr<IResource>>[]> m_pDeferredFreeListsResources;

		std::unordered_map<UUID, std::pair<AssetHandle, uint32_t>> m_DirtyMaterials; // UUID -> Remaining updates map
	};
}