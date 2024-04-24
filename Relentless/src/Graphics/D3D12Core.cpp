#include "D3D12Core.h"
namespace Relentless
{
	Microsoft::WRL::ComPtr<ID3D12Device9> D3D12Core::m_pDevice{ nullptr };
	Microsoft::WRL::ComPtr<IDXGIFactory7> D3D12Core::m_pFactory{nullptr};
	D3D12Command D3D12Core::m_DirectCommandInterface{};
	uint8_t D3D12Core::m_NrOfBufferedFrames{ 3u };
	uint32_t D3D12Core::m_CurrentFrame{ 0u };
	bool D3D12Core::m_IsInitialized{ false };
#if defined(RLS_DEBUG)
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> D3D12Core::m_pInfoQueue{nullptr};
	Microsoft::WRL::ComPtr<ID3D12Debug5> D3D12Core::m_pDebugController{ nullptr };
#endif
	std::vector<std::deque<D3D12SingleCommand>> D3D12Core::m_AvailableCommandResources;
	ThreadSafeQueue<D3D12SingleCommand> D3D12Core::m_UsedCommandResources;
	std::mutex D3D12Core::m_CommandMutex;
	std::condition_variable D3D12Core::m_CommandCondition;


	D3D12SingleCommand::D3D12SingleCommand(D3D12_COMMAND_LIST_TYPE commandType, uint8_t nrOfBufferedFrames)
	{
		RLS_ASSERT(nrOfBufferedFrames > 0, "Number of buffered frames must exceed 0.");
		m_CommandType = commandType;

		DXCall(D3D12Core::GetDevice()->CreateCommandAllocator(m_CommandType, IID_PPV_ARGS(&m_pCommandAllocator)));
		NAME_D12_OBJECT(m_pCommandAllocator, L"Command Allocator (Single)");

		DXCall(D3D12Core::GetDevice()->CreateCommandList(0u, m_CommandType, m_pCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_pCommandList)));
		NAME_D12_OBJECT(m_pCommandList, L"Command List (Single)");

		DXCall(m_pCommandList->Close());
	}

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

			Microsoft::WRL::ComPtr<ID3D12Device> pTempDevice{ nullptr };
			if (S_OK == ::D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), &pTempDevice))
			{
				adapterFound = true;
				break;
			}
		}
		RLS_ASSERT(adapterFound, "Unable to find a supported adapter.");

		DXCall(::D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_pDevice)));
		NAME_D12_OBJECT(m_pDevice, L"Main Device");

#if defined(RLS_DEBUG)
		D3D12Debug::Initialize();
#endif
		m_DirectCommandInterface.Initialize(D3D12_COMMAND_LIST_TYPE_DIRECT, m_NrOfBufferedFrames);

		m_AvailableCommandResources.resize(GetNrOfBufferedFrames());
		for (uint8_t i{ 0u }; i < GetNrOfBufferedFrames(); ++i)
		{
			for (uint32_t j{ 0u }; j < 40u; ++j)
			{
				D3D12SingleCommand command(D3D12_COMMAND_LIST_TYPE_DIRECT, m_NrOfBufferedFrames);
				m_AvailableCommandResources[i].push_back(command);
			}
		}

		m_IsInitialized = true;
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