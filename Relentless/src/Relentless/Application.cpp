#include "Application.h"
#include "Window.h"
#include "Graphics/D3D12Core.h"
#include "Events/LayerStack.h"
#include "Events/EventBuss.h"
#include "Graphics/Renderer/RenderCommand.h"
namespace Relentless
{
	Application::Application(const ApplicationSpecification& applicationSpecification) noexcept
		: m_ApplicationSpecification{ applicationSpecification }, m_IsRunning{ true }, m_CurrentFrameIndex{0u}
	{
		EventBuss::Get().SetMainApplication(this);
		Log::Initialize();
		D3D12Core::Initialize();
		Window::Initialize(m_ApplicationSpecification.Name);
		PushOverlay(std::make_unique<ImguiLayer>());

		//TEMP:
		m_pFenceValues = std::move(std::make_unique<uint64_t[]>(2u));

		DXCall(D3D12Core::GetDevice()->CreateFence(0u, D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence)));
		m_pFenceValues[m_CurrentFrameIndex]++;
		m_FenceEvent = ::CreateEvent(nullptr, false, false, nullptr);
		RLS_ASSERT(m_FenceEvent, "Fence event creation failed.");
	}

	void Application::Run() noexcept
	{
		while (m_IsRunning)
		{
			//Temp test bed:
			uint32_t currentBackBufferIndex{ Window::GetCurrentBackbufferIndex() };
			BackBuffer backBuffer{ Window::GetBackBuffers()[currentBackBufferIndex] };
			auto pCommandAllocator{ D3D12Core::GetCommandAllocator(m_CurrentFrameIndex) };
			auto pCommandList{ D3D12Core::GetCommandList() };

			DXCall(pCommandAllocator->Reset());
			DXCall(pCommandList->Reset(pCommandAllocator.Get(), nullptr));

			//Transition backbuffer resource from present state to render target state:
			RenderCommand::TransitionResource(backBuffer.pBackBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

			//Clear current back buffer:
			RenderCommand::ClearRenderTarget(backBuffer.Handle.CPUHandle, DirectX::Colors::Brown);

			//Set the back buffer as a render target:
			DXCall_STD(pCommandList->OMSetRenderTargets(1u, &backBuffer.Handle.CPUHandle, false, nullptr));

			//Transition the back buffer back to present state now that is is finished:
			RenderCommand::TransitionResource(backBuffer.pBackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

			DXCall(pCommandList->Close());
			
			ID3D12CommandList* pCommandLists[] = { pCommandList.Get() };
			DXCall_STD(D3D12Core::GetCommandQueue()->ExecuteCommandLists(ARRAYSIZE(pCommandLists), pCommandLists));
			Window::Present();

			const uint64_t currentFenceValue = m_pFenceValues[m_CurrentFrameIndex];
			DXCall(D3D12Core::GetCommandQueue()->Signal(m_pFence.Get(), currentFenceValue));
			m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % D3D12Core::GetNrOfBufferedFrames();

			if (m_pFence->GetCompletedValue() < m_pFenceValues[m_CurrentFrameIndex])
			{
				DXCall(m_pFence->SetEventOnCompletion(m_pFenceValues[m_CurrentFrameIndex], m_FenceEvent));
				::WaitForSingleObjectEx(m_FenceEvent, INFINITE, false);
			}
			m_pFenceValues[m_CurrentFrameIndex] = currentFenceValue + 1;

			Window::OnUpdate();
		}
	}

	void Application::PushLayer(std::unique_ptr<Layer> pLayer) const noexcept
	{
		LayerStack::Get().PushLayer(std::move(pLayer));
	}

	void Application::PushOverlay(std::unique_ptr<Layer> pLayer) const noexcept
	{
		LayerStack::Get().PushOverlay(std::move(pLayer));
	}

	void Application::OnEvent(IEvent& event) noexcept
	{
		switch (event.GetEventType())
		{
		case EventType::WindowCloseEvent:
			m_IsRunning = false;
			break;
		}
	}
}