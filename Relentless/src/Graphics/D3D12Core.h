#pragma once
#include "D3D12Command.h"
namespace Relentless
{
	class D3D12Core
	{
	public:
		static void Initialize() noexcept;
		[[nodiscard]] static constexpr Microsoft::WRL::ComPtr<ID3D12Device9>& GetDevice() noexcept { return m_pDevice; }
		[[nodiscard]] static constexpr Microsoft::WRL::ComPtr<ID3D12CommandQueue>& GetCommandQueue() noexcept { return m_DirectCommandInterface.GetCommandQueue(); }
		[[nodiscard]] static constexpr Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& GetCommandAllocator(const uint32_t index) noexcept { return m_DirectCommandInterface.GetCommandAllocator(index); }
		[[nodiscard]] static constexpr Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4>& GetCommandList(const uint32_t index = (GetCurrentFrame() % GetNrOfBufferedFrames())) noexcept { return m_DirectCommandInterface.GetCommandList(index); }
		[[nodiscard]] static constexpr Microsoft::WRL::ComPtr<IDXGIFactory7>& GetFactory() noexcept { return m_pFactory; }
		[[nodiscard]] static constexpr uint8_t GetNrOfBufferedFrames() noexcept { return m_NrOfBufferedFrames; }
		[[nodiscard]] static constexpr uint32_t GetCurrentFrame() noexcept { return m_CurrentFrame; }
		[[nodiscard]] static constexpr bool IsInitialized() noexcept { return m_IsInitialized; }
		static void AdvanceToNextFrame() noexcept { m_CurrentFrame++; }
	private:
		D3D12Core() noexcept = default;
		~D3D12Core() noexcept = default;
		static void CreateDebugAndValidationLayer() noexcept;
	private:
		static Microsoft::WRL::ComPtr<ID3D12Device9> m_pDevice;
		static Microsoft::WRL::ComPtr<IDXGIFactory7> m_pFactory;
		static D3D12Command m_DirectCommandInterface;
		static uint8_t m_NrOfBufferedFrames;
		static uint32_t m_CurrentFrame;
		static bool m_IsInitialized;
#if defined(RLS_DEBUG)
		static Microsoft::WRL::ComPtr<ID3D12Debug5> m_pDebugController;
		static Microsoft::WRL::ComPtr<ID3D12InfoQueue> m_pInfoQueue;
#endif
	};
}