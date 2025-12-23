#include "Texture2D.h"

namespace Relentless
{
	Texture2D::Texture2D(const TextureDesc& aTextureDesc, DirectX::ScratchImage&& aImage) noexcept
		:m_Desc(aTextureDesc),
		 m_ScratchImage(std::move(aImage))
	{
	}

	const DirectX::ScratchImage& Texture2D::GetImage() const noexcept
	{
		return m_ScratchImage;
	}

	const TextureDesc& Texture2D::GetDesc() const noexcept
	{
		return m_Desc;
	}

}