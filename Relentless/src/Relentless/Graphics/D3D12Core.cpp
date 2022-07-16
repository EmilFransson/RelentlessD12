#include "D3D12Core.h"
namespace Relentless
{
	Microsoft::WRL::ComPtr<ID3D12Device10> D3D12Core::m_pDevice{ nullptr };
	Microsoft::WRL::ComPtr<IDXGIFactory7> D3D12Core::m_pFactory{nullptr};
	D3D12Command D3D12Core::m_DirectCommand{};
#if defined(RLS_DEBUG)
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> D3D12Core::m_pInfoQueue{nullptr};
	Microsoft::WRL::ComPtr<ID3D12Debug6> D3D12Core::m_pDebugController{ nullptr };
#endif

	void D3D12Core::Initialize() noexcept
	{
		CreateDebugAndValidationLayer();

#if defined(RLS_DEBUG)
		uint32_t factoryCreationFlags{ DXGI_CREATE_FACTORY_DEBUG };
		DXCall(::CreateDXGIFactory2(factoryCreationFlags, IID_PPV_ARGS(&m_pFactory)));
#else
		::CreateDXGIFactory2(0u, IID_PPV_ARGS(&m_pFactory));
#endif

		//Enumerate adapters:
		Microsoft::WRL::ComPtr<IDXGIAdapter4> pAdapter{ nullptr };
		bool adapterFound{ false };
		for (uint32_t i{ 0u }; m_pFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&pAdapter)) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			DXGI_ADAPTER_DESC3 adapterDescriptor{};
			pAdapter->GetDesc3(&adapterDescriptor);
			if (adapterDescriptor.Flags == DXGI_ADAPTER_FLAG3_SOFTWARE)
				continue;

			Microsoft::WRL::ComPtr<ID3D12Device10> pTempDevice{ nullptr };
			if (S_OK == ::D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device10), &pTempDevice))
			{
				adapterFound = true;
				break;
			}
		}
		RLS_ASSERT(adapterFound, "Unable to find a supported adapter.");

		DXCall(::D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_pDevice)));
		NAME_D12_OBJECT(m_pDevice, L"Main Device");

#if defined(RLS_DEBUG)
		D3D12Debug::Initialize();
#endif
		m_DirectCommand.Initialize(D3D12_COMMAND_LIST_TYPE_DIRECT, 2u);
	}

	void D3D12Core::CreateDebugAndValidationLayer() noexcept
	{
		#if defined(RLS_DEBUG)
			RLS_ASSERT(::D3D12GetDebugInterface(IID_PPV_ARGS(&m_pDebugController)) == S_OK, "Failed to retrieve debug interface.");
			m_pDebugController->EnableDebugLayer();
			m_pDebugController->SetEnableGPUBasedValidation(TRUE);
		#endif
	}
}