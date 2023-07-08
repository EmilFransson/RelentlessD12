#include "MasterRenderer.h"
#include "../D3D12Core.h"
#include "RenderCommand.h"
#include "../Shaders/ShaderLibrary.h"
#include "RenderPass.h"
#include "../../Core/Window.h"
#include "../Resources/Buffer.h"
#include "FrameBuffer.h"
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

		Microsoft::WRL::ComPtr<ID3D12QueryHeap> m_pQueryHeap{ nullptr };
		std::shared_ptr<ReadBackBuffer> m_pQueryResultBuffer{ nullptr };
		UINT NrOfQueries{ 2 };
		UINT64 GPUFrequency{ 0 };

		std::shared_ptr<FrameBuffer> m_pCompositeFrameBuffer{ nullptr };
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

		D3D12_QUERY_HEAP_DESC HeapDesc = {};
		HeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
		HeapDesc.Count = s_Data.NrOfQueries;
		HeapDesc.NodeMask = 0;

		DXCall(D3D12Core::GetDevice()->CreateQueryHeap(&HeapDesc,  IID_PPV_ARGS(&s_Data.m_pQueryHeap)));

		DXCall(D3D12Core::GetCommandQueue()->GetTimestampFrequency(&s_Data.GPUFrequency));

		s_Data.m_pQueryResultBuffer = ReadBackBuffer::Create(s_Data.NrOfQueries * sizeof(UINT64), "QueryResult Readback Buffer");
	
		//Final composite frame buffer:
		{
			ColorAttachment colorAttachment;
			colorAttachment.Format = TextureFormat::RGBA32F;
			colorAttachment.OperatorOnLoad = OperatorOnLoad::LoadOnly;
			colorAttachment.Transfer = true;

			FrameBufferSpecification fbSpec;
			fbSpec.DebugName = "Final Composite FrameBuffer";
			fbSpec.Width = 800u;
			fbSpec.Height = 600u;
			fbSpec.Attachments.ColorAttachments = { colorAttachment };

			s_Data.m_pCompositeFrameBuffer = FrameBuffer::Create(fbSpec);
		}
		
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

		UINT64* pTimestamps = nullptr;

		D3D12_RANGE readRange = { 0, s_Data.NrOfQueries * sizeof(UINT64) };
		DXCall(s_Data.m_pQueryResultBuffer->GetInterface()->Map(0, &readRange, reinterpret_cast<void**>(&pTimestamps)));

		UINT64 startTime = pTimestamps[0];
		UINT64 endTime = pTimestamps[1];

		double TimeInMs = (((double)endTime - (double)startTime) / (double)s_Data.GPUFrequency) * 1000.0f;
		std::cout << "Time in MS: " << std::to_string(TimeInMs) << "\n";
		DXCall_STD(s_Data.m_pQueryResultBuffer->GetInterface()->Unmap(0, nullptr));
	}

	void MasterRenderer::WaitAndSyncAllFramesInFlight() noexcept
	{
		auto WaitForPreviousFrame = [&](UINT frameIndex) {
			const UINT64 currentFenceValue = s_Data.pFenceValues[frameIndex];
			DXCall(D3D12Core::GetCommandQueue()->Signal(s_Data.pFence.Get(), currentFenceValue));
			s_Data.pFenceValues[frameIndex] = currentFenceValue + 1;

			// If the next frame is not ready to be rendered yet, wait until the fence to complete.
			if (s_Data.pFence->GetCompletedValue() < currentFenceValue) {
				DXCall(s_Data.pFence->SetEventOnCompletion(currentFenceValue, s_Data.fenceEvent));
				WaitForSingleObject(s_Data.fenceEvent, INFINITE);
			}
		};

		for (uint32_t i{ 0u }; i < D3D12Core::GetNrOfBufferedFrames(); ++i)
		{
			WaitForPreviousFrame(i);
		}
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

	void MasterRenderer::Begin() noexcept
	{
		ResetFrameCommandUnits(GetCurrentFrameIndex());
		DXCall_STD(D3D12Core::GetCommandList()->EndQuery(s_Data.m_pQueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0));
	}

	void MasterRenderer::End() noexcept
	{
		DXCall_STD(D3D12Core::GetCommandList()->EndQuery(s_Data.m_pQueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 1));

		// Resolve the query data
		DXCall_STD(D3D12Core::GetCommandList()->ResolveQueryData(s_Data.m_pQueryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0, s_Data.NrOfQueries, s_Data.m_pQueryResultBuffer->GetInterface().Get(), 0));
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
		auto pFrameBuffer = pPipeline->GetFrameBuffer();
		auto& outputs = pFrameBuffer->GetSpecification().Attachments.ColorAttachments;

		for (uint32_t i{ 0u }; i < outputs.size(); ++i)
		{
			if (outputs[i].Output->GetCurrentState() != D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET)
				RenderCommand::TransitionResource(outputs[i].Output, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}

		DXCall_STD(D3D12Core::GetCommandList()->BeginRenderPass(static_cast<UINT>(pRenderPass->GetAllOutputs().size()), pRenderPass->GetAllOutputs().data(), pipelineSpecification.DepthWrite ? &pRenderPass->GetDepthOutput2() : nullptr, D3D12_RENDER_PASS_FLAG_NONE));

		RenderCommand::SetRootSignature(pPipeline->GetRootSig());
		RenderCommand::SetPipelineState(pPipeline->GetInterface2());
		RenderCommand::SetTopology(RLSTopologyToD3D12Topology(pipelineSpecification.Topology));
	}

	void MasterRenderer::EndRenderPass() noexcept
	{
		DXCall_STD(D3D12Core::GetCommandList()->EndRenderPass());
	}

	void MasterRenderer::PrepareBackBuffer() noexcept
	{
		//Transition the back buffer back to present state now that is is finished:
		BackBuffer& backBuffer{ Window::GetCurrentBackBuffer() };
		RenderCommand::TransitionResource(backBuffer.pBackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	}

	const std::shared_ptr<FrameBuffer> MasterRenderer::GetFrameBuffer() noexcept
	{
		return s_Data.m_pCompositeFrameBuffer;
	}

	void MasterRenderer::Resize(const uint32_t width, const uint32_t height) noexcept
	{
		s_Data.m_pCompositeFrameBuffer->Resize(width, height);
	}
}