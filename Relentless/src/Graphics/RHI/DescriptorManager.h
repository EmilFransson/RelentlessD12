#pragma once
#include "DescriptorHeap.h"
namespace Relentless
{
	class DescriptorManager
	{
	public:
		DescriptorManager(GraphicsDevice* pDevice) noexcept;
		~DescriptorManager() noexcept = default;
		[[nodiscard]] const DescriptorHandle CreateDescriptorHandle(DescriptorHandleType descriptorHandleType) noexcept;
		[[nodiscard]] const std::vector<DescriptorHandle> CreateDescriptorHandleBlock(DescriptorHandleType descriptorHandleType, uint32 blockSize) noexcept;
		void DeferReleaseDescriptorHandle(const DescriptorHandle& descriptorHandle, const SyncPoint& syncPoint) noexcept;
		[[nodiscard]] DescriptorHeap* GetShaderBindableDescriptorHeap() const noexcept { return m_pShaderBindablesDescriptorHeap; }
		[[nodiscard]] DescriptorHeap* GetCBVSRVUAVDescriptorHeap() const noexcept { return m_pShaderBindablesDescriptorHeapNV; }
		[[nodiscard]] DescriptorHeap* GetRTVDescriptorHeap() const noexcept { return m_pRTVDescriptorHeap; }
		[[nodiscard]] DescriptorHeap* GetDSVDescriptorHeap() const noexcept { return m_pDSVDescriptorHeap; }
		[[nodiscard]] DescriptorHeap* GetSamplerDescriptorHeap() const noexcept { return m_pSamplerDescriptorHeap; }
	private:
		Ref<DescriptorHeap> m_pRTVDescriptorHeap = nullptr;
		Ref<DescriptorHeap> m_pDSVDescriptorHeap = nullptr;
		Ref<DescriptorHeap> m_pShaderBindablesDescriptorHeapNV = nullptr;
		Ref<DescriptorHeap> m_pShaderBindablesDescriptorHeap = nullptr;
		Ref<DescriptorHeap> m_pSamplerDescriptorHeap = nullptr;
	};
}