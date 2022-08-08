#include "Application.h"
#include "Window.h"
#include "ImGui\ImguiLayer.h"
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
		OnStartUp();
	}

	void Application::Run() noexcept
	{
		while (m_IsRunning)
		{
			Window::OnUpdate();
			
			MemoryManager::Get().PerformDeferredDeletion();

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

			Renderer3D::WaitForGPU();
			Window::Resize();
			break;
		}
		}
	}

	void Application::OnStartUp() noexcept
	{
		EventBuss::Get().SetMainApplication(this);
		Log::Initialize();
		D3D12Core::Initialize();
		MemoryManager::Get().Initialize();

		uint32_t windowWidth{ 1280u };
		uint32_t windowHeight{ 720u };
		if (std::filesystem::exists("engine.ini"))
		{
			std::ifstream inFile("engine.ini");
			std::string s;
			while (inFile >> s)
			{
				if (s == "[RenderWindow][Dimensions]")
				{
					inFile.ignore(1);
					inFile >> windowWidth;
					inFile >> windowHeight;
					break;
				}
			}
			inFile.close();
		}

		Window::Initialize(m_ApplicationSpecification.Name, windowWidth, windowHeight);
		Renderer3D::Initialize();
		PushOverlay(std::make_unique<ImguiLayer>());

		m_IsRunning = true;
	}

	void Application::ShutDown() noexcept
	{
		Renderer3D::OnShutDown();

		std::ofstream outFile("engine.ini");
		outFile << "[RenderWindow][Dimensions]\n";
		outFile << Window::GetWidth() << "\n";
		outFile << Window::GetHeight();
		outFile.close();

		m_IsRunning = false;
	}
}