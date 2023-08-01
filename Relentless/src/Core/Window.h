#pragma once
#include "../EventSystem/EventPublisher.h"
#include "../Graphics/DescriptorHeap.h"
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
		static void Initialize(const std::string& windowTitle = "WindowTitle", const uint32_t width = 1280u, const uint32_t height = 720u);
		[[nodiscard]] static constexpr uint32_t GetWidth() noexcept { return m_Width; }
		[[nodiscard]] static constexpr uint32_t GetHeight() noexcept { return m_Height; }
		[[nodiscard]] static constexpr std::pair<uint32_t, uint32_t> GetWindowResolution() noexcept { return { m_Width, m_Height }; }
		[[nodiscard]] static const HWND GetHandle() noexcept { return m_WindowHandle; }
		[[nodiscard]] static const RECT GetClientRect() noexcept { return m_ClientRect; }
		[[nodiscard]] static const RECT GetNonClientRect() noexcept { return m_NonClientRect; }
		static void OnUpdate();
		static void HideMouseCursor() noexcept;
		static void ShowMouseCursor() noexcept;
		static void ConfineMouseCursor(float left, float right, float bottom, float top) noexcept;
		static void FreeMouseCursor() noexcept;
		static void Present() noexcept;
		[[nodiscard]] static constexpr std::vector<BackBuffer>& GetBackBuffers() noexcept { return m_BackBuffers; }
		[[nodiscard]] static uint32_t GetCurrentBackbufferIndex() noexcept { return m_pSwapChain->GetCurrentBackBufferIndex(); }
		[[nodiscard]] static BackBuffer& GetCurrentBackBuffer() noexcept { return GetBackBuffers()[GetCurrentBackbufferIndex()]; }
		static void Resize() noexcept;
		static void ToggleVSync() noexcept;
		[[nodiscard]] static bool IsVSyncing() noexcept { return m_IsVsyncing; }
		[[nodiscard]] static bool IsFullScreen() noexcept { return m_IsFullScreen; }
		static void PrepareForFullScreenToggling(bool fullScreen) noexcept
		{
			m_ShouldToggleFullScreen = true;
		}
	private:
		static void ToggleFullScreen() noexcept;
		static LRESULT HandleMessages(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
		static void CreateSwapchain() noexcept;
		static void Finalize() noexcept;
		STATIC_CLASS(Window);
	private:
		static std::string m_Title;
		static std::string m_ClassName;
		static uint32_t m_Width;
		static uint32_t m_Height;
		static UINT m_WindowStyle;
		static HWND m_WindowHandle;
		static RECT m_ClientRect;
		static RECT m_NonClientRect;
		static MSG m_WindowMessage;
		static BOOL m_IsVisible;
		static Microsoft::WRL::ComPtr<IDXGISwapChain4> m_pSwapChain;
		static uint8_t m_NrOfBackBuffers;
		static std::vector<BackBuffer> m_BackBuffers;
		static bool m_IsResizing;
		static bool m_IsFullScreen;
		static bool m_IsVsyncing;

		static bool m_ShouldToggleFullScreen;
	};
}