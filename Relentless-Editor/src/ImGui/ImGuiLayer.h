#pragma once
#include <Relentless.h>
#include "ImGui/ImGuiIncludes.h"

namespace Relentless
{
	#define OPENSANS_BOLD_18 0

	class ImGuiSRVAllocator
	{
	public:
		void Allocate(D3D12_CPU_DESCRIPTOR_HANDLE* aOutCpu, D3D12_GPU_DESCRIPTOR_HANDLE* aOutGpu) noexcept
		{
			const uint32 index = m_NextFree++;
			RLS_VERIFY(index < m_DescriptorHandles.size(), "[ImGuiSRVAllocator::Allocate]: Descriptor handle request out-of-bounds error.");
			
			DescriptorHandle& handle = m_DescriptorHandles[index];
			*aOutCpu = handle.CPUHandle;
			*aOutGpu = handle.GPUHandle;
		}

		void Free(MAYBE_UNUSED D3D12_CPU_DESCRIPTOR_HANDLE aCPU, MAYBE_UNUSED D3D12_GPU_DESCRIPTOR_HANDLE aGPU) noexcept
		{
			//Nothing for now
		}

		void Initialize(GraphicsDevice* aDevice) noexcept
		{
			m_DescriptorHandles = aDevice->RegisterGlobalDescriptorBlock(DescriptorHandleType::SRV, 64);
		}
	private:
		std::vector<DescriptorHandle> m_DescriptorHandles;
		std::atomic<size_t> m_NextFree = 0u;
	};

	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer(GraphicsDevice* pDevice) noexcept;
		virtual ~ImGuiLayer() noexcept override = default;

		void BeginFrame(Ref<Texture> pTarget, CommandContext* pCommandContext) noexcept;
		void EndFrame(CommandContext* pCommandContext) noexcept;

		virtual void OnImGuiRender() noexcept override final;
	private:
		virtual void OnAttach() override final;
		virtual void OnDetach() override final;
	private:
		GraphicsDevice* m_pDevice = nullptr;
		UniquePtr<DescriptorHeap> m_pDescriptorHeap = nullptr;
		DescriptorHandle m_DescriptorHandle;
		ImGuiSRVAllocator m_Allocator;
	};
}
