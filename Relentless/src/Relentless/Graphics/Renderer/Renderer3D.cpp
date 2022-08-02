#include "Renderer3D.h"
#include "../D3D12Core.h"
#include "RenderCommand.h"
#include "../../Window.h"
#include "../../ImGui/ImguiLayer.h"
namespace Relentless
{
	struct Renderer3dData 
	{
		//Swap to fence-object:
		Microsoft::WRL::ComPtr<ID3D12Fence1> pFence{nullptr};
		HANDLE fenceEvent{ nullptr };
		std::unique_ptr<uint64_t[]> pFenceValues{ nullptr };
		uint32_t currentFrameIndex{0u};
		std::shared_ptr<RenderTextureMSAA> m_pRenderTexture{ nullptr };
		D3D12_VIEWPORT viewPort{};
		RECT scissorRect{};
	};
	static Renderer3dData s_RendererData = {};
	
	void Renderer3D::Initialize() noexcept
	{
		s_RendererData.pFenceValues = std::move(std::make_unique<uint64_t[]>(D3D12Core::GetNrOfBufferedFrames()));

		DXCall(D3D12Core::GetDevice()->CreateFence(0u, D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&s_RendererData.pFence)));
		s_RendererData.pFenceValues[s_RendererData.currentFrameIndex]++;
		s_RendererData.fenceEvent = ::CreateEvent(nullptr, false, false, nullptr);
		RLS_ASSERT(s_RendererData.fenceEvent, "Fence event creation failed.");

		s_RendererData.m_pRenderTexture = RenderTextureMSAA::Create(800, 600, 8u);
		

		s_RendererData.viewPort.TopLeftX = 0.0f;
		s_RendererData.viewPort.TopLeftY = 0.0f;
		s_RendererData.viewPort.Width = 800;
		s_RendererData.viewPort.Height = 600;
		s_RendererData.viewPort.MinDepth = 0.0f;
		s_RendererData.viewPort.MaxDepth = 1.0f;

		s_RendererData.scissorRect.left = 0u;
		s_RendererData.scissorRect.top = 0u;
		s_RendererData.scissorRect.right = static_cast<LONG>(s_RendererData.viewPort.Width);
		s_RendererData.scissorRect.bottom = static_cast<LONG>(s_RendererData.viewPort.Height);
	}

	void Renderer3D::Begin() noexcept
	{
		auto pCommandAllocator{ D3D12Core::GetCommandAllocator(s_RendererData.currentFrameIndex) };
		auto pCommandList{ D3D12Core::GetCommandList() };
		DXCall(pCommandAllocator->Reset());
		DXCall(pCommandList->Reset(pCommandAllocator.Get(), nullptr));

		DXCall_STD(pCommandList->RSSetViewports(1u, & s_RendererData.viewPort));
		DXCall_STD(pCommandList->RSSetScissorRects(1u, &s_RendererData.scissorRect));

		RenderCommand::TransitionResource(s_RendererData.m_pRenderTexture->GetInterface().Get(), D3D12_RESOURCE_STATE_RESOLVE_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

		RenderCommand::ClearRenderTarget(s_RendererData.m_pRenderTexture->GetRTVDescriptorHandle().CPUHandle, DirectX::Colors::Brown);
		DXCall_STD(pCommandList->OMSetRenderTargets(1u, &s_RendererData.m_pRenderTexture->GetRTVDescriptorHandle().CPUHandle, false, nullptr));
	}

	void Renderer3D::Submit() noexcept
	{
		//...
	}

	void Renderer3D::End() noexcept
	{
		BackBuffer backBuffer = Window::GetCurrentBackBuffer();

		RenderCommand::TransitionResource
		(
			s_RendererData.m_pRenderTexture->GetInterface().Get(), 
			D3D12_RESOURCE_STATE_RENDER_TARGET, 
			D3D12_RESOURCE_STATE_RESOLVE_SOURCE
		);

		RenderCommand::TransitionResource
		(
			ImguiLayer::GetUITexture().Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RESOLVE_DEST
		);
		 
		DXCall_STD(D3D12Core::GetCommandList()->ResolveSubresource
		(
			ImguiLayer::GetUITexture().Get(),
			0, 
			s_RendererData.m_pRenderTexture->GetInterface().Get(), 
			0, 
			DXGI_FORMAT_R8G8B8A8_UNORM)
		); 

		RenderCommand::TransitionResource
		(
			ImguiLayer::GetUITexture().Get(),
			D3D12_RESOURCE_STATE_RESOLVE_DEST,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		);

		RenderCommand::TransitionResource(backBuffer.pBackBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		DXCall_STD(D3D12Core::GetCommandList()->OMSetRenderTargets(1u, &backBuffer.Handle.CPUHandle, false, nullptr));
	}

	void Renderer3D::ExecuteCommands() noexcept
	{
		//Transition the back buffer back to present state now that is is finished:
		BackBuffer& backBuffer{ Window::GetCurrentBackBuffer() };
		RenderCommand::TransitionResource(backBuffer.pBackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

		auto pCommandList{ D3D12Core::GetCommandList() };
		DXCall(pCommandList->Close());
		ID3D12CommandList* pCommandLists[] = { pCommandList.Get() };
		DXCall_STD(D3D12Core::GetCommandQueue()->ExecuteCommandLists(ARRAYSIZE(pCommandLists), pCommandLists));
	}

	void Renderer3D::WaitAndSync() noexcept
	{
		// Schedule a Signal command in the queue.
		const UINT64 currentFenceValue = s_RendererData.pFenceValues[s_RendererData.currentFrameIndex];
		DXCall(D3D12Core::GetCommandQueue()->Signal(s_RendererData.pFence.Get(), currentFenceValue));

		// Update the frame index.
		s_RendererData.currentFrameIndex = Window::Get().GetCurrentBackbufferIndex();

		// If the next frame is not ready to be rendered yet, wait until it is ready.
		if (s_RendererData.pFence->GetCompletedValue() < s_RendererData.pFenceValues[s_RendererData.currentFrameIndex])
		{
			DXCall(s_RendererData.pFence->SetEventOnCompletion(s_RendererData.pFenceValues[s_RendererData.currentFrameIndex], s_RendererData.fenceEvent));
			::WaitForSingleObjectEx(s_RendererData.fenceEvent, INFINITE, FALSE);
		}

		// Set the fence value for the next frame.
		s_RendererData.pFenceValues[s_RendererData.currentFrameIndex] = currentFenceValue + 1;
	}

	void Renderer3D::WaitForGPU() noexcept
	{
		// Schedule a Signal command in the queue.
		DXCall(D3D12Core::GetCommandQueue()->Signal(s_RendererData.pFence.Get(), s_RendererData.pFenceValues[s_RendererData.currentFrameIndex]));
		
		// Wait until the fence has been processed.
		DXCall(s_RendererData.pFence->SetEventOnCompletion(s_RendererData.pFenceValues[s_RendererData.currentFrameIndex], s_RendererData.fenceEvent));
		WaitForSingleObjectEx(s_RendererData.fenceEvent, INFINITE, FALSE);
		
		// Increment the fence value for the current frame.
		s_RendererData.pFenceValues[s_RendererData.currentFrameIndex]++;
	}

	void Renderer3D::OnShutDown() noexcept
	{
		WaitForGPU();
	}

	const std::shared_ptr<RenderTextureMSAA>& Renderer3D::GetUITexture() noexcept
	{
		return s_RendererData.m_pRenderTexture;
	}

	void Renderer3D::OnSceneViewportChanged(const uint32_t width, const uint32_t height) noexcept
	{
		s_RendererData.m_pRenderTexture = RenderTextureMSAA::Create(width, height, 8u);

		s_RendererData.viewPort.Width = static_cast<float>(width);
		s_RendererData.viewPort.Height = static_cast<float>(height);

		s_RendererData.scissorRect.right = static_cast<LONG>(s_RendererData.viewPort.Width);
		s_RendererData.scissorRect.bottom = static_cast<LONG>(s_RendererData.viewPort.Height);
	}
}