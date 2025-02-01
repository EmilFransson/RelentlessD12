#include "Buffer.h"

#include "ResourceViews.h"

namespace Relentless
{
	Buffer::Buffer(GraphicsDevice* pParent, const BufferDesc& desc, ID3D12Resource* pResource) noexcept
		:
		DeviceResource(pParent, pResource),
		m_Desc(desc)
	{}

	const BufferDesc& Buffer::GetDesc() const noexcept
	{
		return m_Desc;
	}

	void* Buffer::GetMappedData() const noexcept
	{
		RLS_ASSERT(m_pMappedPtr, "Buffer is not mapped.");
		return m_pMappedPtr;
	}

	uint64 Buffer::GetSize() const noexcept
	{
		return m_Desc.Size;
	}

	uint64 Buffer::GetNrOfElements() const noexcept
	{
		return m_Desc.NumElements();
	}

	ShaderResourceView* Buffer::GetSRV() const noexcept
	{
		return m_pSRV;
	}

	uint32 Buffer::GetSRVIndex() const noexcept
	{
		RLS_ASSERT(m_pSRV, "Shader Resource View Is Invalid.");
		return m_pSRV->GetDescriptorIndex();
	}

	void Buffer::Map(uint32 subresource, const D3D12_RANGE* pRange) noexcept
	{
		ID3D12Resource* pResource = GetResource();
		RLS_ASSERT(pResource, "D3D12 resource is invalid");
		
		VERIFY_HR(pResource->Map(subresource, pRange, &m_pMappedPtr));
	}

	void Buffer::SetSRV(Ref<ShaderResourceView> pSRV) noexcept
	{
		m_pSRV = pSRV;
	}

}
