#include "Application.h"
#include "Window.h"
#include "../Graphics/D3D12Core.h"
#include "../EventSystem/LayerStack.h"
#include "../EventSystem/EventBus.h"
#include "../Graphics/Renderer/Renderer3D.h"
#include "../Graphics/MemoryManager.h"
#include "Timer.h"
#include "../Input/Mouse.h"
namespace Relentless
{
	Application::Application(const ApplicationSpecification& applicationSpecification) noexcept
		: m_ApplicationSpecification{ applicationSpecification }
	{
		OnStartUp();
	}

	void Application::Run() noexcept
	{
		Renderer3D::ExecuteCommands();
		Renderer3D::WaitForGPU();
		while (m_IsRunning)
		{
			Timer::Update();
			
			MemoryManager::Get().PerformDeferredDeletion();

			for (auto& pLayer : LayerStack::Get())
			{
				pLayer->OnUpdate(Timer::GetDeltaTime());
				pLayer->OnRender();
			}

			ImguiLayer::BeginFrame();
			for (auto& pLayer : LayerStack::Get())
				pLayer->OnImGuiRender();
			ImguiLayer::EndFrame();

			Renderer3D::PrepareBackBuffer();
			Renderer3D::ExecuteCommands();

			Window::Present();

			Renderer3D::WaitAndSync();
			Mouse::Reset();

			Window::OnUpdate();
			D3D12Core::AdvanceToNextFrame();
		}
		ShutDown();
	}

	void Application::PushLayer(Layer* pLayer) const noexcept
	{
		LayerStack::Get().PushLayer(pLayer);
	}

	void Application::PushOverlay(Layer* pLayer) const noexcept
	{
		LayerStack::Get().PushOverlay(pLayer);
	}

	void Application::OnEvent(IEvent& event) noexcept
	{
		switch (event.GetEventType())
		{
		case EventType::WindowClosedEvent:
			m_IsRunning = false;
			break;
		case EventType::WindowResizedEvent:
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
		EventBus::Get().SetMainApplication(this);
		Log::Initialize();
		D3D12Core::Initialize();
		MemoryManager::Get().Initialize();

		std::string engineIni = std::string(MAIN_ENGINE_DIRECTORY) + std::string("engine.ini");

		uint32_t windowWidth{ 1280u };
		uint32_t windowHeight{ 720u };
		if (std::filesystem::exists(engineIni))
		{
			std::ifstream inFile(engineIni);
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
		PushOverlay(&m_ImGuiLayer);

		m_IsRunning = true;
	}

	void Application::ShutDown() noexcept
	{
		std::string engineDirectory = std::string(MAIN_ENGINE_DIRECTORY) + std::string("engine.ini");

		std::ofstream outFile(engineDirectory);
		outFile << "[RenderWindow][Dimensions]\n";
		outFile << Window::GetWidth() << "\n";
		outFile << Window::GetHeight();
		outFile.close();

		Renderer3D::OnShutDown();

		m_IsRunning = false;
	}
}