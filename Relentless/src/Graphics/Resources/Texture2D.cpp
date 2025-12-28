#include "Texture2D.h"
#include "Core/Application.h"

namespace Relentless
{
	Texture2D::Texture2D(const TextureDesc& aTextureDesc, DirectX::ScratchImage&& aImage) noexcept
		:m_Desc(aTextureDesc),
		 m_ScratchImage(std::move(aImage))
	{
	}

	void Texture2D::CreateResource() noexcept
	{
		GraphicsDevice* pDevice = Application::Get().GetGraphicsDevice();
		if (!pDevice)
			return;

		m_pGPUResource = pDevice->CreateTexture(m_Desc, GetName().c_str(), m_ScratchImage);
	}

	const DirectX::ScratchImage& Texture2D::GetImage() const noexcept
	{
		return m_ScratchImage;
	}

	const TextureDesc& Texture2D::GetDesc() const noexcept
	{
		return m_Desc;
	}

	const Ref<Texture>& Texture2D::GetResource() const noexcept
	{
		return m_pGPUResource;
	}
}