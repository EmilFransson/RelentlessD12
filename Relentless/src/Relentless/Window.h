#pragma once
#include "Events\EventPublisher.h"
#include "Events\MouseEvents.h"
#include "Graphics\DescriptorHeap.h"
namespace Relentless
{
	struct BackBuffer
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> pBackBuffer;
		DescriptorHandle Handle;
	};
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
		static void Present() noexcept;
		[[nodiscard]] static constexpr std::vector<BackBuffer>& GetBackBuffers() noexcept { return m_BackBuffers; }
		[[nodiscard]] static uint32_t GetCurrentBackbufferIndex() noexcept { return m_pSwapChain->GetCurrentBackBufferIndex(); }
	private:
		Window() noexcept = default;
		~Window() noexcept = default;
		static LRESULT HandleMessages(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
		static void CreateSwapchain() noexcept;
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
		static Microsoft::WRL::ComPtr<IDXGISwapChain4> m_pSwapChain;
		static std::unique_ptr<DescriptorHeap> m_pBackBufferRTVHeap;
		static uint8_t m_NrOfBackBuffers;
		static std::vector<BackBuffer> m_BackBuffers;
	};
}