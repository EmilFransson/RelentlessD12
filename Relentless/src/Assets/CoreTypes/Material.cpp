#include "Material.h"
#include "Assets/AssetManager.h"
#include "Texture2D.h"

namespace Relentless
{
	Material::Material(const UUID& aUUID) noexcept
		: AssetBase<Material>(aUUID)
	{
	}

	const Vector4& Material::GetAlbedoColor() const noexcept
	{
		return m_AlbedoColor;
	}

	float Material::GetAlphaCutOff() const noexcept
	{
		return m_AlphaCutOff;
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

	float Material::GetIOR() const noexcept
	{
		return m_IOR;
	}

	float Material::GetMetalness() const noexcept
	{
		return m_Metallic;
	}

	float Material::GetRefractionStrength() const noexcept
	{
		return m_RefractionStrength;
	}

	float Material::GetRoughness() const noexcept
	{
		return m_Roughness;
	}

	Ref<Texture2D> Material::GetTexture(ETextureType aTextureType) const noexcept
	{
		return AssetManager::Get<Texture2D>(m_Textures[(size_t)aTextureType].TextureHandle);
	}

	const AssetHandle& Material::GetTextureHandle(ETextureType aTextureType) const noexcept
	{
		return m_Textures[(size_t)aTextureType].TextureHandle;
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
		if (m_Textures[(size_t)aTextureType].TextureHandle == NULL_HANDLE)
			return;

		m_Textures[(size_t)aTextureType].TextureHandle = NULL_HANDLE;
		NOTIFY_PROPERTY_CHANGED(m_Textures);
	}

	bool Material::SerializeCore(IArchive& aArchive) noexcept
	{
		return 
		aArchive.Process(m_Textures) &&
		aArchive.Process(m_AlbedoColor) &&
		aArchive.Process(m_EmissiveColor) &&
		aArchive.Process(m_GlobalUVTransform) &&
		aArchive.Process(m_BlendMode) &&
		aArchive.Process(m_Metallic) &&
		aArchive.Process(m_EmissionIntensity) &&
		aArchive.Process(m_Roughness) &&
		aArchive.Process(m_DisplacementIntensity) &&
		aArchive.Process(m_AmbientOcclusionIntensity) &&
		aArchive.Process(m_AlphaCutOff) &&
		aArchive.Process(m_IOR) &&
		aArchive.Process(m_RefractionStrength) &&
		aArchive.Process(m_IsTwoSided) &&
		aArchive.IsValid();
	}

	void Material::SetAlbedoColor(const Vector4& aColor) noexcept
	{
		if (m_AlbedoColor == aColor)
			return;

		m_AlbedoColor = aColor;
		NOTIFY_PROPERTY_CHANGED(m_AlbedoColor);
	}

	void Material::SetAlbedoColor(const Color& aColor) noexcept
	{
		if (m_AlbedoColor == aColor)
			return;

		m_AlbedoColor = aColor;
		NOTIFY_PROPERTY_CHANGED(m_AlbedoColor);
	}

	void Material::SetAlphaCutOff(float aAlphaCutOff) noexcept
	{
		if (Math::AreValuesClose(m_AlphaCutOff, aAlphaCutOff))
			return;

		m_AlphaCutOff = aAlphaCutOff;
		NOTIFY_PROPERTY_CHANGED(m_AlphaCutOff);
	}

	void Material::SetAmbientOcclusionIntensity(float aAmbientOcclusionIntensity) noexcept
	{
		if (Math::AreValuesClose(m_AmbientOcclusionIntensity, aAmbientOcclusionIntensity))
			return;

		m_AmbientOcclusionIntensity = aAmbientOcclusionIntensity;
		NOTIFY_PROPERTY_CHANGED(m_AmbientOcclusionIntensity);
	}

	void Material::SetBlendMode(EBlendMode aBlendMode) noexcept
	{
		if (m_BlendMode == aBlendMode)
			return;

		m_BlendMode = aBlendMode;
		NOTIFY_PROPERTY_CHANGED(m_BlendMode);
	}

	void Material::SetDisplacementIntensity(float aDisplacementIntensity) noexcept
	{
		if (Math::AreValuesClose(m_DisplacementIntensity, aDisplacementIntensity))
			return;

		m_DisplacementIntensity = aDisplacementIntensity;
		NOTIFY_PROPERTY_CHANGED(m_DisplacementIntensity);
	}

	void Material::SetEmissiveColor(const Vector4& aColor) noexcept
	{
		if (m_EmissiveColor == aColor)
			return;

		m_EmissiveColor = aColor;
		NOTIFY_PROPERTY_CHANGED(m_EmissiveColor);
	}

	void Material::SetEmissiveColor(const Color& aColor) noexcept
	{
		if (m_EmissiveColor == aColor)
			return;

		m_EmissiveColor = aColor;
		NOTIFY_PROPERTY_CHANGED(m_EmissiveColor);
	}

	void Material::SetEmissiveIntensity(float aEmissiveIntensity) noexcept
	{
		if (Math::AreValuesClose(m_EmissionIntensity, aEmissiveIntensity))
			return;

		m_EmissionIntensity = aEmissiveIntensity;
		NOTIFY_PROPERTY_CHANGED(m_EmissionIntensity);
	}

	void Material::SetGlobalOffset(const Vector2& aOffset) noexcept
	{
		if (m_GlobalUVTransform.Offset == aOffset)
			return;

		m_GlobalUVTransform.Offset = aOffset;
		NOTIFY_PROPERTY_CHANGED(m_GlobalUVTransform.Offset);
	}

	void Material::SetGlobalTilingFactor(const Vector2& aTilingFactor) noexcept
	{
		if (m_GlobalUVTransform.TilingFactor == aTilingFactor)
			return;

		m_GlobalUVTransform.TilingFactor = aTilingFactor;
		NOTIFY_PROPERTY_CHANGED(m_GlobalUVTransform.TilingFactor);
	}

	void Material::SetIOR(float aIOR) noexcept
	{
		if (Math::AreValuesClose(m_IOR, aIOR))
			return;

		m_IOR = aIOR;
		NOTIFY_PROPERTY_CHANGED(m_IOR);
	}

	void Material::SetIsTwoSided(bool aIsTwoSided) noexcept
	{
		if (m_IsTwoSided == aIsTwoSided)
			return;

		m_IsTwoSided = aIsTwoSided;
		NOTIFY_PROPERTY_CHANGED(m_IsTwoSided);
	}

	void Material::SetMetalness(float aMetalness) noexcept
	{
		if (Math::AreValuesClose(m_Metallic, aMetalness))
			return;

		m_Metallic = aMetalness;
		NOTIFY_PROPERTY_CHANGED(m_Metallic);
	}

	void Material::SetRefractionStrength(float aRefractionStrength) noexcept
	{
		if (Math::AreValuesClose(m_RefractionStrength, aRefractionStrength))
			return;

		m_RefractionStrength = aRefractionStrength;
		NOTIFY_PROPERTY_CHANGED(m_RefractionStrength);
	}

	void Material::SetRoughness(float aRoughness) noexcept
	{
		if (Math::AreValuesClose(m_Roughness, aRoughness))
			return;

		m_Roughness = aRoughness;
		NOTIFY_PROPERTY_CHANGED(m_Roughness);
	}

	void Material::SetTexture(ETextureType aTextureType, const AssetHandle& aTextureHandle) noexcept
	{
		if (m_Textures[(size_t)aTextureType].TextureHandle == aTextureHandle)
			return;

		m_Textures[(size_t)aTextureType].TextureHandle = aTextureHandle;
		NOTIFY_PROPERTY_CHANGED(m_Textures);
	}

	void Material::SetTextureEnabled(ETextureType aTextureType, bool aEnable) noexcept
	{
		if (m_Textures[(size_t)aTextureType].IsEnabled == aEnable)
			return;

		m_Textures[(size_t)aTextureType].IsEnabled = aEnable;
		NOTIFY_PROPERTY_CHANGED(m_Textures);
	}

	void Material::PostLoad()
	{
		for (auto& textureEntry : m_Textures)
		{
			if (textureEntry.TextureHandle.IsValid())
				AssetManager::LoadAsset(textureEntry.TextureHandle);
		}
	}
}