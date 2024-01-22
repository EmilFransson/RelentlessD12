#include "Application.h"
#include "Assets/AssetManager.h"
#include "EventSystem/LayerStack.h"
#include "EventSystem/EventBus.h"
#include "Graphics/D3D12Core.h"
#include "Graphics/Renderer/MasterRenderer.h"
#include "Graphics/MemoryManager.h"
#include "Input/Mouse.h"
#include "Timer.h"
#include "UI/UI.h"
#include "Window.h"

namespace Relentless
{
	Application* Application::s_Instance = nullptr;

	Application& Application::Get() noexcept
	{
		return *s_Instance;
	}

	Application::Application(const ApplicationSpecification& applicationSpecification) noexcept
		: m_ApplicationSpecification{ applicationSpecification }
	{
		RLS_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		OnStartUp();
	}

	void Application::Run() noexcept
	{
		MasterRenderer::ExecuteCommands();
		MasterRenderer::WaitForGPU();
		while (m_IsRunning)
		{
			PROFILE_FUNC;

			ExecuteMainThreadQueue();

			Timer::Update();
			
			
			MemoryManager::Get().PerformDeferredDeletion();

			{
				PROFILE_SCOPE("Application::Run::OnUpdateAndRender");

				for (auto& pLayer : LayerStack::Get())
				{
					pLayer->OnUpdate(Timer::GetDeltaTime());
					pLayer->OnRender();
				}
			}

			{
				PROFILE_SCOPE("Application::Run::OnImGuiRender");

				ImguiLayer::BeginFrame();
				for (auto& pLayer : LayerStack::Get())
					pLayer->OnImGuiRender();
				ImguiLayer::EndFrame();
			}

			MasterRenderer::PrepareBackBuffer();
			MemoryManager::Get().GetUploadBuffer()->Upload();
			MasterRenderer::ExecuteCommands();


			Window::Present();
			MasterRenderer::WaitAndSync();

			{
				PROFILE_SCOPE("Application::Run::OnPostRender");

				for (auto& pLayer : LayerStack::Get())
				{
					pLayer->OnPostRender();
				}
			}

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

			MasterRenderer::WaitForGPU();
			Window::Resize();
			break;
		}
		}
	}

	void Application::SubmitToMainThread(const std::function<void()>& func)
	{
		std::scoped_lock(m_MainThreadFunctionQueueMutex);
		m_MainThreadFunctionQueue.push(func);
	}

	void Application::OnStartUp() noexcept
	{
		EventBus::Get().SetMainApplication(this);
		Log::Initialize();
		D3D12Core::Initialize();
		MemoryManager::Get().Initialize();
		AssetManager::Initialize();
		AssetRegistry::RecursiveScanDirectoryForAssets(ENGINE_ASSET_DIRECTORY);
		AssetRegistry::RecursiveScanDirectoryForAssets(EDITOR_ASSET_DIRECTORY);
		UI::Initialize();

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
		MasterRenderer::Initialize();
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

		MasterRenderer::OnShutDown();

		m_IsRunning = false;
	}

	void Application::ExecuteMainThreadQueue() noexcept
	{
		std::scoped_lock(m_MainThreadFunctionQueueMutex);

		while (m_MainThreadFunctionQueue.size() > 0)
		{
			auto& func = m_MainThreadFunctionQueue.front();
			func();

			m_MainThreadFunctionQueue.pop();
		}
	}

}