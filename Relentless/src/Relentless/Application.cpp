#include "Application.h"
#include "Window.h"
#include "Graphics/D3D12Core.h"
#include "Events/LayerStack.h"
#include "Events/EventBuss.h"
#include "Graphics/Renderer/Renderer3D.h"
#include "Graphics/MemoryManager.h"
namespace Relentless
{
	Application::Application(const ApplicationSpecification& applicationSpecification) noexcept
		: m_ApplicationSpecification{ applicationSpecification }
	{
		EventBuss::Get().SetMainApplication(this);
		Log::Initialize();
		D3D12Core::Initialize();
		MemoryManager::Get().Initialize();
		Window::Initialize(m_ApplicationSpecification.Name);
		Renderer3D::Initialize();
		PushOverlay(std::make_unique<ImguiLayer>());

		m_IsRunning = true;
	}

	void Application::Run() noexcept
	{
		while (m_IsRunning)
		{
			MemoryManager::Get().PerformDeferredDeletion();

			Window::OnUpdate();

			for (auto& pLayer : LayerStack::Get())
				pLayer->OnUpdate(0.1f);

			ImguiLayer::BeginFrame();
			for (auto& pLayer : LayerStack::Get())
				pLayer->OnImGuiRender();
			ImguiLayer::EndFrame();

			Renderer3D::ExecuteCommands();

			Window::Present();

			Renderer3D::WaitAndSync();
		}
		ShutDown();
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
		case EventType::WindowResizeEvent:
		{
			if (!IsInitialized())
				return;

			auto[width, height] = static_cast<WindowResizeEvent&>(event).GetNewDimension();
			RLS_CORE_INFO("Resized window: [{0},{1}]", width, height);

			Renderer3D::WaitForGPU();
			Window::CreateSizeDependentResources();
			MemoryManager::Get().PerformDeferredDeletion();
			break;
		}
		}
	}

	void Application::ShutDown() noexcept
	{
		Renderer3D::OnShutDown();
	}
}