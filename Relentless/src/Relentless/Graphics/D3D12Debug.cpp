#include "D3D12Debug.h"
#include "D3D12Core.h"
namespace Relentless
{
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> D3D12Debug::m_pInfoQueue{ nullptr };

	void D3D12Debug::Initialize() noexcept
	{
		HRESULT hr = D3D12Core::GetDevice()->QueryInterface(IID_PPV_ARGS(&m_pInfoQueue));
		RLS_ASSERT(SUCCEEDED(hr), "Failed to query Information Queue interface.");
	}
}