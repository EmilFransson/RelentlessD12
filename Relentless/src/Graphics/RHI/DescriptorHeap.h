#pragma once 
#include "DeviceResource.h"
#include "Fence.h"

namespace Relentless
{
	enum class DescriptorHandleTypeEx { RTV = 0u, DSV, SRV, CBV, UAV, SRV_NV, CBV_NV, UAV_NV, None }; //NV = Non shader visible

	struct DescriptorHandleEx
	{
		D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE GPUHandle;
		DescriptorHandleTypeEx Type = DescriptorHandleTypeEx::None; //Set by descriptor manager!
		uint32_t Index;
#if defined(RLS_DEBUG)
		friend class DescriptorHeapEx;
		DescriptorHeapEx* pDebugInterface = nullptr;
#endif
	};

	class DescriptorHeapEx : public DeviceObject
	{
	public:
		static constexpr uint32 INVALID_DESCRIPTOR_INDEX = 0xFFFFFFFF;

		DescriptorHeapEx(GraphicsDevice* pParent, const D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType, const uint32_t capacity, const bool isShaderVisible) noexcept;
		virtual ~DescriptorHeapEx() noexcept override = default;
		[[nodiscard]] DescriptorHandleEx AllocateDescriptor() noexcept;
		[[nodiscard]] std::vector<DescriptorHandleEx> AllocateDescriptorBlock(uint32 blockSize) noexcept;
		void FreeDescriptor(const DescriptorHandleEx& descriptorHandle, const SyncPoint& syncPoint) noexcept;
		[[nodiscard]] ID3D12DescriptorHeap* GetDescriptorHeapInterface() const noexcept { return m_pDescriptorHeap; }
		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCPUStartHandle() const  noexcept { return m_CpuHandleStart; }
		[[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetGPUStartHandle() const  noexcept { return m_GpuHandleStart; }
		[[nodiscard]] uint32_t GetNrOfDescriptorsInUse() const noexcept { return m_CurrentNrOfDescriptors; }
		uint32_t m_DescriptorSize;
	private:
		[[nodiscard]] bool IsShaderVisible() const noexcept { return m_GpuHandleStart.ptr != 0u; }
		void SetDebugName() noexcept;
	private:
		struct FencedDescriptorHandle
		{
			FencedDescriptorHandle(const DescriptorHandleEx& descriptorHandle, const SyncPoint& syncPoint)
				: DescriptorHandle{ descriptorHandle }, SyncPoint{ syncPoint } {}

			DescriptorHandleEx DescriptorHandle;
			SyncPoint SyncPoint;
		};


		Ref<ID3D12DescriptorHeap> m_pDescriptorHeap = nullptr;
		std::mutex m_Mutex;
		D3D12_CPU_DESCRIPTOR_HANDLE m_CpuHandleStart;
		D3D12_GPU_DESCRIPTOR_HANDLE m_GpuHandleStart;
		D3D12_DESCRIPTOR_HEAP_TYPE m_Type;
		uint32_t m_Capacity;
		uint32_t m_CurrentNrOfDescriptors = 0u;
		std::unique_ptr<uint32_t[]> m_FreeHandles;
		std::queue<FencedDescriptorHandle> m_FreeList;
		uint32_t m_CreatedDescriptors{ 0u };
		uint32_t m_FreedDescriptors{ 0u };
	};
}