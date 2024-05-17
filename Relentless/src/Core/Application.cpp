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

#include "Math/Vector3.h"

#include "Utility/SystemPaths.h"

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
		MemoryManager::Get().GetUploadBuffer()->Upload();
		m_GPUTaskManager.WaitForAllFramesComplete();

		while (true)
		{
			PROFILE_FUNC;

			ExecuteMainThreadQueue();
			Window::OnUpdate();
			if (!m_IsRunning)
				break;

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
			
				Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> pCommandList = m_GPUTaskManager.RequestCommandList(CommandType::Direct);
				
				ImguiLayer::BeginFrame(pCommandList);
				for (auto& pLayer : LayerStack::Get())
					pLayer->OnImGuiRender();
				ImguiLayer::EndFrame(pCommandList);
			}

			{
				PROFILE_SCOPE("Application::Run::OnPostRender");

				for (auto& pLayer : LayerStack::Get())
				{
					pLayer->OnPostRender();
				}
			}
			 
			MemoryManager::Get().GetUploadBuffer()->Upload();
			Mouse::Reset();

			MasterRenderer::PrepareBackBuffer();

			m_GPUTaskManager.Flush();

			if (m_ShouldResizeWindow)
			{
				m_GPUTaskManager.WaitForAllFramesComplete();
				Window::Resize();
				m_ShouldResizeWindow = false;
			}

			Window::Present();

			m_GPUTaskManager.MoveToNextFrame();
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

			m_ShouldResizeWindow = true;
			break;
		}
		}
	}

	void Application::SubmitToMainThread(const std::function<void()>& func)
	{
		std::scoped_lock(m_MainThreadFunctionQueueMutex);
		m_MainThreadFunctionQueue.push(func);
	}

	ThreadPool& Application::GetThreadPool() noexcept
	{
		return m_ThreadPool;
	}

	GPUTaskManager& Application::GetGPUTaskManager() noexcept
	{
		return m_GPUTaskManager;
	}

	void Application::OnStartUp() noexcept
	{
		EventBus::Get().SetMainApplication(this);
		Log::Initialize();
		SystemPaths::Initialize();
		D3D12Core::Initialize();
		m_GPUTaskManager.Initialize();
		MemoryManager::Get().Initialize();
		MasterRenderer::Initialize();
		AssetRegistry::RecursiveScanDirectoryForAssets(ENGINE_ASSET_DIRECTORY);
		AssetRegistry::RecursiveScanDirectoryForAssets(EDITOR_ASSET_DIRECTORY);
		AssetManager::Initialize();
		
		UI::Initialize();

		const std::filesystem::path engineIni = FilepathUtils::Combine(SystemPaths::GetUserDocumentsDirectory(), "engine.ini");

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
		else
		{
			std::ofstream outFile(engineIni);
			outFile << "[RenderWindow][Dimensions]\n";
			outFile << windowWidth << "\n";
			outFile << windowHeight;
			outFile.close();
		}

		Window::Initialize(m_ApplicationSpecification.Name, windowWidth, windowHeight);
		
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

		m_GPUTaskManager.WaitForAllFramesComplete();
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