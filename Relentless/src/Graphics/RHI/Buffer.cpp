#include "Buffer.h"
#include "Device.h"
#include "ResourceViews.h"

namespace Relentless
{
	BufferEx::BufferEx(GraphicsDevice* pParent, const BufferDesc& desc, ID3D12Resource2* pResource) noexcept
		:
		DeviceResource(pParent, pResource),
		m_Desc(desc)
	{}

	BufferEx::~BufferEx() noexcept
	{
		
	}

	const BufferDesc& BufferEx::GetDesc() const noexcept
	{
		return m_Desc;
	}

	void* BufferEx::GetMappedData() const noexcept
	{
		RLS_ASSERT(m_pMappedPtr, "Buffer is not mapped.");
		return m_pMappedPtr;
	}

	uint64 BufferEx::GetSize() const noexcept
	{
		return m_Desc.Size;
	}

	uint64 BufferEx::GetNrOfElements() const noexcept
	{
		return m_Desc.NumElements();
	}

	ShaderResourceView* BufferEx::GetSRV() const noexcept
	{
		return m_pSRV;
	}

	uint32 BufferEx::GetSRVIndex() const noexcept
	{
		RLS_ASSERT(m_pSRV, "Shader Resource View Is Invalid.");
		return m_pSRV->GetDescriptorIndex();
	}

	void BufferEx::Map(uint32 subresource, const D3D12_RANGE* pRange) noexcept
	{
		ID3D12Resource* pResource = GetResource();
		RLS_ASSERT(pResource, "D3D12 resource is invalid");
		
		VERIFY_HR(pResource->Map(subresource, pRange, &m_pMappedPtr));
	}

	void BufferEx::Unmap(uint32 subresource, const D3D12_RANGE* pRange) noexcept
	{
		ID3D12Resource* pResource = GetResource();
		RLS_ASSERT(pResource, "D3D12 resource is invalid");

		pResource->Unmap(subresource, pRange);
		m_pMappedPtr = nullptr;
	}

	void BufferEx::SetSRV(Ref<ShaderResourceView> pSRV) noexcept
	{
		m_pSRV = pSRV;
	}

}
