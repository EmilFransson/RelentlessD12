#include "Texture.h"
#include "Device.h"
#include "ResourceViews.h"

namespace Relentless
{
	Texture::Texture(GraphicsDevice* pParent, const TextureDesc& desc, ID3D12Resource2* pResource) noexcept
		: 
		 DeviceResource{ pParent, pResource }
		,m_Desc{ desc }
	{
	}

	Texture::~Texture() noexcept = default;

	uint32 Texture::GetWidth() const noexcept
	{
		return m_Desc.Width;
	}

	uint32 Texture::GetHeight() const noexcept
	{
		return m_Desc.Height;
	}

	uint32 Texture::GetArraySize() const noexcept
	{
		return m_Desc.DepthOrArraySize;
	}

	uint32 Texture::GetMipLevels() const noexcept
	{
		return m_Desc.Mips;
	}

	uint32 Texture::GetSampleCount() const noexcept
	{
		return m_Desc.SampleCount;
	}

	TextureType Texture::GetType() const noexcept
	{
		return m_Desc.Type;
	}

	ResourceFormat Texture::GetFormat() const noexcept
	{
		return m_Desc.Format;
	}

	const ClearBinding& Texture::GetClearBinding() const noexcept
	{
		return m_Desc.ClearBindingValue;
	}

	const TextureDesc& Texture::GetDesc() const noexcept
	{
		return m_Desc;
	}

	DepthStencilView* Texture::GetDSV(uint32 subResourceIndex) const noexcept
	{
		RLS_ASSERT(m_DSVs.size() > subResourceIndex, "[Texture] Index Out Of Bounds Error.");
		return m_DSVs[subResourceIndex];
	}

	uint32 Texture::GetDSVIndex(uint32 subResourceIndex) const noexcept
	{
		RLS_ASSERT(m_DSVs.size() > subResourceIndex, "[Texture] Index Out Of Bounds Error.");
		return m_DSVs[subResourceIndex]->GetDescriptorIndex();
	}

	DepthStencilView* Texture::GetReadOnlyDSV(uint32 subResourceIndex /*= 0*/) const noexcept
	{
		RLS_ASSERT(m_ReadOnlyDSVs.size() > subResourceIndex, "[Texture] Index Out Of Bounds Error.");
		return m_ReadOnlyDSVs[subResourceIndex];
	}

	uint32 Texture::GetReadOnlyDSVIndex(uint32 subResourceIndex /*= 0*/) const noexcept
	{
		return m_ReadOnlyDSVs[subResourceIndex]->GetDescriptorIndex();
	}

	uint32 Texture::GetWritableDSVIndex(uint32 subResourceIndex /*= 0*/) const noexcept
	{
		return m_WritableDSVs[subResourceIndex]->GetDescriptorIndex();
	}

	DepthStencilView* Texture::GetWritableDSV(uint32 subResourceIndex /*= 0*/) const noexcept
	{
		RLS_ASSERT(m_WritableDSVs.size() > subResourceIndex, "[Texture] Index Out Of Bounds Error.");
		return m_WritableDSVs[subResourceIndex];
	}

	ShaderResourceView* Texture::GetSRV() const noexcept
	{
		return m_pSRV;
	}

	uint32 Texture::GetSRVIndex() const noexcept
	{
		RLS_ASSERT(m_pSRV, "[Texture] Shader Resource View Is Invalid");
		return m_pSRV->GetDescriptorIndex();
	}

	uint32 Texture::GetArraySRVIndex(uint32 aMip) const noexcept
	{
		RLS_ASSERT(m_PerMipSRVs.size() > aMip, "[Texture::GetArraySRVIndex] Index Out Of Bounds Error.");
		return m_PerMipSRVs[aMip]->GetDescriptorIndex();
	}

	UnorderedAccessView* Texture::GetUAV(uint32 subResourceIndex) const noexcept
	{
		RLS_ASSERT(m_UAVs.size() > subResourceIndex, "[Texture::GetUAV] Index Out Of Bounds Error.");
		return m_UAVs[subResourceIndex];
	}

	uint32 Texture::GetUAVIndex(uint32 subResourceIndex/* = 0*/) const noexcept
	{
		RLS_ASSERT(m_UAVs.size() > subResourceIndex, "[Texture::GetUAVIndex] Index Out Of Bounds Error.");
		return m_UAVs[subResourceIndex]->GetDescriptorIndex();
	}

	RenderTargetView* Texture::GetRTV(uint32 subResourceIndex /*= 0*/) const noexcept
	{
		RLS_ASSERT(m_RTVs.size() > subResourceIndex, "[Texture] Index Out Of Bounds Error.");
		return m_RTVs[subResourceIndex];
	}

	uint32 Texture::GetRTVIndex(uint32 subResourceIndex) const noexcept
	{
		RLS_ASSERT(m_RTVs.size() > subResourceIndex, "[Texture] Index Out Of Bounds Error.");
		return m_RTVs[subResourceIndex]->GetDescriptorIndex();
	}

	bool Texture::HasDSV(uint32 subResourceIndex) const noexcept
	{
		return subResourceIndex < m_DSVs.size();
	}

	bool Texture::HasRTV(uint32 subResourceIndex /*= 0*/) const noexcept
	{
		return subResourceIndex < m_RTVs.size();
	}

	void Texture::SetReadOnlyDSV(Ref<DepthStencilView> pDSV, uint32 subResourceIndex /*= 0u*/) noexcept
	{
		if (subResourceIndex >= m_ReadOnlyDSVs.size())
			m_ReadOnlyDSVs.resize(subResourceIndex + 1);
		
		m_ReadOnlyDSVs[subResourceIndex] = pDSV;
	}

	void Texture::SetWritableDSV(Ref<DepthStencilView> pDSV, uint32 subResourceIndex /*= 0u*/) noexcept
	{
		if (subResourceIndex >= m_WritableDSVs.size())
			m_WritableDSVs.resize(subResourceIndex + 1);

		m_WritableDSVs[subResourceIndex] = pDSV;
	}

	void Texture::SetDSV(Ref<DepthStencilView> pDSV, uint32 subResourceIndex) noexcept
	{
		if (subResourceIndex >= m_DSVs.size())
			m_DSVs.resize(subResourceIndex + 1);

		m_DSVs[subResourceIndex] = pDSV;
	}

	void Texture::SetSRV(Ref<ShaderResourceView> pSRV) noexcept
	{
		m_pSRV = pSRV;
	}
	
	void Texture::SetMipArraySRV(Ref<ShaderResourceView> aSRV, uint32 aMip) noexcept
	{
		if (aMip >= m_PerMipSRVs.size())
			m_PerMipSRVs.resize(aMip + 1);
		
		m_PerMipSRVs[aMip] = aSRV;
	}

	void Texture::SetRTV(Ref<RenderTargetView> pRTV, uint32 subResourceIndex) noexcept
	{
		if (subResourceIndex >= m_RTVs.size())
			m_RTVs.resize(subResourceIndex + 1);

		m_RTVs[subResourceIndex] = pRTV;
	}

	void Texture::SetUAV(Ref<UnorderedAccessView> pUAV, uint32 subResourceIndex) noexcept
	{
		if (subResourceIndex >= m_UAVs.size())
			m_UAVs.resize(subResourceIndex + 1);

		m_UAVs[subResourceIndex] = pUAV;
	}

}