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

	ShaderResourceView* Texture::GetSRV() const noexcept
	{
		return m_pSRV;
	}

	uint32 Texture::GetSRVIndex() const noexcept
	{
		RLS_ASSERT(m_pSRV, "[Texture] Shader Resource View Is Invalid");
		return m_pSRV->GetDescriptorIndex();
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