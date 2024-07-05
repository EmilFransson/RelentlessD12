#pragma once
namespace Relentless
{
	class D3D12Core
	{
	public:
		static void Initialize() noexcept;
		[[nodiscard]] static constexpr Microsoft::WRL::ComPtr<ID3D12Device9>& GetDevice() noexcept { return m_pDevice; }
		[[nodiscard]] static constexpr Microsoft::WRL::ComPtr<IDXGIFactory7>& GetFactory() noexcept { return m_pFactory; }
		[[nodiscard]] static constexpr bool IsInitialized() noexcept { return m_IsInitialized; }
		static void ReportLiveObjects() noexcept;
	private:
		D3D12Core() noexcept = default;
		static void CreateDebugAndValidationLayer() noexcept;
	private:
		static Microsoft::WRL::ComPtr<ID3D12Device9> m_pDevice;
		static Microsoft::WRL::ComPtr<IDXGIFactory7> m_pFactory;
		static bool m_IsInitialized;
#if defined(RLS_DEBUG)
		static Microsoft::WRL::ComPtr<ID3D12Debug5> m_pDebugController;
		static Microsoft::WRL::ComPtr<ID3D12InfoQueue> m_pInfoQueue;
#endif
	};
}