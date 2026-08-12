#include "ViewportSurface.h"

namespace Relentless
{
	ViewportSurface::Desc ViewportSurface::Desc::Fixed(const Vector2u& aSize) noexcept
	{
		RLS_ASSERT(aSize.x >= 1u && aSize.y >= 1u, "[Desc::Fixed]: Size must be greater or equal to 1.");
		Desc desc{};
		desc.m_FixedSize = aSize;
		desc.m_Mode = EResolutionMode::Fixed;
		return desc;
	}

	ViewportSurface::Desc ViewportSurface::Desc::FixedHeight(uint32 aHeight) noexcept
	{
		RLS_ASSERT(aHeight >= 1u, "[Desc::FixedHeight]: Height must be greater or equal to 1.");
		Desc desc{};
		desc.m_FixedHeight = aHeight;
		desc.m_Mode = EResolutionMode::FixedHeight;
		return desc;
	}

	ViewportSurface::Desc ViewportSurface::Desc::Native() noexcept
	{
		return Desc{};
	}

	ViewportSurface::Desc ViewportSurface::Desc::Scaled(float aScale) noexcept
	{
		RLS_ASSERT(aScale > 0.0f, "[Desc::Scaled]: Scale must be greater than 0.");
		Desc desc{};
		desc.m_Mode = EResolutionMode::Scale;
		desc.m_Scale = aScale;
		return desc;
	}

	ViewportSurface::ViewportSurface(const Desc& aDesc)
		: m_Desc{aDesc}
	{
	}

	Vector2u ViewportSurface::ComputeRenderSize() noexcept
	{
		if (m_RequestedSize.x == 0u || m_RequestedSize.y == 0u)
			return Vector2u::Zero();

		constexpr uint32 MAX_DIMENSION = 16'384u;
		constexpr float MIN_SCALE = 0.05f;
		constexpr float MAX_SCALE = 2.0f;
		
		switch (m_Desc.GetMode())
		{
		case EResolutionMode::Native: 
			return m_RequestedSize;
		case EResolutionMode::Fixed:
			return Vector2u(Math::Clamp(m_Desc.m_FixedSize.x, 1u, MAX_DIMENSION), Math::Clamp(m_Desc.m_FixedSize.y, 1u, MAX_DIMENSION));
		case EResolutionMode::FixedHeight:
		{
			const uint32 height = Math::Clamp(m_Desc.m_FixedHeight, 1u, MAX_DIMENSION);
			const float  aspect = static_cast<float>(m_RequestedSize.x) / static_cast<float>(m_RequestedSize.y);
			const uint32 width = Math::Clamp(static_cast<uint32>(Math::Round(height * aspect)), 1u, MAX_DIMENSION);
			return Vector2u(width, height);
		}
		case EResolutionMode::Scale:
		{
			const float scale = Math::Clamp(m_Desc.m_Scale, MIN_SCALE, MAX_SCALE);
			return Vector2u(Math::Max(1u, static_cast<uint32>(Math::Round(m_RequestedSize.x * scale))), Math::Max(1u, static_cast<uint32>(Math::Round(m_RequestedSize.y * scale))));
		}
		}

		RLS_ASSERT(false, "[ViewportSurface::ComputeRenderSize]: Invalid resolution mode encountered.");
		return m_RequestedSize;
	}

	void ViewportSurface::Flush() noexcept
	{
		if (!m_Dirty)
			return;

		m_Dirty = false;

		const Vector2u renderSize = ComputeRenderSize();
		if (renderSize.x == 0u || renderSize.y == 0u)
			return;

		if (m_pTexture)
		{
			const TextureDesc& desc = m_pTexture->GetDesc();
			if (m_RequestedSize.x == desc.Width && m_RequestedSize.y == desc.Height)
				return;
		}

		m_pTexture = Application::Get().GetGraphicsDevice()->CreateTexture(TextureDesc::Create2D(renderSize.x, renderSize.y, m_Desc.Format, 1u, m_Desc.Flags), m_Desc.DebugName.c_str());
		RLS_ASSERT(m_pTexture, "[ViewportSurface::Flush]: Texture creation failed.");
	}

	float ViewportSurface::GetAspectRatio() const noexcept
	{
		const Vector2u size = GetSize();
		return static_cast<float>(size.x) / static_cast<float>(size.y);
	}

	Vector2u ViewportSurface::GetSize() const noexcept
	{
		if (m_Desc.GetMode() == EResolutionMode::Fixed)
		{
			if (!m_pTexture || m_Dirty)
				return m_RequestedSize;
			else
			{
				const TextureDesc& textureDesc = m_pTexture->GetDesc();
				return Vector2u(textureDesc.Width, textureDesc.Height);
			}
		}

		return m_RequestedSize;
	}

	Texture* ViewportSurface::GetTexture() const noexcept
	{
		return m_pTexture;
	}

	void ViewportSurface::RequestResize(const Vector2u& aSize) noexcept
	{
		if (m_RequestedSize == aSize)
			return;

		m_RequestedSize = aSize;

		if (m_Desc.GetMode() != EResolutionMode::Fixed)
			m_Dirty = true;
	}

	void ViewportSurface::SetResolutionMode(EResolutionMode aMode) noexcept
	{
		if (m_Desc.GetMode() == aMode)
			return;

		m_Desc.m_Mode = aMode;
		m_Dirty = true;
	}

}