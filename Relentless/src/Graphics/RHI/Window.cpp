#include "Window.h"
#include "../../../vendor/includes/ImGUI/imgui.h"
#include "../../../vendor/includes/ImGUI/backends/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Relentless
{
	WindowEx::WindowEx(uint32 width, uint32 height) noexcept
	{
		RLS_ASSERT(width > 0u, "[Window::Window] Invalid Window Width.");
		RLS_ASSERT(height > 0u, "[Window::Window] Invalid Window Height.");

		ImGui_ImplWin32_EnableDpiAwareness();

		WNDCLASSEX wc{};
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_OWNDC;
		wc.hInstance = ::GetModuleHandleA(0);
		wc.hbrBackground = (HBRUSH)::GetStockObject(WHITE_BRUSH);
		wc.lpfnWndProc = WndProcStatic;
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpszClassName = WINDOW_CLASS_NAME;
		wc.hCursor = ::LoadCursorA(nullptr, IDC_ARROW);
		RLS_VERIFY(::RegisterClassExA(&wc), "[Window::Window] Window Class Registration Failed.");

		const DWORD windowStyle = WS_OVERLAPPEDWINDOW;
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

	WindowEx::~WindowEx() noexcept
	{
		::CloseWindow(m_WindowHandle);
		::UnregisterClassA(WINDOW_CLASS_NAME, ::GetModuleHandleA(nullptr));
	}

	Vector2i WindowEx::GetDisplaySize() noexcept
	{
		const int displayWidth = ::GetSystemMetrics(SM_CXSCREEN);
		const int displayHeight = ::GetSystemMetrics(SM_CYSCREEN);
		return Vector2i(displayWidth, displayHeight);
	}

	WindowHandle WindowEx::GetNativeWindow() const noexcept
	{
		return m_WindowHandle;
	}

	void WindowEx::PollMessages() noexcept
	{
		MSG msg{};
		while (::PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessageA(&msg);
		}
	}

	LRESULT WindowEx::WndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		WindowEx* pThis = nullptr;
		if (message == WM_NCCREATE)
		{
			pThis = static_cast<WindowEx*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
			SetWindowLongPtrA(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
		}
		else
		{
			pThis = reinterpret_cast<WindowEx*>(GetWindowLongPtrA(hWnd, GWLP_USERDATA));
			return pThis->HandleMessages(hWnd, message, wParam, lParam);
		}
		return ::DefWindowProcA(hWnd, message, wParam, lParam);
	}

	LRESULT WindowEx::HandleMessages(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		if (::ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
			return true;

		switch (msg)
		{
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
			const bool resized = newWidth != m_DisplayWidth || newHeight != m_DisplayHeight;
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
			if (::GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER)) == -1)
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
			OnKeyInput((uint32)wParam, true);
			break;
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

	void WindowEx::SetTitle(const char* pTitle) noexcept
	{
		::SetWindowTextA(m_WindowHandle, pTitle);
	}
}

