#pragma once
#include "ImGui/ImguiLayer.h"
#include "Threading/ThreadPool.h"
#include "Callback/Broadcaster.h"
#include "Graphics/GPUTaskManager.h"
#include "Graphics/MemoryManager.h"
#include "Graphics/Resources/ResourceManager.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/RHI.h"
#include "Graphics/RHI/Swapchain.h"
#include "Graphics/RHI/Window.h"
#include "Graphics/Renderer/Renderer.h"

#include "EventSystem/EventPublisher.h"

namespace Relentless
{
	struct ApplicationSpecification
	{
		std::string Name;
	};
	
	class Layer;

	class Application : public EventPublisher
	{
	public:
		[[nodiscard]] static Application& Get() noexcept;
		Application(const ApplicationSpecification& applicationSpecification) noexcept;
		virtual ~Application() noexcept = default;

		[[nodiscard]] GraphicsDevice* GetGraphicsDevice() const noexcept;
		[[nodiscard]] const UniquePtr<WindowEx>& GetWindow() const noexcept;

		void Run() noexcept;
		void PushLayer(Layer* pLayer) const noexcept;
		void PushOverlay(Layer* pLayer) const noexcept;
		void OnEvent(IEvent& event) noexcept;

		virtual void Initialize() noexcept {}
		virtual void Update() noexcept {}
		virtual void ShutDown() noexcept {}

		void SubmitToMainThread(const std::function<void()>& func); //Determine usage?
		[[nodiscard]] ThreadPool& GetThreadPool() noexcept;
	private:
		void Initialize_Internal() noexcept;
		void Update_Internal() noexcept;
		void ShutDown_Internal() noexcept;

		void ExecuteMainThreadQueue() noexcept;

		void OnWindowClosedOrDestroyed() noexcept;
		void OnMouseInput(uint32 keyCode, bool isPressed) noexcept;
		void OnMouseMoved(uint32 x, uint32 y) noexcept;
		void OnMouseRaw(long x, long y) noexcept;
		void OnMouseScrolled(float scrollAmount) noexcept;
		void OnKeyInput(uint32 keyCode, bool pressed) noexcept;
		void OnWindowResizedOrMoved(uint32 width, uint32 height) noexcept;
	protected:
		Ref<GraphicsDevice> m_pGraphicsDevice = nullptr;
		Ref<Swapchain> m_pSwapchain = nullptr;
		UniquePtr<WindowEx> m_pWindow = nullptr;
	private:
		static Application* s_Instance;
		ThreadPool m_ThreadPool;

		ApplicationSpecification m_ApplicationSpecification;
		UniquePtr<ImguiLayer> m_pImGuiLayer = nullptr;
		bool m_IsRunning;

		std::queue<std::function<void()>> m_MainThreadFunctionQueue;
		std::mutex m_MainThreadFunctionQueueMutex;
	};

	//To be defined in client (runtime-project):
	[[nodiscard]] const std::unique_ptr<Application> CreateApplication() noexcept;
}