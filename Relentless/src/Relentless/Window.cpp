#include "Window.h"
#include "Utility.h"
#include "Input\Mouse.h"
#include "Input\Keyboard.h"
//#include "Renderer\Graphics.h"
//#include "DXDebug.h"
//#include "imgui.h"

//extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Relentless
{
	const Window Window::m_sInstance;
	std::string Window::m_Title = "?";
	std::string Window::m_ClassName = "RelentlessWindowClass";
	uint32_t Window::m_Width = 0u;
	uint32_t Window::m_Height = 0u;
	HWND Window::m_WindowHandle;
	MSG Window::m_WindowMessage = { 0 };
	bool Window::m_IsVSync = false;
	BOOL Window::m_IsVisible = 0;
	bool Window::m_CursorVisible = false;
	uint32_t Window::m_MouseX = 0u;
	uint32_t Window::m_MouseY = 0u;

	const Window& Window::Get() noexcept
	{
		return m_sInstance;
	}

	void Window::Initialize(const std::string& windowTitle, const uint32_t width, const uint32_t height)
	{
		WNDCLASSEX windowClass = { 0 };									//[https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-wndclassexa]
		windowClass.cbSize = sizeof(WNDCLASSEX);						//Size in bytes of structure.
		windowClass.style = CS_OWNDC;									//Every window get its own device context.
		windowClass.lpfnWndProc = HandleMessages;						//Long pointer to function handling the window messages (setup in this case).
		windowClass.cbClsExtra = 0;										//Extra bytes allocated to the window class structure.
		windowClass.cbWndExtra = 0;										//Extra bytes allocated following the window instance.
		windowClass.hInstance = ::GetModuleHandle(nullptr); 				//Handle to the instance that contains the window procedure for class.
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
		/*Calculate the rectangle equivalent to the client region of the window based
		  on the window styles included for the window.*/
		RECT windowRect = { 0 };
		windowRect.left = 0;
		windowRect.right = width + windowRect.left;
		windowRect.top = 0;
		windowRect.bottom = windowRect.top + height;
		::AdjustWindowRect(&windowRect, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_SIZEBOX, FALSE);

		//Calculate dimension-agnostic start position for window:
		int windowStartPositionX = static_cast<int>((::GetSystemMetrics(SM_CXSCREEN) *
			(((::GetSystemMetrics(SM_CXSCREEN) - width)
				/ (float)::GetSystemMetrics(SM_CXSCREEN)) / 2)));

		int windowStartPositionY = static_cast<int>((::GetSystemMetrics(SM_CYSCREEN) *
			(((::GetSystemMetrics(SM_CYSCREEN) - height)
				/ (float)::GetSystemMetrics(SM_CYSCREEN)) / 2)));
		//[https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexa]
		m_WindowHandle = CreateWindowEx(0,										//Extended window styles (bits).
			m_ClassName.c_str(),					//The window class name.
			windowTitle.c_str(),					//The window title.
			WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_SIZEBOX,	//The window styles defining the window appearance.
			windowStartPositionX,					//Start x-position for window.
			windowStartPositionY,					//Start y-position for window.
			windowRect.right - windowRect.left,		//Width of window.
			windowRect.bottom - windowRect.top,		//Height of window.
			nullptr,								//Handle to window parent.								
			nullptr,								//Handle to menu.
			::GetModuleHandle(nullptr),				//Handle to the instance that contains the window procedure for class. 
			nullptr);									//Pointer to value passed to the window through CREATESTRUCT structure.
		RLS_ASSERT(m_WindowHandle != nullptr, "Failed to create window.");

		m_Title = windowTitle;
		m_Width = width;
		m_Height = height;
		RLS_CORE_INFO("Created Window: {0} ({1},{2})", m_Title, m_Width, m_Height);

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
	}

	void Window::OnUpdate()
	{
		//PROFILE_FUNC;

		while (PeekMessage(&m_WindowMessage, nullptr, 0u, 0u, PM_REMOVE))
		{
			TranslateMessage(&m_WindowMessage);
			DispatchMessage(&m_WindowMessage);
		}
		//HR_I(Graphics::GetSwapChain()->Present(0u, 0u));
		//CHECK_STD(Graphics::GetContext()->DiscardView(Graphics::GetBackBufferRTV().Get()));
		//CHECK_STD(Graphics::GetContext()->DiscardView(Graphics::GetDepthStencilView().Get()));
	}

	void Window::HideMouseCursor() noexcept
	{
		auto mouseCoords = Mouse::GetCursorCoordinates();
		m_MouseX = mouseCoords.x;
		m_MouseY = mouseCoords.y;

		while (ShowCursor(false) >= 0);
		m_CursorVisible = false;
	}

	void Window::ShowMouseCursor() noexcept
	{
		while (ShowCursor(true) < 0);
		m_CursorVisible = true;

		RECT rect = {};
		GetWindowRect(m_WindowHandle, &rect);
		m_MouseX += (rect.left + 8);
		m_MouseY += (rect.top + 31);
		SetCursorPos(m_MouseX, m_MouseY);
	}

	void Window::ConfineMouseCursor(float left, float right, float bottom, float top) noexcept
	{
		RECT rect = {};
		rect.left = static_cast<LONG>(left);
		rect.right = static_cast<LONG>(right);
		rect.bottom = static_cast<LONG>(bottom);
		rect.top = static_cast<LONG>(top);
		ClipCursor(&rect);
	}

	void Window::FreeMouseCursor() noexcept
	{
		ClipCursor(nullptr);
	}

	LRESULT Window::HandleMessages(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		//if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		//	return true;

		switch (msg)
		{
		case WM_MOVE:
		{
			break;
		}
		case WM_MOUSEMOVE:
		{
			Mouse::Get().OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

			break;
		}
		case WM_INPUT:
		{
			UINT size = 0u;
			if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER)) == -1)
				break;

			std::vector<char> rawBuffer;
			rawBuffer.resize(size);
			if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, rawBuffer.data(), &size, sizeof(RAWINPUTHEADER)) != size)
				break;

			auto& ri = reinterpret_cast<const RAWINPUT&>(*rawBuffer.data());
			if (ri.header.dwType == RIM_TYPEMOUSE && (ri.data.mouse.lLastX != 0 || ri.data.mouse.lLastY != 0))
			{
				Mouse::OnRawDelta(ri.data.mouse.lLastX, ri.data.mouse.lLastY);
			}

			break;
		}
		case WM_LBUTTONDOWN:
		{
			Mouse::Get().OnButtonPressed(RLS_MOUSE::Left);
			break;
		}
		case WM_LBUTTONUP:
		{
			Mouse::Get().OnButtonReleased(RLS_MOUSE::Left);
			break;
		}
		case WM_RBUTTONDOWN:
		{
			Mouse::Get().OnButtonPressed(RLS_MOUSE::Right);
			break;
		}
		case WM_RBUTTONUP:
		{
			Mouse::Get().OnButtonReleased(RLS_MOUSE::Right);
			break;
		}
		case WM_MBUTTONDOWN:
		{
			Mouse::Get().OnButtonPressed(RLS_MOUSE::Wheel);
			break;
		}
		case WM_MBUTTONUP:
		{
			Mouse::Get().OnButtonReleased(RLS_MOUSE::Wheel);
			break;
		}
		case WM_MOUSEWHEEL:
		{
			Mouse::Get().OnWheelScrolled(GET_WHEEL_DELTA_WPARAM(wParam));
			break;
		}
		case WM_KEYDOWN:
		{
			BOOL repeatFlag = (HIWORD(lParam) & KF_REPEAT) == KF_REPEAT;
			if (repeatFlag == FALSE)
				Keyboard::Get().OnKeyDown(wParam);
			else
				if (repeatFlag && Keyboard::Get().IsRepeatEnabled())
					Keyboard::Get().OnKeyDown(wParam);
			break;
		}
		case WM_KEYUP:
		{
			Keyboard::Get().OnKeyRelease(wParam);
			break;
		}
		case WM_CLOSE:
		{
			PublishEvent(WindowCloseEvent());
			break;
		}
		case WM_SETFOCUS:
		{
			PublishEvent(WindowGainedFocusEvent());
			break;
		}
		case WM_KILLFOCUS:
		{
			PublishEvent(WindowLostFocusEvent());
			break;
		}
		case WM_SIZE:
		{
			m_Width = LOWORD(lParam);
			m_Height = HIWORD(lParam);
			//if (Graphics::IsInitialized())
			//{
			//	Graphics::CreateWindowSizeDependentResources(m_Width, m_Height);
			//}
			break;
		}
		}
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}