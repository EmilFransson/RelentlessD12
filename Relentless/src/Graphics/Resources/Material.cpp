#include "Material.h"
#include "Assets/AssetManager.h"
#include "Texture2D.h"

namespace Relentless
{
	const Vector4& Material::GetAlbedoColor() const noexcept
	{
		return m_AlbedoColor;
	}

	float Material::GetAmbientOcclusionIntensity() const noexcept
	{
		return m_AmbientOcclusionIntensity;
	}

	EBlendMode Material::GetBlendMode() const noexcept
	{
		return m_BlendMode;
	}

	float Material::GetDisplacementIntensity() const noexcept
	{
		return m_DisplacementIntensity;
	}

	const Vector4& Material::GetEmissiveColor() const noexcept
	{
		return m_EmissiveColor;
	}

	float Material::GetEmissiveIntensity() const noexcept
	{
		return m_EmissionIntensity;
	}

	const Vector2& Material::GetGlobalOffset() const noexcept
	{
		return m_GlobalUVTransform.Offset;
	}

	const Vector2& Material::GetGlobalTilingFactor() const noexcept
	{
		return m_GlobalUVTransform.TilingFactor;
	}

	float Material::GetMetalness() const noexcept
	{
		return m_Metallic;
	}

	float Material::GetRoughness() const noexcept
	{
		return m_Roughness;
	}

	Ref<Texture2D> Material::GetTexture(ETextureType aTextureType) const noexcept
	{
		return AssetManager::Get<Texture2D>(m_Textures[(size_t)aTextureType].TextureHandle);
	}

	bool Material::HasTexture(ETextureType aTextureType) const noexcept
	{
		return m_Textures[(size_t)aTextureType].TextureHandle != NULL_HANDLE;
	}

	bool Material::IsTwoSided() const noexcept
	{
		return m_IsTwoSided;
	}

	void Material::RemoveTexture(ETextureType aTextureType) noexcept
	{
		m_Textures[(size_t)aTextureType].TextureHandle = NULL_HANDLE;
	}

	void Material::SetAlbedoColor(const Vector4& aColor) noexcept
	{
		m_AlbedoColor = aColor;
	}

	void Material::SetAlbedoColor(const Color& aColor) noexcept
	{
		m_AlbedoColor = aColor;
	}

	void Material::SetAmbientOcclusionIntensity(float aAmbientOcclusionIntensity) noexcept
	{
		m_AmbientOcclusionIntensity = aAmbientOcclusionIntensity;
	}

	void Material::SetBlendMode(EBlendMode aBlendMode) noexcept
	{
		m_BlendMode = aBlendMode;
	}

	void Material::SetDisplacementIntensity(float aDisplacementIntensity) noexcept
	{
		m_DisplacementIntensity = aDisplacementIntensity;
	}

	void Material::SetEmissiveColor(const Vector4& aColor) noexcept
	{
		m_EmissiveColor = aColor;
	}

	void Material::SetEmissiveColor(const Color& aColor) noexcept
	{
		m_EmissiveColor = aColor;
	}

	void Material::SetEmissiveIntensity(float aEmissiveIntensity) noexcept
	{
		m_EmissionIntensity = aEmissiveIntensity;
	}

	void Material::SetGlobalOffset(const Vector2& aOffset) noexcept
	{
		m_GlobalUVTransform.Offset = aOffset;
	}

	void Material::SetGlobalTilingFactor(const Vector2& aTilingFactor) noexcept
	{
		m_GlobalUVTransform.TilingFactor = aTilingFactor;
	}

	void Material::SetIsTwoSided(bool aIsTwoSided) noexcept
	{
		m_IsTwoSided = aIsTwoSided;
	}

	void Material::SetMetalness(float aMetalness) noexcept
	{
		m_Metallic = aMetalness;
	}

	void Material::SetRoughness(float aRoughness) noexcept
	{
		m_Roughness = aRoughness;
	}

	void Material::SetTexture(ETextureType aTextureType, const AssetHandle& aTextureHandle) noexcept
	{
		m_Textures[(size_t)aTextureType].TextureHandle = aTextureHandle;
	}

	void Material::SetTextureEnabled(ETextureType aTextureType, bool aEnable) noexcept
	{
		m_Textures[(size_t)aTextureType].IsEnabled = aEnable;
	}
}