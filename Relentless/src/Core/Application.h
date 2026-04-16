#pragma once
#include "DLLExport.h"
#include "EventSystem/EventPublisher.h"
#include "Memory/LinearArena.h"

namespace Relentless
{
	static constexpr size_t FrameScratchSize = 4u * 1024u * 1024u;
	inline static LinearArena g_FrameScratchArena;

	struct ApplicationSpecification
	{
		std::string Name;
	};
	
	class CommandContext;
	class GraphicsDevice;
	class Layer;
	class Swapchain;
	class ThreadPool;
	class Window;

	class RLS_API Application : public EventPublisher
	{
	public:
		NO_DISCARD static Application& Get() noexcept;
		Application(const ApplicationSpecification& applicationSpecification) noexcept;
		virtual ~Application() noexcept;

		NO_DISCARD GraphicsDevice* GetGraphicsDevice() const noexcept;
		NO_DISCARD const UniquePtr<Window>& GetWindow() const noexcept;

		void InitializeShutdownProcedure();

		void Run() noexcept;
		void PushLayer(Layer* pLayer) const noexcept;
		void PushOverlay(Layer* pLayer) const noexcept;

		//Should be protected?
		virtual void Initialize() noexcept {}
		virtual void Update() noexcept {}
		virtual void UIRenderBegin(CommandContext*) noexcept {}
		virtual void UIRenderEnd(CommandContext*) noexcept {}
		virtual void ShutDown() noexcept {}
		
		void SubmitToMainThread(const std::function<void()>& func);
		NO_DISCARD ThreadPool& GetThreadPool() noexcept;
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
		UniquePtr<Window> m_pWindow;
	private:
		static Application* s_Instance;
		UniquePtr<ThreadPool> m_ThreadPool;

		ApplicationSpecification m_ApplicationSpecification;
		bool m_IsRunning;

		std::queue<std::function<void()>> m_MainThreadFunctionQueue;
		std::mutex m_MainThreadFunctionQueueMutex;
	};

	//To be defined in client (runtime-project):
	NO_DISCARD UniquePtr<Application> CreateApplication() noexcept;
}