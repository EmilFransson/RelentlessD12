#pragma once
#include "Events\EventPublisher.h"
#include "Events\MouseEvents.h"
namespace Relentless
{
	class Window : public EventPublisher
	{
	public:
		static const Window& Get() noexcept;
		static void Initialize(const std::string& windowTitle = "WindowTitle", const uint32_t width = 1280u, const uint32_t height = 720u);
		[[nodiscard]] static constexpr uint32_t GetWidth() noexcept { return m_Width; }
		[[nodiscard]] static constexpr uint32_t GetHeight() noexcept { return m_Height; }
		[[nodiscard]] static constexpr std::pair<uint32_t, uint32_t> GetWindowResolution() noexcept { return std::pair<uint32_t, uint32_t>(m_Width, m_Height); }
		static void OnUpdate();
		static void HideMouseCursor() noexcept;
		static void ShowMouseCursor() noexcept;
		static void ConfineMouseCursor(float left, float right, float bottom, float top) noexcept;
		static void FreeMouseCursor() noexcept;
	private:
		Window() noexcept = default;
		~Window() noexcept = default;
		static LRESULT HandleMessages(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
#pragma region "Deleted Ctors"
		Window(const Window& otherWindow) = delete;
		Window& operator=(const Window& otherWindow) = delete;
		Window(const Window&& otherWindow) = delete;
		Window&& operator=(const Window&& otherWindow) = delete;
#pragma endregion
	private:
		static const Window m_sInstance;
		static std::string m_Title;
		static std::string m_ClassName;
		static uint32_t m_Width;
		static uint32_t m_Height;
		static HWND m_WindowHandle;
		static MSG m_WindowMessage;
		static bool m_IsVSync;
		static BOOL m_IsVisible;
		static bool m_CursorVisible;
		static uint32_t m_MouseX;
		static uint32_t m_MouseY;
		static bool disabled;
	};
}