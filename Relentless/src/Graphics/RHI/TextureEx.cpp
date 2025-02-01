#include "TextureEx.h"
#include "ResourceViews.h"

namespace Relentless
{
	TextureEx::TextureEx(GraphicsDevice* pParent, const TextureDesc& desc, ID3D12Resource* pResource) noexcept
		: 
		 DeviceResource{ pParent, pResource }
		,m_Desc{ desc }
	{
	}

	uint32 TextureEx::GetWidth() const noexcept
	{
		return m_Desc.Width;
	}

	uint32 TextureEx::GetHeight() const noexcept
	{
		return m_Desc.Height;
	}

	uint32 TextureEx::GetArraySize() const noexcept
	{
		return m_Desc.DepthOrArraySize;
	}

	uint32 TextureEx::GetMipLevels() const noexcept
	{
		return m_Desc.Mips;
	}

	uint32 TextureEx::GetSampleCount() const noexcept
	{
		return m_Desc.SampleCount;
	}

	TextureType TextureEx::GetType() const noexcept
	{
		return m_Desc.Type;
	}

	ResourceFormat TextureEx::GetFormat() const noexcept
	{
		return m_Desc.Format;
	}

	const ClearBinding& TextureEx::GetClearBinding() const noexcept
	{
		return m_Desc.ClearBindingValue;
	}

	const TextureDesc& TextureEx::GetDesc() const noexcept
	{
		return m_Desc;
	}

	DepthStencilView* TextureEx::GetDSV(uint32 subResourceIndex) const noexcept
	{
		RLS_ASSERT(m_DSVs.size() > subResourceIndex, "[Texture] Index Out Of Bounds Error.");
		return m_DSVs[subResourceIndex];
	}

	uint32 TextureEx::GetDSVIndex(uint32 subResourceIndex) const noexcept
	{
		RLS_ASSERT(m_DSVs.size() > subResourceIndex, "[Texture] Index Out Of Bounds Error.");
		return m_DSVs[subResourceIndex]->GetDescriptorIndex();
	}

	ShaderResourceView* TextureEx::GetSRV() const noexcept
	{
		return m_SRV;
	}

	uint32 TextureEx::GetSRVIndex() const noexcept
	{
		RLS_ASSERT(m_SRV, "[Texture] Shader Resource View Is Invalid");
		return m_SRV->GetDescriptorIndex();
	}

	RenderTargetView* TextureEx::GetRTV(uint32 subResourceIndex /*= 0*/) const noexcept
	{
		RLS_ASSERT(m_RTVs.size() > subResourceIndex, "[Texture] Index Out Of Bounds Error.");
		return m_RTVs[subResourceIndex];
	}

	uint32 TextureEx::GetRTVIndex(uint32 subResourceIndex) const noexcept
	{
		RLS_ASSERT(m_RTVs.size() > subResourceIndex, "[Texture] Index Out Of Bounds Error.");
		return m_RTVs[subResourceIndex]->GetDescriptorIndex();
	}

	void TextureEx::SetDSV(Ref<DepthStencilView> pDSV, uint32 subResourceIndex) noexcept
	{
		if (subResourceIndex >= m_DSVs.size())
			m_DSVs.resize(subResourceIndex + 1);

		m_DSVs[subResourceIndex] = pDSV;
	}

	void TextureEx::SetSRV(Ref<ShaderResourceView> pSRV) noexcept
	{
		m_SRV = pSRV;
	}
	
	void TextureEx::SetRTV(Ref<RenderTargetView> pRTV, uint32 subResourceIndex) noexcept
	{
		if (subResourceIndex >= m_RTVs.size())
			m_RTVs.resize(subResourceIndex + 1);

		m_RTVs[subResourceIndex] = pRTV;
	}
}