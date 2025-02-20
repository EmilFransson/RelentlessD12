#pragma once
#include "DescriptorHeap.h"
namespace Relentless
{
	class DescriptorManager
	{
	public:
		DescriptorManager(GraphicsDevice* pDevice) noexcept;
		~DescriptorManager() noexcept = default;
		[[nodiscard]] const DescriptorHandleEx CreateDescriptorHandle(DescriptorHandleTypeEx descriptorHandleType) noexcept;
		void DeferReleaseDescriptorHandle(const DescriptorHandleEx& descriptorHandle, const SyncPoint& syncPoint) noexcept;
		[[nodiscard]] DescriptorHeapEx* GetShaderBindableDescriptorHeap() const noexcept { return m_pShaderBindablesDescriptorHeap; }
		[[nodiscard]] DescriptorHeapEx* GetCBVSRVUAVDescriptorHeap() const noexcept { return m_pShaderBindablesDescriptorHeapNV; }
		[[nodiscard]] DescriptorHeapEx* GetRTVDescriptorHeap() const noexcept { return m_pRTVDescriptorHeap; }
		[[nodiscard]] DescriptorHeapEx* GetDSVDescriptorHeap() const noexcept { return m_pDSVDescriptorHeap; }
		[[nodiscard]] DescriptorHeapEx* GetSamplerDescriptorHeap() const noexcept { return m_pSamplerDescriptorHeap; }
	private:
		Ref<DescriptorHeapEx> m_pRTVDescriptorHeap = nullptr;
		Ref<DescriptorHeapEx> m_pDSVDescriptorHeap = nullptr;
		Ref<DescriptorHeapEx> m_pShaderBindablesDescriptorHeapNV = nullptr;
		Ref<DescriptorHeapEx> m_pShaderBindablesDescriptorHeap = nullptr;
		Ref<DescriptorHeapEx> m_pSamplerDescriptorHeap = nullptr;
	};
}