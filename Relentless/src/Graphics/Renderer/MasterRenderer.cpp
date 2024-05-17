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
		std::mutex FenceMutex;
	};

	static MasterRendererData s_Data = {};


	void MasterRenderer::Initialize() noexcept
	{
		//Fence:
		s_Data.pFenceValues = std::make_unique<uint64_t[]>(GPUTaskManager::FRAMES_IN_FLIGHT);
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

		DXCall(Application::Get().GetGPUTaskManager().GetCommandQueue(CommandType::Direct)->GetTimestampFrequency(&s_Data.GPUFrequency));

		s_Data.m_pQueryResultBuffer = ReadBackBuffer::Create(s_Data.NrOfQueries * sizeof(UINT64), "QueryResult Readback Buffer");
	
		//Final composite frame buffer:
		{
			ColorAttachment colorAttachment;
			colorAttachment.Format = TextureFormat::RGBA32F;
			colorAttachment.OperatorOnLoad = OperatorOnLoad::LoadOnly;
			colorAttachment.Transfer = true;
			colorAttachment.ShouldResize = true;

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

	void MasterRenderer::OnShutDown() noexcept
	{

	}

	void MasterRenderer::Begin() noexcept
	{
	}

	void MasterRenderer::End() noexcept
	{
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

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> MasterRenderer::BeginRenderPass(const std::shared_ptr<RenderPass>& pRenderPass) noexcept
	{
		auto& pPipeline = pRenderPass->GetPipeline();
		auto& pipelineSpecification = pPipeline->GetSpecification();
		auto pFrameBuffer = pPipeline->GetFrameBuffer();
		auto& outputs = pFrameBuffer->GetSpecification().Attachments.ColorAttachments;
		auto& depthOutput = pFrameBuffer->GetSpecification().Attachments.DepthAttachment;

		bool hasDepthAttachment = false;

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> pCommandList = Application::Get().GetGPUTaskManager().RequestCommandList(CommandType::Direct);

		for (uint32_t i{ 0u }; i < outputs.size(); ++i)
		{
			if (outputs[i].Output->GetCurrentState() != D3D12_RESOURCE_STATE_RENDER_TARGET)
			{
				D3D12_RESOURCE_BARRIER resourceTransitionBarrier{};
				resourceTransitionBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				resourceTransitionBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				resourceTransitionBarrier.Transition.pResource = outputs[i].Output->GetInterface().Get();
				resourceTransitionBarrier.Transition.StateBefore = outputs[i].Output->GetCurrentState();
				resourceTransitionBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
				resourceTransitionBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

				DXCall_STD(pCommandList->ResourceBarrier(1u, &resourceTransitionBarrier));
				outputs[i].Output->SetCurrentState(D3D12_RESOURCE_STATE_RENDER_TARGET);
			}
		}
		if (depthOutput.Output)
		{
			hasDepthAttachment = true;
			if (depthOutput.Output->GetCurrentState() != D3D12_RESOURCE_STATE_DEPTH_WRITE)
			{
				D3D12_RESOURCE_BARRIER resourceTransitionBarrier{};
				resourceTransitionBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				resourceTransitionBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				resourceTransitionBarrier.Transition.pResource = depthOutput.Output->GetInterface().Get();
				resourceTransitionBarrier.Transition.StateBefore = depthOutput.Output->GetCurrentState();
				resourceTransitionBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
				resourceTransitionBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

				DXCall_STD(pCommandList->ResourceBarrier(1u, &resourceTransitionBarrier));
				depthOutput.Output->SetCurrentState(D3D12_RESOURCE_STATE_DEPTH_WRITE);
			}
		}
		
		pCommandList->BeginRenderPass(static_cast<UINT>(pRenderPass->GetAllOutputs().size()), pRenderPass->GetAllOutputs().data(), hasDepthAttachment ? &pRenderPass->GetDepthOutput2() : nullptr, D3D12_RENDER_PASS_FLAG_NONE);

		DXCall_STD(pCommandList->SetGraphicsRootSignature(pPipeline->GetRootSig().Get()));
		DXCall_STD(pCommandList->SetPipelineState(pPipeline->GetInterface2().Get()));
		DXCall_STD(pCommandList->IASetPrimitiveTopology(RLSTopologyToD3D12Topology(pipelineSpecification.Topology)));

		return pCommandList;
	}

	void MasterRenderer::EndRenderPass(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> pCommandList) noexcept
	{
		DXCall_STD(pCommandList->EndRenderPass());

		Application::Get().GetGPUTaskManager().ScheduleCommandList(std::move(pCommandList));
	}

	void MasterRenderer::PrepareBackBuffer() noexcept
	{
		PROFILE_FUNC;

		//Transition the back buffer back to present state now that is is finished:
		BackBuffer& backBuffer{ Window::GetBackBuffers()[Application::Get().GetGPUTaskManager().GetCurrentFrameIndex()] };

		D3D12_RESOURCE_BARRIER resourceTransitionBarrier{};
		resourceTransitionBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		resourceTransitionBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		resourceTransitionBarrier.Transition.pResource = backBuffer.pBackBuffer.Get();
		resourceTransitionBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		resourceTransitionBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		resourceTransitionBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> pCommandList = Application::Get().GetGPUTaskManager().RequestCommandList(CommandType::Direct);
		DXCall_STD(pCommandList->ResourceBarrier(1u, &resourceTransitionBarrier));
		Application::Get().GetGPUTaskManager().ScheduleCommandList(pCommandList);
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