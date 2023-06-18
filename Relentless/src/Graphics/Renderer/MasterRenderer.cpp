#include "MasterRenderer.h"
#include "../D3D12Core.h"
#include "RenderCommand.h"
#include "../Shaders/ShaderLibrary.h"
#include "RenderPass.h"
#include "../../Core/Window.h"
namespace Relentless
{
	struct MasterRendererData
	{
		//Swap to fence-object:
		Microsoft::WRL::ComPtr<ID3D12Fence1> pFence{ nullptr };
		HANDLE fenceEvent{ nullptr };
		std::unique_ptr<uint64_t[]> pFenceValues{ nullptr };

		uint32_t currentFrameIndex{ 0u };

		ShaderLibrary Shaderlibrary;
	};

	static MasterRendererData s_Data = {};


	void MasterRenderer::Initialize() noexcept
	{
		//Fence:
		s_Data.pFenceValues = std::make_unique<uint64_t[]>(D3D12Core::GetNrOfBufferedFrames());
		DXCall(D3D12Core::GetDevice()->CreateFence(0u, D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&s_Data.pFence)));
		s_Data.pFenceValues[s_Data.currentFrameIndex]++;
		s_Data.fenceEvent = ::CreateEvent(nullptr, false, false, nullptr);
		RLS_ASSERT(s_Data.fenceEvent, "Fence event creation failed.");

		s_Data.Shaderlibrary.Initialize();
	}

	ShaderLibrary& MasterRenderer::GetShaderLibrary() noexcept
	{
		return s_Data.Shaderlibrary;
	}

	void MasterRenderer::ExecuteCommands() noexcept
	{
		PROFILE_FUNC;

		auto pCommandList{ D3D12Core::GetCommandList() };
		DXCall(pCommandList->Close());
		ID3D12CommandList* pCommandLists[] = { pCommandList.Get() };
		DXCall_STD(D3D12Core::GetCommandQueue()->ExecuteCommandLists(ARRAYSIZE(pCommandLists), pCommandLists));
	}

	void MasterRenderer::WaitAndSync() noexcept
	{
		PROFILE_FUNC;

		// Schedule a Signal command in the queue.
		const UINT64 currentFenceValue = s_Data.pFenceValues[s_Data.currentFrameIndex];
		DXCall(D3D12Core::GetCommandQueue()->Signal(s_Data.pFence.Get(), currentFenceValue));

		s_Data.currentFrameIndex = D3D12Core::GetCurrentFrame() % D3D12Core::GetNrOfBufferedFrames();

		// If the next frame is not ready to be rendered yet, wait until it is ready.
		if (s_Data.pFence->GetCompletedValue() < s_Data.pFenceValues[s_Data.currentFrameIndex])
		{
			DXCall(s_Data.pFence->SetEventOnCompletion(s_Data.pFenceValues[s_Data.currentFrameIndex], s_Data.fenceEvent));
			::WaitForSingleObjectEx(s_Data.fenceEvent, INFINITE, FALSE);
		}

		// Set the fence value for the next frame.
		s_Data.pFenceValues[s_Data.currentFrameIndex] = currentFenceValue + 1;
	}

	void MasterRenderer::WaitForGPU() noexcept
	{
		// Schedule a Signal command in the queue.
		DXCall(D3D12Core::GetCommandQueue()->Signal(s_Data.pFence.Get(), s_Data.pFenceValues[s_Data.currentFrameIndex]));

		// Wait until the fence has been processed.
		DXCall(s_Data.pFence->SetEventOnCompletion(s_Data.pFenceValues[s_Data.currentFrameIndex], s_Data.fenceEvent));
		WaitForSingleObjectEx(s_Data.fenceEvent, INFINITE, FALSE);

		// Increment the fence value for the current frame.
		s_Data.pFenceValues[s_Data.currentFrameIndex]++;
	}

	void MasterRenderer::OnShutDown() noexcept
	{
		WaitForGPU();
	}

	void MasterRenderer::ResetFrameCommandUnits(const uint32_t frameIndex) noexcept
	{
		auto pCommandAllocator{ D3D12Core::GetCommandAllocator(frameIndex) };
		auto pCommandList{ D3D12Core::GetCommandList() };
		DXCall(pCommandAllocator->Reset());
		DXCall(pCommandList->Reset(pCommandAllocator.Get(), nullptr));
	}

	uint32_t MasterRenderer::GetCurrentFrameIndex() noexcept
	{
		return (D3D12Core::GetCurrentFrame() % D3D12Core::GetNrOfBufferedFrames());
	}

	static D3D_PRIMITIVE_TOPOLOGY RLSTopologyToD3D12Topology(RLS::Topology topology) noexcept
	{
		switch (topology)
		{
		case Topology::Triangle:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			break;
		case Topology::Line:
			return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
			break;
		case Topology::Point:
			return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
			break;
		}
		
		RLS_ASSERT(false, "Unknown topology type encountered.");
		return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	}

	void MasterRenderer::BeginRenderPass(const std::shared_ptr<RenderPass>& pRenderPass) noexcept
	{
		auto& pPipeline = pRenderPass->GetPipeline();
		auto& pipelineSpecification = pPipeline->GetSpecification();
		FrameBuffer* pFrameBuffer = pPipeline->GetFrameBuffer();
		auto& frameBufferSpecification = pFrameBuffer->GetSpecification();

		auto& pColorOutput = pPipeline->GetFrameBuffer()->GetColorBuffer();

		//If transfer this means that it would have been set to another state to facilitate that.
		if (frameBufferSpecification.Transfer)
		{
			RenderCommand::TransitionResource(pColorOutput, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}

		//Here we should always check attachment operations and act on them accordingly!! (which we aren't)
		RenderCommand::ClearRenderTarget(pColorOutput->GetRTVDescriptorHandle().CPUHandle, pColorOutput->GetClearColor());
		
		if (pipelineSpecification.DepthWrite)
		{
			auto& pDepthOutput = pPipeline->GetFrameBuffer()->GetDepthBuffer();
			RenderCommand::ClearDepthStencil(pDepthOutput);
			RenderCommand::SetRenderTarget(pColorOutput, pDepthOutput);
		}
		else
		{
			RenderCommand::SetRenderTarget(pColorOutput, nullptr);
		}

		RenderCommand::SetRootSignature(pPipeline->GetRootSig());
		RenderCommand::SetPipelineState(pPipeline->GetInterface2());
		RenderCommand::SetTopology(RLSTopologyToD3D12Topology(pipelineSpecification.Topology));
	}

	void MasterRenderer::PrepareBackBuffer() noexcept
	{
		//Transition the back buffer back to present state now that is is finished:
		BackBuffer& backBuffer{ Window::GetCurrentBackBuffer() };
		RenderCommand::TransitionResource(backBuffer.pBackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	}
}