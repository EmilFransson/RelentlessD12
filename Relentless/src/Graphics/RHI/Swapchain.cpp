#include "Swapchain.h"
#include "Device.h"
#include "D3D.h"
#include "Fence.h"
#include "TextureEx.h"

namespace Relentless
{
	Swapchain::Swapchain(GraphicsDevice* pParent, uint32 numFrames, WindowHandle windowHandle) noexcept
		: DeviceObject(pParent), m_NumFrames{ numFrames }, m_WindowHandle{ windowHandle }
	{
		m_pPresentFence = new Fence(pParent, "Present Fence");
		RecreateSwapchain();
	}

	Swapchain::~Swapchain() noexcept
	{
		m_pPresentFence->CPUWait();
		m_pSwapchain->SetFullscreenState(false, nullptr);
	}

	TextureEx* Swapchain::GetBackBuffer() const noexcept
	{
		return m_Backbuffers[m_CurrentImage];
	}

	void Swapchain::OnResizeOrMove(uint32 width, uint32 height) noexcept
	{
		if (m_Width != width || m_Height != height)
		{
			m_Width = width;
			m_Height = height;

			m_pPresentFence->CPUWait();

			for (size_t i = 0u; i < m_NumFrames; ++i)
				m_Backbuffers[i].Reset();

			//Resize the buffers
			DXGI_SWAP_CHAIN_DESC1 desc{};
			m_pSwapchain->GetDesc1(&desc);

			VERIFY_HR(m_pSwapchain->ResizeBuffers(
				(uint32)m_Backbuffers.size(),
				width,
				height,
				D3D::ConvertFormat(m_Format),
				desc.Flags
			));

			UINT colorSpaceSupport = 0;
			DXGI_COLOR_SPACE_TYPE colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
			if (SUCCEEDED(m_pSwapchain->CheckColorSpaceSupport(colorSpace, &colorSpaceSupport)) &&
				(colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT) == DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT)
			{
				VERIFY_HR(m_pSwapchain->SetColorSpace1(colorSpace));
			}

			//Recreate the render target views
			for (uint32 i = 0; i < (uint32)m_Backbuffers.size(); ++i)
			{
				ID3D12ResourceX* pResource = nullptr;
				VERIFY_HR(m_pSwapchain->GetBuffer(i, IID_PPV_ARGS(&pResource)));
				m_Backbuffers[i] = GetParent()->CreateTextureForSwapchain(pResource, i);
			}

			m_CurrentImage = m_pSwapchain->GetCurrentBackBufferIndex();
		}
	}

	void Swapchain::Present() noexcept
	{
		VERIFY_HR_EX(m_pSwapchain->Present(m_VSync ? 1 : 0, !m_VSync ? DXGI_PRESENT_ALLOW_TEARING : 0), GetParent()->GetDevice());
		m_CurrentImage = m_pSwapchain->GetCurrentBackBufferIndex();

		// Signal and store when the GPU work for the frame we just flipped is finished.
		CommandQueue* pDirectQueue = GetParent()->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
		m_pPresentFence->Signal(pDirectQueue);

		::WaitForSingleObject(m_WaitableObject, INFINITE);
	}

	void Swapchain::SetVSync(bool enabled) noexcept
	{
		m_VSync = enabled;
	}

	void Swapchain::RecreateSwapchain() noexcept
	{
		DXGI_SWAP_CHAIN_DESC1 swapChainDescriptor{};
		swapChainDescriptor.Width = 0;
		swapChainDescriptor.Height = 0;
		swapChainDescriptor.Format = D3D::ConvertFormat(m_Format);
		swapChainDescriptor.Stereo = false;
		swapChainDescriptor.SampleDesc = { 1u, 0u };
		swapChainDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDescriptor.BufferCount = m_NumFrames;
		swapChainDescriptor.Scaling = DXGI_SCALING_STRETCH;
		swapChainDescriptor.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDescriptor.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
		swapChainDescriptor.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsDesc{};
		fsDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		fsDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		fsDesc.Windowed = true;

		m_Backbuffers.clear();
		m_Backbuffers.resize(m_NumFrames);
		m_pSwapchain.Reset();

		Ref<IDXGISwapChain1> pSwapchain{ nullptr };
		CommandQueue* pPresentQueue = GetParent()->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);

		VERIFY_HR(GetParent()->GetFactory()->CreateSwapChainForHwnd
		(
			pPresentQueue->GetCommandQueue(),
			m_WindowHandle,
			&swapChainDescriptor,
			&fsDesc,
			nullptr,
			pSwapchain.GetAddressOf()
		));

		pSwapchain.As(&m_pSwapchain);

		if (m_WaitableObject)
		{
			::CloseHandle(m_WaitableObject);
			m_WaitableObject = nullptr;
		}

		m_Width = 0;
		m_Height = 0;

		DXGI_SWAP_CHAIN_DESC1 descActual{};
		VERIFY_HR(m_pSwapchain->GetDesc1(&descActual));
		OnResizeOrMove(descActual.Width, descActual.Height);
	}
}
