#include "Window.h"

namespace Relentless
{
	void Window::SetWndProcHook(WndProcHook aHook) noexcept
	{
		s_WndProcHook = aHook;
	}

	Window::Window(uint32 width, uint32 height) noexcept
	{
		RLS_ASSERT(width > 0u, "[Window::Window] Invalid Window Width.");
		RLS_ASSERT(height > 0u, "[Window::Window] Invalid Window Height.");

		WNDCLASSEX wc{};
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.hInstance = ::GetModuleHandleA(0);
		wc.hbrBackground = (HBRUSH)::GetStockObject(WHITE_BRUSH);
		wc.lpfnWndProc = WndProcStatic;
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		wc.lpszClassName = WINDOW_CLASS_NAME;
		wc.hCursor = ::LoadCursorA(nullptr, IDC_ARROW);
		RLS_VERIFY(::RegisterClassExA(&wc), "[Window::Window] Window Class Registration Failed.");

		const DWORD windowStyle = WS_POPUP | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU | WS_THICKFRAME;
		RECT windowRect = { 0, 0, (LONG)width, (LONG)height };
		::AdjustWindowRect(&windowRect, windowStyle, false);
	
		const Vector2i displayDimensions = GetDisplaySize();
		const int x = (displayDimensions.x - width) / 2;
		const int y = (displayDimensions.y - height) / 2;

		m_WindowHandle = CreateWindowExA(
			0,
			WINDOW_CLASS_NAME,
			"",
			windowStyle,
			x,
			y,
			windowRect.right - windowRect.left,
			windowRect.bottom - windowRect.top,
			nullptr,
			nullptr,
			::GetModuleHandleA(nullptr),
			this
		);
		RLS_VERIFY(m_WindowHandle, "[Window::Window] Window Creation Failed.");

		RAWINPUTDEVICE rawInputDevice;
		rawInputDevice.usUsagePage = 0x01;
		rawInputDevice.usUsage = 0x02;
		rawInputDevice.dwFlags = 0;
		rawInputDevice.hwndTarget = nullptr;
		RLS_VERIFY(::RegisterRawInputDevices(&rawInputDevice, 1u, sizeof(rawInputDevice)), "[Window::Window] Raw Input Device Registration Failed.");

		::ShowWindow(m_WindowHandle, SW_SHOWNORMAL);
		::UpdateWindow(m_WindowHandle);
	}

	Window::~Window() noexcept
	{
		::CloseWindow(m_WindowHandle);
		::UnregisterClassA(WINDOW_CLASS_NAME, ::GetModuleHandleA(nullptr));
	}

	Vector2i Window::GetDisplaySize() noexcept
	{
		const int displayWidth = ::GetSystemMetrics(SM_CXSCREEN);
		const int displayHeight = ::GetSystemMetrics(SM_CYSCREEN);
		return Vector2i(displayWidth, displayHeight);
	}

	WindowHandle Window::GetNativeWindow() const noexcept
	{
		return m_WindowHandle;
	}

	Vector2u Window::GetTopLeft() const noexcept
	{
		RECT rect;
		::GetWindowRect(::GetActiveWindow(), &rect);
		return Vector2u(rect.left, rect.top);
	}

	void Window::PollMessages() noexcept
	{
		MSG msg{};
		while (::PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessageA(&msg);
		}
	}

	LRESULT Window::WndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		Window* pThis = nullptr;
		if (message == WM_NCCREATE)
		{
			pThis = static_cast<Window*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
			SetWindowLongPtrA(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
		}
		else
		{
			pThis = reinterpret_cast<Window*>(GetWindowLongPtrA(hWnd, GWLP_USERDATA));
			return pThis->HandleMessages(hWnd, message, wParam, lParam);
		}
		return ::DefWindowProcA(hWnd, message, wParam, lParam);
	}

	LRESULT Window::HandleMessages(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		switch (msg)
		{
		case WM_LBUTTONDOWN:
		{
			const POINT point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			constexpr LONG titlebarHeight = 40u;
			const bool pointIsInTitlebar = point.y >= 0 && point.y <= titlebarHeight && point.x > 100;

			if (pointIsInTitlebar)
			{
				POINT screenPoint = point;
				::ClientToScreen(hWnd, &screenPoint);
				::ReleaseCapture();
				::SendMessageW(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(screenPoint.x, screenPoint.y));

				return 0;
			}
			break;
		}
		case WM_LBUTTONDBLCLK:
		{
			const POINT point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			constexpr LONG titlebarHeight = 40u;
			const bool pointIsInTitlebar = point.y >= 0 && point.y <= titlebarHeight && point.x > 100;

			if (pointIsInTitlebar)
			{
				const bool isMaximized = ::IsZoomed(hWnd) != 0;
				::ShowWindow(hWnd, isMaximized ? SW_RESTORE : SW_MAXIMIZE);
				return 0;
			}

			break;
		}
		}

		if (s_WndProcHook && s_WndProcHook(hWnd, msg, wParam, lParam))
			return true;

		switch (msg)
		{
		case WM_NCCALCSIZE:
		{
			//Make the client area cover the whole window including the title bar and borders.
			if (wParam)
				return 0;

			break;
		}
		case WM_NCHITTEST:
		{
			const LONG x = GET_X_LPARAM(lParam);
			const LONG y = GET_Y_LPARAM(lParam);

			RECT windowRect{};
			::GetWindowRect(hWnd, &windowRect);
			
			constexpr LONG borderWidth = 6;

			const bool onLeft = x >= windowRect.left && x < windowRect.left + borderWidth;
			const bool onRight = x < windowRect.right && x >= windowRect.right - borderWidth;
			const bool onTop = y >= windowRect.top && y < windowRect.top + borderWidth;
			const bool onBottom = y < windowRect.bottom && y >= windowRect.bottom - borderWidth;

			if (onTop && onLeft)
				return HTTOPLEFT;
			else if (onTop && onRight)
				return HTTOPRIGHT;
			else if (onBottom && onLeft)
				return HTBOTTOMLEFT;
			else if (onBottom && onRight)
				return HTBOTTOMRIGHT;
			else if (onLeft)
				return HTLEFT;
			else if (onRight)
				return HTRIGHT;
			else if (onTop)
				return HTTOP;
			else if (onBottom)
				return HTBOTTOM;

			return HTCLIENT;

		}
		case WM_CLOSE:
		case WM_DESTROY:
			OnCloseOrDestroy();
			break;
		case WM_ACTIVATE:
			OnFocusChanged(LOWORD(wParam) != WA_INACTIVE);
			break;
		case WM_SIZE:
		{
			// Save the new client area dimensions.
			const int newWidth = LOWORD(lParam);
			const int newHeight = HIWORD(lParam);
			const bool resized = newWidth != static_cast<int>(m_DisplayWidth) || newHeight != static_cast<int>(m_DisplayHeight);
			bool shouldResize = false;

			if (wParam == SIZE_MINIMIZED)
			{
				OnFocusChanged(false);
				m_Minimized = true;
				m_Maximized = false;
		}
			else if (wParam == SIZE_MAXIMIZED)
			{
				OnFocusChanged(true);
				m_Minimized = false;
				m_Maximized = true;
				shouldResize = true;
			}
			else if (wParam == SIZE_RESTORED)
			{
				// Restoring from minimized state?
				if (m_Minimized)
				{
					OnFocusChanged(true);
					m_Minimized = false;
					shouldResize = true;
				}
				// Restoring from maximized state?
				else if (m_Maximized)
				{
					OnFocusChanged(true);
					m_Maximized = false;
					shouldResize = true;
				}
				else if (!m_IsResizing) // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
				{
					shouldResize = true;
				}
			}

			if (shouldResize && resized)
			{
				m_DisplayWidth = LOWORD(lParam);
				m_DisplayHeight = HIWORD(lParam);
				OnResizeOrMove(m_DisplayWidth, m_DisplayHeight);
			}
			break;
		}
		case WM_MOUSEWHEEL:
		{
			const float mouseWheel = (float)GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
			OnMouseScroll(mouseWheel);
			break;
		}
		case WM_LBUTTONDOWN:
			OnMouseInput(VK_LBUTTON, true);
			break;
		case WM_MBUTTONDOWN:
			OnMouseInput(VK_MBUTTON, true);
			break;
		case WM_RBUTTONDOWN:
			OnMouseInput(VK_RBUTTON, true);
			break;
		case WM_LBUTTONUP:
			OnMouseInput(VK_LBUTTON, false);
			break;
		case WM_MBUTTONUP:
			OnMouseInput(VK_MBUTTON, false);
			break;
		case WM_RBUTTONUP:
			OnMouseInput(VK_RBUTTON, false);
			break;
		case WM_MOUSEMOVE:
			OnMouseMove((uint32_t)GET_X_LPARAM(lParam), (uint32_t)GET_Y_LPARAM(lParam));
			break;
		case WM_INPUT:
		{
			UINT size = 0u;
			if (::GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER)) == std::numeric_limits<UINT>::max())
				break;

			std::vector<char> rawBuffer;
			rawBuffer.resize(size);
			if (::GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, rawBuffer.data(), &size, sizeof(RAWINPUTHEADER)) != size)
				break;

			auto& ri = reinterpret_cast<const RAWINPUT&>(*rawBuffer.data());
			if (ri.header.dwType == RIM_TYPEMOUSE && (ri.data.mouse.lLastX != 0 || ri.data.mouse.lLastY != 0))
			{
				OnMouseRaw(ri.data.mouse.lLastX, ri.data.mouse.lLastY );
			}

			break;
		}
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			const bool isRepeat = (lParam & (1LL << 30)) != 0;
			if (!isRepeat)
				OnKeyInput((uint32)wParam, true);
			break;
		}
		case WM_KEYUP:
		case WM_SYSKEYUP:
			OnKeyInput((uint32)wParam, false);
			break;
		case WM_CHAR:
		{
			if (wParam < 256)
				OnCharInput((uint32)wParam);
			break;
		}
		case WM_ENTERSIZEMOVE:
		{
			OnFocusChanged(false);
			m_IsResizing = true;
			break;
		}
		case WM_EXITSIZEMOVE:
		{
			OnFocusChanged(true);
			RECT rect;
			::GetClientRect(hWnd, &rect);
			m_DisplayWidth = rect.right - rect.left;
			m_DisplayHeight = rect.bottom - rect.top;
			OnResizeOrMove(m_DisplayWidth, m_DisplayHeight);
			m_IsResizing = false;
			break;
		}
		}
		return ::DefWindowProc(hWnd, msg, wParam, lParam);
	}

	void Window::SetTitle(const char* pTitle) noexcept
	{
		::SetWindowTextA(m_WindowHandle, pTitle);
	}

	void Window::SetPosition(const Vector2u& newPosition) noexcept
	{
		::SetWindowPos(m_WindowHandle, nullptr, newPosition.x, newPosition.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}
}

