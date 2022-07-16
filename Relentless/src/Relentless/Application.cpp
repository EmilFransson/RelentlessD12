#include "Application.h"
#include "Window.h"
#include "Graphics/D3D12Core.h"
#include "Events/LayerStack.h"
#include "Events/EventBuss.h"
namespace Relentless
{
	Application::Application(const ApplicationSpecification& applicationSpecification) noexcept
		: m_ApplicationSpecification{ applicationSpecification }, m_IsRunning{ true }
	{
		EventBuss::Get().SetMainApplication(this);
		Log::Initialize();
		Window::Initialize(m_ApplicationSpecification.Name);
		D3D12Core::Initialize();
	}

	void Application::Run() noexcept
	{
		while(m_IsRunning)
			Window::OnUpdate();
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