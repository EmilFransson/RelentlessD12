#pragma once
#include "Assets/AssetMeta.h"
#include "DescriptorHeap.h"
#include "Renderer/MasterRenderer.h"
#include "Resources/StructuredBuffer.h"
#include "Resources/Texture.h"
#include "Resources/UploadBuffer.h"
#include "Utility/Common.h"
namespace Relentless
{
	constexpr size_t INVALID_CONSTANT_BUFFER_ID = static_cast<size_t>(-1);

	class ConstantBuffer;
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
		[[nodiscard]] constexpr const std::unique_ptr<DescriptorHeap>& GetCBVSRVUAVDescriptorHeap() const { return m_pShaderBindablesDescriptorHeapNV; }
		[[nodiscard]] constexpr const std::unique_ptr<DescriptorHeap>& GetRTVDescriptorHeap() const { return m_pRTVDescriptorHeap; }
		[[nodiscard]] constexpr const std::unique_ptr<DescriptorHeap>& GetDSVDescriptorHeap() const { return m_pDSVDescriptorHeap; }
		[[nodiscard]] const size_t GetNrOfConstantBuffersInUse() const { return m_ConstantBuffers.size() - m_FreeConstantBufferHandles.size(); }
		void UpdateConstantBuffer(const ConstantBuffer& constantBuffer, void* pData) noexcept;
		void UpdateConstantBuffer(size_t id, void* pData) noexcept;
		void UpdateStructuredBuffer(const StructuredBuffer& structuredBuffer, void* pData, uint32_t index) noexcept;
		void UpdateStructuredBuffer(const StructuredBuffer& structuredBuffer, void* pData, uint32_t index, uint32_t frameIndex) noexcept;
		[[nodiscard]] size_t CreateConstantBuffer(uint32_t sizeInBytes) noexcept;
		[[nodiscard]] std::unique_ptr<ConstantBuffer>& GetConstantBuffer(size_t ID) noexcept { return m_ConstantBuffers[ID]; }
		[[nodiscard]] uint32_t GetCBDescriptorIndex(const size_t constantBufferHandle) noexcept;
		void FreeConstantBuffer(size_t cbHandle) noexcept;
		void SetDirtyMaterial(const AssetHandle& handle) noexcept;
		void UpdateDirtyMaterials();
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

		std::queue<size_t> m_FreeConstantBufferHandles;
		std::vector<std::unique_ptr<ConstantBuffer>> m_ConstantBuffers;
		std::unordered_map<UUID, std::pair<AssetHandle, uint32_t>> m_DirtyMaterials; // UUID -> Remaining updates map
	};
}