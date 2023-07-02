#include "Window.h"
#include "../EventSystem/MouseEvents.h"
#include "../EventSystem/KeyboardEvents.h"
#include "../EventSystem/WindowEvents.h"
#include "Utility.h"
#include "../Input/Mouse.h"
#include "../Input/Keyboard.h"
#include "../Graphics/D3D12Core.h"
#include "../Graphics/MemoryManager.h"
#include "../../vendor/includes/ImGUI/imgui.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Relentless
{
	std::string Window::m_Title = "?";
	std::string Window::m_ClassName = "RelentlessWindowClass";
	uint32_t Window::m_Width = 0u;
	uint32_t Window::m_Height = 0u;
	UINT Window::m_WindowStyle{ 0u };
	HWND Window::m_WindowHandle{nullptr};
	RECT Window::m_ClientRect{};
	RECT Window::m_NonClientRect{};
	MSG Window::m_WindowMessage = { 0 };
	BOOL Window::m_IsVisible = 0;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> Window::m_pSwapChain{ nullptr };
	uint8_t Window::m_NrOfBackBuffers{ 2u };
	std::vector<BackBuffer> Window::m_BackBuffers;
	bool Window::m_IsResizing{ false };
	bool Window::m_IsFullScreen{ false };

	void Window::Initialize(const std::string& windowTitle, const uint32_t width, const uint32_t height)
	{
		WNDCLASSEX windowClass = { 0 };									//[https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-wndclassexa]
		windowClass.cbSize = sizeof(WNDCLASSEX);						//Size in bytes of structure.
		windowClass.style = CS_OWNDC;									//Every window get its own device context.
		windowClass.lpfnWndProc = HandleMessages;						//Long pointer to function handling the window messages (setup in this case).
		windowClass.cbClsExtra = 0;										//Extra bytes allocated to the window class structure.
		windowClass.cbWndExtra = 0;										//Extra bytes allocated following the window instance.
		windowClass.hInstance = ::GetModuleHandle(nullptr); 			//Handle to the instance that contains the window procedure for class.
		windowClass.hIcon = nullptr;									//Handle to the class icon.
		windowClass.hCursor = nullptr;									//Handle to the class cursor.
		windowClass.hbrBackground = nullptr;							//Handle to the class background brush.
		windowClass.lpszMenuName = nullptr;								//Pointer to null-terminated character string specifying resource name of class menu.
		windowClass.lpszClassName = m_ClassName.c_str();				//Window class name
#if defined(RLS_DEBUG)
		int result = static_cast<int>(::RegisterClassEx(&windowClass));
		RLS_ASSERT(result != 0, "Failed to register window class.");
#else
		::RegisterClassEx(&windowClass);
#endif
		m_WindowStyle = WS_OVERLAPPEDWINDOW;

		/*Calculate the rectangle equivalent to the client region of the window based
		  on the window styles included for the window.*/
		RECT windowRect = { 0 };
		windowRect.left = 0;
		windowRect.right = width + windowRect.left;
		windowRect.top = 0;
		windowRect.bottom = windowRect.top + height;
		::AdjustWindowRect(&windowRect, m_WindowStyle, FALSE);

		//Calculate dimension-agnostic start position for window:
		int windowStartPositionX = static_cast<int>((::GetSystemMetrics(SM_CXSCREEN) *
			(((::GetSystemMetrics(SM_CXSCREEN) - width)
				/ (float)::GetSystemMetrics(SM_CXSCREEN)) / 2)));

		int windowStartPositionY = static_cast<int>((::GetSystemMetrics(SM_CYSCREEN) *
			(((::GetSystemMetrics(SM_CYSCREEN) - height)
				/ (float)::GetSystemMetrics(SM_CYSCREEN)) / 2)));
		//[https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexa]
		m_WindowHandle = CreateWindowEx(0,										//Extended window styles (bits).
			m_ClassName.c_str(),												//The window class name.
			windowTitle.c_str(),												//The window title.
			m_WindowStyle,														//The window styles defining the window appearance.
			windowStartPositionX,												//Start x-position for window.
			windowStartPositionY,												//Start y-position for window.
			windowRect.right - windowRect.left,									//Width of window.
			windowRect.bottom - windowRect.top,									//Height of window.
			nullptr,															//Handle to window parent.								
			nullptr,															//Handle to menu.
			::GetModuleHandle(nullptr),											//Handle to the instance that contains the window procedure for class. 
			nullptr);															//Pointer to value passed to the window through CREATESTRUCT structure.
		RLS_ASSERT(m_WindowHandle != nullptr, "Failed to create window.");

		m_Title = windowTitle;
		m_Width = width;
		m_Height = height;
		RLS_CORE_INFO("Created Window: {0} ({1},{2})", m_Title, m_Width, m_Height);

		CreateSwapchain();

		RAWINPUTDEVICE rawInputDevice;
		rawInputDevice.usUsagePage = 0x01;
		rawInputDevice.usUsage = 0x02;
		rawInputDevice.dwFlags = 0;
		rawInputDevice.hwndTarget = nullptr;
#if defined(RLS_DEBUG)
		if (::RegisterRawInputDevices(&rawInputDevice, 1u, sizeof(rawInputDevice)) == FALSE)
			RLS_ASSERT(false, "Failed to register mouse device for raw input.");
#else		
		::RegisterRawInputDevices(&rawInputDevice, 1u, sizeof(rawInputDevice));
#endif

		m_IsVisible = ::ShowWindow(m_WindowHandle, SW_SHOWNORMAL);
		::UpdateWindow(m_WindowHandle);
	}

	void Window::OnUpdate()
	{
		while (::PeekMessage(&m_WindowMessage, nullptr, 0u, 0u, PM_REMOVE))
		{
			::TranslateMessage(&m_WindowMessage);
			::DispatchMessage(&m_WindowMessage);
		}
		if (m_IsResizing)
		{
			PublishEvent<WindowResizedEvent>(Vector2u{ m_Width, m_Height });
			m_IsResizing = false;
		}
	}

	void Window::Present() noexcept
	{
		UINT presentFlags = !m_IsFullScreen ? DXGI_PRESENT_ALLOW_TEARING : 0;
		DXCall(m_pSwapChain->Present(0u, presentFlags));
	}

	void Window::ToggleFullScreen() noexcept
	{
		if (m_IsFullScreen)
		{
			// Restore the window's attributes and size.
			::SetWindowLong(m_WindowHandle, GWL_STYLE, m_WindowStyle);

			::SetWindowPos(
				m_WindowHandle,
				HWND_NOTOPMOST,
				m_NonClientRect.left,
				m_NonClientRect.top,
				m_NonClientRect.right - m_NonClientRect.left,
				m_NonClientRect.bottom - m_NonClientRect.top,
				SWP_FRAMECHANGED | SWP_NOACTIVATE);

			::ShowWindow(m_WindowHandle, SW_NORMAL);
		}
		else
		{
			// Save the old window rect so we can restore it when exiting fullscreen mode.
			::GetWindowRect(m_WindowHandle, &m_NonClientRect);

			// Make the window borderless so that the client area can fill the screen.
			::SetWindowLong(m_WindowHandle, GWL_STYLE, m_WindowStyle & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME));

			RECT fullscreenWindowRect;
		
			// Get the settings of the display on which the app's window is currently displayed
			Microsoft::WRL::ComPtr<IDXGIOutput> pOutput;
			DXCall(m_pSwapChain->GetContainingOutput(&pOutput));
			DXGI_OUTPUT_DESC Desc;
			DXCall(pOutput->GetDesc(&Desc));
			fullscreenWindowRect = Desc.DesktopCoordinates;

			::SetWindowPos(
				m_WindowHandle,
				HWND_TOPMOST,
				fullscreenWindowRect.left,
				fullscreenWindowRect.top,
				fullscreenWindowRect.right,
				fullscreenWindowRect.bottom,
				SWP_FRAMECHANGED | SWP_NOACTIVATE);


			::ShowWindow(m_WindowHandle, SW_MAXIMIZE);
		}

		m_IsFullScreen = !m_IsFullScreen;
	}

	LRESULT Window::HandleMessages(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		if (::ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
			return true;

		switch (msg)
		{
		case WM_MOVE:
		{
			POINT centerPointRelativeToScreen{ 0,0 };
			#if defined RLS_DEBUG
			RLS_ASSERT(::GetWindowRect(hWnd, &m_NonClientRect) != 0, "Failed to retrieve window client rectangle.");
			RLS_ASSERT(::ClientToScreen(hWnd, &centerPointRelativeToScreen) != 0, "Failed to retrieve window non client center point.");
			#else
			::GetWindowRect(hWnd, &m_NonClientRect);
			::ClientToScreen(hWnd, &centerPointRelativeToScreen);
			#endif

			const uint32_t invisibleResizeBorderSize = (centerPointRelativeToScreen.x - m_NonClientRect.left);
			m_ClientRect.left = centerPointRelativeToScreen.x;
			m_ClientRect.top = centerPointRelativeToScreen.y;
			m_ClientRect.right = m_NonClientRect.right - invisibleResizeBorderSize;
			m_ClientRect.bottom = m_NonClientRect.bottom - invisibleResizeBorderSize;

			break;
		}
		case WM_CLOSE:
		{
			PublishEvent<WindowClosedEvent>();
			break;
		}
		case WM_SETFOCUS:
		{
			PublishEvent<WindowGainedFocusEvent>();
			break;
		}
		case WM_KILLFOCUS:
		{
			PublishEvent<WindowLostFocusEvent>();
			break;
		}
		case WM_SIZE:
		{
			m_Width = LOWORD(lParam);
			m_Height = HIWORD(lParam);
			m_IsResizing = true;
			POINT centerPointRelativeToScreen{ 0,0 };
#if defined RLS_DEBUG
			RLS_ASSERT(::GetWindowRect(hWnd, &m_NonClientRect) != 0, "Failed to retrieve window client rectangle.");
			RLS_ASSERT(::ClientToScreen(hWnd, &centerPointRelativeToScreen) != 0, "Failed to retrieve window non client center point.");
#else
			::GetWindowRect(hWnd, &m_NonClientRect);
			::ClientToScreen(hWnd, &centerPointRelativeToScreen);
#endif
			const uint32_t invisibleResizeBorderSize = (centerPointRelativeToScreen.x - m_NonClientRect.left);
			m_ClientRect.left = centerPointRelativeToScreen.x;
			m_ClientRect.top = centerPointRelativeToScreen.y;
			m_ClientRect.right = m_NonClientRect.right - invisibleResizeBorderSize;
			m_ClientRect.bottom = m_NonClientRect.bottom - invisibleResizeBorderSize;

			break;
		}
		case WM_MOUSEMOVE:
		case WM_INPUT:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		{
			Mouse::OnWindowsEvent(msg, lParam, wParam);
			break;
		}
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			Keyboard::OnWindowsEvent(msg, lParam, wParam);
			break;
		}
		case WM_SYSKEYDOWN:
		{
			// Handle ALT+ENTER:
			if ((wParam == VK_RETURN) && (lParam & (1 << 29)))
			{
				ToggleFullScreen();
				return 0;
			}
			Keyboard::OnWindowsEvent(msg, lParam, wParam);
			break;
		}
		}
		return ::DefWindowProc(hWnd, msg, wParam, lParam);
	}

	void Window::CreateSwapchain() noexcept
	{
		Microsoft::WRL::ComPtr<IDXGIFactory7> pFactory{ nullptr };
	#if defined(RLS_DEBUG)
		DXCall(::CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&pFactory)));
	#else
		::CreateDXGIFactory2(0u, IID_PPV_ARGS(&pFactory));
	#endif

		DXGI_SWAP_CHAIN_DESC1 swapChainDescriptor{};
		swapChainDescriptor.Width = m_Width;
		swapChainDescriptor.Height = m_Height;
		swapChainDescriptor.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
		swapChainDescriptor.Stereo = false;
		swapChainDescriptor.SampleDesc = { 1u, 0u };
		swapChainDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDescriptor.BufferCount = m_NrOfBackBuffers;
		swapChainDescriptor.Scaling = DXGI_SCALING_STRETCH;
		swapChainDescriptor.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDescriptor.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapChainDescriptor.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

		Microsoft::WRL::ComPtr<IDXGISwapChain1> pTempSwapChain{ nullptr };
		DXCall(pFactory->CreateSwapChainForHwnd
		(
			D3D12Core::GetCommandQueue().Get(),
			m_WindowHandle,
			&swapChainDescriptor,
			nullptr,
			nullptr,
			&pTempSwapChain
		));
		DXCall(pTempSwapChain->QueryInterface(IID_PPV_ARGS(&m_pSwapChain)));
	
		m_BackBuffers.reserve(m_NrOfBackBuffers);
		Finalize();
	}

	void Window::Resize() noexcept
	{
		for (uint8_t i{ 0u }; i < m_BackBuffers.size(); ++i)
		{
			MemoryManager::Get().DestroyDescriptorHandle(m_BackBuffers[i].Handle);
		}
		m_BackBuffers.clear();
		DXCall(m_pSwapChain->ResizeBuffers(m_NrOfBackBuffers, m_Width, m_Height, DXGI_FORMAT_R10G10B10A2_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING));

		Finalize();
		RLS_CORE_INFO("Resized window with [width, height]=[{0},{1}]", m_Width, m_Height);
	}

	void Window::Finalize() noexcept
	{
		DXCall(m_pSwapChain->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709));

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		for (uint32_t i{ 0u }; i < m_NrOfBackBuffers; ++i)
		{
			BackBuffer backBuffer{};

			DXCall(m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer.pBackBuffer)));
			backBuffer.Handle = MemoryManager::Get().CreateDescriptorHandle(DescriptorHandleType::RTV);

			DXCall_STD(D3D12Core::GetDevice()->CreateRenderTargetView(backBuffer.pBackBuffer.Get(), &rtvDesc, backBuffer.Handle.CPUHandle));

			NAME_D12_OBJECT_INDEXED(backBuffer.pBackBuffer, L"Back buffer", i);
			m_BackBuffers.emplace_back(std::move(backBuffer));
		}
	}
}