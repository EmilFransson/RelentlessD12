#include "Application.h"
#include "Assets/AssetManager.h"

#include "Callback/Callback.h"
#include "Core/Window.h"

#include "EventSystem/LayerStack.h"
#include "EventSystem/EventBus.h"
#include "EventSystem/MouseEvents.h"
#include "EventSystem/KeyboardEvents.h"

#include "Input/Mouse.h"
#include "Input/Keyboard.h"

#include "Graphics/RHI/RHI.h"
#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/ResourceViews.h"
#include "Graphics/RHI/Swapchain.h"

#include "Time.h"
#include "Threading/ThreadPool.h"

#include "Utility/SystemPaths.h"

namespace Relentless
{
	Application* Application::s_Instance = nullptr;

	void Application::OnWindowClosedOrDestroyed() noexcept
	{
		m_IsRunning = false;
	}

	void Application::OnMouseInput(uint32 keyCode, bool isPressed) noexcept
	{
		Mouse::UpdateState(keyCode, isPressed);
		const RLS_Button button = Mouse::KeyCodeToButton(keyCode);
		switch (button)
		{
		case RLS_Button::Left:
		{
			if (isPressed)
				PublishEvent<LeftMouseButtonPressedEvent>(Mouse::GetCursorPosition());
			else 
				PublishEvent<LeftMouseButtonReleasedEvent>(Mouse::GetCursorPosition());
			break;
		}
		case RLS_Button::Right:
		{
			if (isPressed)
				PublishEvent<RightMouseButtonPressedEvent>(Mouse::GetCursorPosition());
			else
				PublishEvent<RightMouseButtonReleasedEvent>(Mouse::GetCursorPosition());
			break;
		}
		case RLS_Button::Wheel:
		{
			if (isPressed)
				PublishEvent<MiddleMouseButtonPressedEvent>(Mouse::GetCursorPosition());
			else
				PublishEvent<MiddleMouseButtonReleasedEvent>(Mouse::GetCursorPosition());
			break;
		}
		default:
			break;
		}
	}

	void Application::OnMouseMoved(uint32 x, uint32 y) noexcept
	{
		const Vector2u coords(x, y);
		Mouse::OnMove(coords);
		PublishEvent<MouseMovedEvent>(coords);
	}

	void Application::OnMouseRaw(long x, long y) noexcept
	{
		const Vector2i delta(x, y);
		Mouse::OnRawDelta(delta);
		PublishEvent<RawMouseMoveEvent>(delta);
	}

	void Application::OnMouseScrolled(float delta) noexcept
	{
		Mouse::UpdateMouseWheel(delta);
		PublishEvent<MouseWheelScrolledEvent>(delta);
	}

	void Application::OnKeyInput(uint32 keyCode, bool pressed) noexcept
	{
		if (keyCode <= 256)
		{
			Keyboard::UpdateKeyState(keyCode, pressed);
			if (pressed)
				PublishEvent<KeyPressedEvent>((RLS_Key)keyCode);
			else
				PublishEvent<KeyReleasedEvent>((RLS_Key)keyCode);
		}
	}

	void Application::OnWindowResizedOrMoved(uint32 width, uint32 height) noexcept
	{
		RLS_CORE_INFO("Window Resized: {0}x{1}", width, height);
		m_pSwapchain->OnResizeOrMove(width, height);
	}

	Application& Application::Get() noexcept
	{
		return *s_Instance;
	}

	Application::Application(const ApplicationSpecification& applicationSpecification) noexcept
		: m_ApplicationSpecification{ applicationSpecification }
	{
		RLS_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;
	}

	Application::~Application() noexcept = default;

	GraphicsDevice* Application::GetGraphicsDevice() const noexcept
	{
		return m_pGraphicsDevice;
	}

	const UniquePtr<Window>& Application::GetWindow() const noexcept
	{
		return m_pWindow;
	}

	void Application::InitializeShutdownProcedure()
	{
		m_IsRunning = false;
	}

	void Application::Run() noexcept
	{
		Initialize_Internal();
		while (m_IsRunning)
		{
			PROFILE_FUNC;

			m_pWindow->PollMessages();
			Update_Internal();
		}
		ShutDown_Internal();
	}

	void Application::PushLayer(Layer* pLayer) const noexcept
	{
		LayerStack::Get().PushLayer(pLayer);
	}

	void Application::PushOverlay(Layer* pLayer) const noexcept
	{
		LayerStack::Get().PushOverlay(pLayer);
	}

	void Application::SubmitToMainThread(const std::function<void()>& func)
	{
		std::scoped_lock lock(m_MainThreadFunctionQueueMutex);
		m_MainThreadFunctionQueue.push(func);
	}

	ThreadPool& Application::GetThreadPool() noexcept
	{
		return *m_ThreadPool;
	}

	void Application::Initialize_Internal() noexcept
	{
		void* pScratchMemory = ::operator new(FrameScratchSize);
		g_FrameScratchArena.Init(pScratchMemory, FrameScratchSize);

		m_ThreadPool = std::make_unique<ThreadPool>();
		GraphicsDeviceOptions options;
		RLS_DEBUG_ONLY(options.UseDebugDevice = true);
		//options.UseGPUValidation = true;
		m_pGraphicsDevice = new GraphicsDevice(options);

		const Vector2i displaySize = Window::GetDisplaySize();
		m_pWindow = std::make_unique<Window>(uint32(displaySize.x * 0.85f), uint32(displaySize.y * 0.85f));
		m_pWindow->SetTitle(m_ApplicationSpecification.Name.c_str());

		m_pWindow->OnCloseOrDestroy.Connect(this, &Application::OnWindowClosedOrDestroyed);
		m_pWindow->OnMouseInput.Connect(this, &Application::OnMouseInput);
		m_pWindow->OnMouseMove.Connect(this, &Application::OnMouseMoved);
		m_pWindow->OnMouseRaw.Connect(this, &Application::OnMouseRaw);
		m_pWindow->OnMouseScroll.Connect(this, &Application::OnMouseScrolled);
		m_pWindow->OnKeyInput.Connect(this, &Application::OnKeyInput);
		m_pWindow->OnResizeOrMove.Connect(this, &Application::OnWindowResizedOrMoved);

		m_pSwapchain = RLS_NEW Swapchain(m_pGraphicsDevice, 3, m_pWindow->GetNativeWindow());

		SystemPaths::Initialize();

		Initialize();
		m_IsRunning = true;
	}

	void Application::Update_Internal() noexcept
	{
		if (!m_IsRunning)
			return;

		g_FrameScratchArena.Reset();

		Time::Tick();

		FlushMainThreadQueue();

		{
			PROFILE_SCOPE("Application::Update_Internal::OnLayersUpdate");

			for (auto& pLayer : LayerStack::Get())
				pLayer->OnUpdate(Time::GetDeltaTime());
		}

		{
			PROFILE_SCOPE("Application::Update_Internal::Update");
			Update();
		}

		{
			PROFILE_SCOPE("Application::Update_Internal::OnImGuiRender");
			CommandContext* pContext = m_pGraphicsDevice->AllocateCommandContext();
			pContext->InsertResourceBarrier(m_pSwapchain->GetBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			pContext->Execute();
			
			CommandContext* pCommandContext = m_pGraphicsDevice->AllocateCommandContext();
			UIRenderBegin(pCommandContext);

			for (auto& pLayer : LayerStack::Get())
				pLayer->OnImGuiRender();

			UIRenderEnd(pCommandContext);
			
			pCommandContext->Execute();
		}

		{
			CommandContext* pCommandContext = m_pGraphicsDevice->AllocateCommandContext();
			pCommandContext->InsertResourceBarrier(m_pSwapchain->GetBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
			pCommandContext->Execute();
		}

		Mouse::Update();
		Keyboard::Update();

		m_pSwapchain->Present();
		m_pGraphicsDevice->TickFrame();
	}


	void Application::ShutDown_Internal() noexcept
	{
		ShutDown();

		LayerStack::Get().PopAllLayers();
		m_pGraphicsDevice->IdleGPU();

		AssetManager::Shutdown();
	}

	void Application::FlushMainThreadQueue() noexcept
	{
		std::scoped_lock lock(m_MainThreadFunctionQueueMutex);

		while (m_MainThreadFunctionQueue.size() > 0)
		{
			auto& func = m_MainThreadFunctionQueue.front();
			func();

			m_MainThreadFunctionQueue.pop();
		}
	}
}