#pragma once
#include "Core/DLLExport.h"
#include "Callback/Broadcaster.h"

namespace Relentless
{
	using WindowHandle = HWND;

	constexpr const char* WINDOW_CLASS_NAME = "RelentlessWindowClass";

	class RLS_API Window
	{
	public:
		using WndProcHook = LRESULT(*)(HWND, UINT, WPARAM, LPARAM);
		static void SetWndProcHook(WndProcHook aHook) noexcept;

		Window(uint32 width, uint32 height) noexcept;
		~Window() noexcept;

		NO_DISCARD static Vector2i GetDisplaySize() noexcept;
		NO_DISCARD WindowHandle GetNativeWindow() const noexcept;
		NO_DISCARD Vector2u GetTopLeft() const noexcept;
		void PollMessages() noexcept;
		void SetTitle(const char* pTitle) noexcept;
		void SetPosition(const Vector2u& newPosition) noexcept;

		Broadcaster<void(bool maximized)> OnFocusChanged;
		Broadcaster<void(uint32 width, uint32 height)> OnResizeOrMove;
		Broadcaster<void(uint32 button, bool pressed)> OnMouseInput;
		Broadcaster<void(float scrollValue)> OnMouseScroll;
		Broadcaster<void(uint32 x, uint32 y)> OnMouseMove;
		Broadcaster<void(long x, long y)> OnMouseRaw;
		Broadcaster<void(uint32 key, bool pressed)> OnKeyInput;
		Broadcaster<void(uint32 key)> OnCharInput;
		Broadcaster<void()> OnCloseOrDestroy;

	private:
		static LRESULT WndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		LRESULT HandleMessages(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
	private:
		WindowHandle m_WindowHandle = nullptr;
		static inline WndProcHook s_WndProcHook = nullptr;
		uint32 m_DisplayWidth = 0u;
		uint32 m_DisplayHeight = 0u;
		bool m_IsResizing = false;
		bool m_Minimized = false;
		bool m_Maximized = false;
	};
}