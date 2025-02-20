#pragma once
#include "DeviceResource.h"

namespace Relentless
{
	using WindowHandle = HWND;

	class Swapchain : public DeviceObject
	{
	public:
		Swapchain(GraphicsDevice* pParent, uint32 numFrames, WindowHandle windowHandle) noexcept;
		virtual ~Swapchain() noexcept override;
		[[nodiscard]] TextureEx* GetBackBuffer() const noexcept;
		void OnResizeOrMove(uint32 width, uint32 height) noexcept;
		void Present() noexcept;
		void SetVSync(bool enabled) noexcept;
	private:
		void RecreateSwapchain() noexcept;
	private:
		std::vector<Ref<TextureEx>> m_Backbuffers;
		uint32 m_Width = 0u;
		uint32 m_Height = 0u;
		uint32 m_NumFrames = 0u;
		uint32 m_CurrentImage = 0u;
		WindowHandle m_WindowHandle = nullptr;
		Ref<Fence> m_pPresentFence = nullptr;
		Ref<IDXGISwapChainX> m_pSwapchain = nullptr;
		bool m_VSync = false;
		HANDLE m_WaitableObject = nullptr;

		const ResourceFormat m_Format = ResourceFormat::RGB10A2_UNORM;
	};
}