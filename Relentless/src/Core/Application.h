#pragma once
#include "ImGui/ImguiLayer.h"
#include "Threading/ThreadPool.h"

#include "Callback/Broadcaster.h"
namespace Relentless
{
	struct ApplicationSpecification
	{
		std::string Name;
	};
	
	class Layer;
	class IEvent;
	class Application
	{
	public:
		[[nodiscard]] static Application& Get() noexcept;
		Application(const ApplicationSpecification& applicationSpecification) noexcept;
		virtual ~Application() noexcept = default;
		void Run() noexcept;
		void PushLayer(Layer* pLayer) const noexcept;
		void PushOverlay(Layer* pLayer) const noexcept;
		void OnEvent(IEvent& event) noexcept;

		void SubmitToMainThread(const std::function<void()>& func);
		[[nodiscard]] ThreadPool& GetThreadPool() noexcept;
	private:
		void OnStartUp() noexcept;
		void ShutDown() noexcept;
		void ExecuteMainThreadQueue() noexcept;
		[[nodiscard]] constexpr const bool IsInitialized() const noexcept { return m_IsRunning == true; }
	private:
		static Application* s_Instance;

		ApplicationSpecification m_ApplicationSpecification;
		ImguiLayer m_ImGuiLayer;
		bool m_IsRunning;

		std::queue<std::function<void()>> m_MainThreadFunctionQueue;
		std::mutex m_MainThreadFunctionQueueMutex;

		ThreadPool m_ThreadPool;
	};

	//To be defined in client (runtime-project):
	[[nodiscard]] const std::unique_ptr<Application> CreateApplication() noexcept;
}