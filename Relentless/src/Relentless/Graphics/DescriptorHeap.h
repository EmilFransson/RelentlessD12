#pragma once 
namespace Relentless
{
	class DescriptorHeap;

	struct DescriptorHandle
	{
		D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE GPUHandle;

#if defined(RLS_DEBUG)
		friend class DescriptorHeap;
		DescriptorHeap* pDebugInterface;
		uint32_t Index;
#endif
	};

	class DescriptorHeap
	{
	public:
		explicit DescriptorHeap(const D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType, const uint32_t capacity, const bool isShaderVisible) noexcept;
		~DescriptorHeap() noexcept = default;
		[[nodiscard]] DescriptorHandle AllocateDescriptor() noexcept;
		void FreeDescriptor(const DescriptorHandle& descriptorHandle) noexcept;
		[[nodiscard]] constexpr Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& GetDescriptorHeapInterface() noexcept { return m_pDescriptorHeap; }
		[[nodiscard]] constexpr D3D12_CPU_DESCRIPTOR_HANDLE GetCPUStartHandle() noexcept { return m_CpuHandleStart; }
		[[nodiscard]] constexpr D3D12_GPU_DESCRIPTOR_HANDLE GetGPUStartHandle() noexcept { return m_GpuHandleStart; }
	private:
		[[nodiscard]] constexpr bool IsShaderVisible() noexcept { return m_GpuHandleStart.ptr != 0u; }
	private:
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_pDescriptorHeap;
		std::mutex m_Mutex;
		D3D12_CPU_DESCRIPTOR_HANDLE m_CpuHandleStart;
		D3D12_GPU_DESCRIPTOR_HANDLE m_GpuHandleStart;
		uint32_t m_DescriptorSize;
		D3D12_DESCRIPTOR_HEAP_TYPE m_Type;
		uint32_t m_Capacity;
		uint32_t m_CurrentNrOfDescriptors;
		std::unique_ptr<uint32_t[]> m_FreeHandles;
		std::vector<DescriptorHandle> m_DeferredFreeList;
	};
}