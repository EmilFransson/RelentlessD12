#include "Material.h"
#include "Assets/AssetManager.h"
#include "../D3D12Core.h"
namespace Relentless
{
	Material::Material() noexcept
		: m_AlbedoColor{1.0f, 1.0f, 1.0f, 1.0f},
		  m_EmissionColor{0.0f, 0.0f, 0.0f, 1.0f},
		  m_Metallic{0.0f},
		  m_Padding{0.0f},
		  m_EmissionIntensity{0.0f},
		  m_Roughness{0.5f},
		  m_AlbedoTextureIndex{0xFFFFFFFF},
		  m_MetallicTextureIndex{0xFFFFFFFF},
		  m_RoughnessTextureIndex{0xFFFFFFFF},
		  m_NormalMapIndex{0xFFFFFFFF},
		  m_HeightMapIndex{0xFFFFFFFF},
		  m_AmbientOcclusionTextureIndex{ 0xFFFFFFFF },
		  m_EmissionTextureIndex{ 0xFFFFFFFF },
		  m_HeightScale{1.0f},
		  m_AOScale{1.0f},
		  m_CombinedRoughnessMetallnesMap{false},
		  m_TilingFactor{1.0f, 1.0f},
		  m_Offset{ 0.0f, 0.0f },
		  m_Name{"Unnamed Material"},
		  m_AlbedoTextureHandle{ NULL_HANDLE },
		  m_MetallicTextureHandle{ NULL_HANDLE },
		  m_RoughnessTextureHandle{ NULL_HANDLE },
		  m_NormalMapHandle{NULL_HANDLE},
		  m_HeightMapHandle{ NULL_HANDLE },
		  m_AmbientOcclusionTextureHandle{ NULL_HANDLE },
		  m_EmissionTextureHandle{ NULL_HANDLE },
		  m_UseAlbedoTexture{true},
		  m_UseMetallicTexture{true},
		  m_UseRoughnessTexture{true},
		  m_UseHeightMap{true},
		  m_UseNormalMap{true},
		  m_UseAmbientOcclusionTexture{ true },
	 	  m_UseEmissionTexture{ true },
		  m_RenderMode{ RenderMode::Opaque }
	{
	}

	Material::Material(const Material& otherMaterial) noexcept
	{
		m_AlbedoColor = otherMaterial.m_AlbedoColor;
		m_Metallic = otherMaterial.m_Metallic;
		m_Roughness = otherMaterial.m_Roughness;
		m_EmissionColor = otherMaterial.m_EmissionColor;
		m_EmissionIntensity = otherMaterial.m_EmissionIntensity;
		m_HeightScale = otherMaterial.m_HeightScale;
		m_AOScale = otherMaterial.m_AOScale;
		m_CombinedRoughnessMetallnesMap = otherMaterial.m_CombinedRoughnessMetallnesMap;
		m_AlbedoTextureIndex = otherMaterial.m_AlbedoTextureIndex;
		m_MetallicTextureIndex = otherMaterial.m_MetallicTextureIndex;
		m_RoughnessTextureIndex = otherMaterial.m_RoughnessTextureIndex;
		m_NormalMapIndex = otherMaterial.m_NormalMapIndex;
		m_HeightMapIndex = otherMaterial.m_HeightMapIndex;
		m_AmbientOcclusionTextureIndex = otherMaterial.m_AmbientOcclusionTextureIndex;
		m_EmissionTextureIndex = otherMaterial.m_EmissionTextureIndex;
		m_TilingFactor = otherMaterial.m_TilingFactor;
		m_Offset = otherMaterial.m_Offset;
		m_Name = otherMaterial.m_Name;
		m_AlbedoTextureHandle = otherMaterial.m_AlbedoTextureHandle;
		m_MetallicTextureHandle = otherMaterial.m_MetallicTextureHandle;
		m_RoughnessTextureHandle = otherMaterial.m_RoughnessTextureHandle;
		m_NormalMapHandle = otherMaterial.m_NormalMapHandle;
		m_HeightMapHandle = otherMaterial.m_HeightMapHandle;
		m_AmbientOcclusionTextureHandle = otherMaterial.m_AmbientOcclusionTextureHandle;
		m_EmissionTextureHandle = otherMaterial.m_EmissionTextureHandle;
		m_UseAlbedoTexture = otherMaterial.m_UseAlbedoTexture;
		m_UseMetallicTexture = otherMaterial.m_UseMetallicTexture;
		m_UseRoughnessTexture = otherMaterial.m_UseRoughnessTexture;
		m_UseHeightMap = otherMaterial.m_UseHeightMap;
		m_UseNormalMap = otherMaterial.m_UseNormalMap;
		m_UseAmbientOcclusionTexture = otherMaterial.m_UseAmbientOcclusionTexture;
		m_UseEmissionTexture = otherMaterial.m_UseEmissionTexture;
		m_RenderMode = otherMaterial.m_RenderMode;
	}

	Material& Material::operator=(const Material& otherMaterial) noexcept
	{
		if (this != &otherMaterial)
		{
			m_AlbedoColor = otherMaterial.m_AlbedoColor;
			m_Metallic = otherMaterial.m_Metallic;
			m_Roughness = otherMaterial.m_Roughness;
			m_EmissionColor = otherMaterial.m_EmissionColor;
			m_EmissionIntensity = otherMaterial.m_EmissionIntensity;
			m_HeightScale = otherMaterial.m_HeightScale;
			m_AOScale = otherMaterial.m_AOScale;
			m_CombinedRoughnessMetallnesMap = otherMaterial.m_CombinedRoughnessMetallnesMap;
			m_AlbedoTextureIndex = otherMaterial.m_AlbedoTextureIndex;
			m_MetallicTextureIndex = otherMaterial.m_MetallicTextureIndex;
			m_RoughnessTextureIndex = otherMaterial.m_RoughnessTextureIndex;
			m_NormalMapIndex = otherMaterial.m_NormalMapIndex;
			m_HeightMapIndex = otherMaterial.m_HeightMapIndex;
			m_AmbientOcclusionTextureIndex = otherMaterial.m_AmbientOcclusionTextureIndex;
			m_EmissionTextureIndex = otherMaterial.m_EmissionTextureIndex;
			m_TilingFactor = otherMaterial.m_TilingFactor;
			m_Offset = otherMaterial.m_Offset;
			m_Name = otherMaterial.m_Name;
			m_AlbedoTextureHandle = otherMaterial.m_AlbedoTextureHandle;
			m_MetallicTextureHandle = otherMaterial.m_MetallicTextureHandle;
			m_RoughnessTextureHandle = otherMaterial.m_RoughnessTextureHandle;
			m_NormalMapHandle = otherMaterial.m_NormalMapHandle;
			m_HeightMapHandle = otherMaterial.m_HeightMapHandle;
			m_AmbientOcclusionTextureHandle = otherMaterial.m_AmbientOcclusionTextureHandle;
			m_EmissionTextureHandle = otherMaterial.m_EmissionTextureHandle;
			m_UseAlbedoTexture = otherMaterial.m_UseAlbedoTexture;
			m_UseMetallicTexture = otherMaterial.m_UseMetallicTexture;
			m_UseRoughnessTexture = otherMaterial.m_UseRoughnessTexture;
			m_UseHeightMap = otherMaterial.m_UseHeightMap;
			m_UseNormalMap = otherMaterial.m_UseNormalMap;
			m_UseAmbientOcclusionTexture = otherMaterial.m_UseAmbientOcclusionTexture;
			m_UseEmissionTexture = otherMaterial.m_UseEmissionTexture;
			m_RenderMode = otherMaterial.m_RenderMode;
		}

		return *this;
	}

	Material::Material(Material&& otherMaterial) noexcept
	{
		m_AlbedoColor = std::move(otherMaterial.m_AlbedoColor);
		m_Metallic = std::move(otherMaterial.m_Metallic);
		m_Roughness = std::move(otherMaterial.m_Roughness);
		m_EmissionColor = std::move(otherMaterial.m_EmissionColor);
		m_EmissionIntensity = std::move(otherMaterial.m_EmissionIntensity);
		m_HeightScale = std::move(otherMaterial.m_HeightScale);
		m_AOScale = std::move(otherMaterial.m_AOScale);
		m_CombinedRoughnessMetallnesMap = std::move(otherMaterial.m_CombinedRoughnessMetallnesMap);
		m_AlbedoTextureIndex = std::move(otherMaterial.m_AlbedoTextureIndex);
		m_MetallicTextureIndex = std::move(otherMaterial.m_MetallicTextureIndex);
		m_RoughnessTextureIndex = std::move(otherMaterial.m_RoughnessTextureIndex);
		m_NormalMapIndex = std::move(otherMaterial.m_NormalMapIndex);
		m_HeightMapIndex = std::move(otherMaterial.m_HeightMapIndex);
		m_AmbientOcclusionTextureIndex = std::move(otherMaterial.m_AmbientOcclusionTextureIndex);
		m_EmissionTextureIndex = std::move(otherMaterial.m_EmissionTextureIndex);
		m_TilingFactor = std::move(otherMaterial.m_TilingFactor);
		m_Offset = std::move(otherMaterial.m_Offset);
		m_Name = std::move(otherMaterial.m_Name);
		m_AlbedoTextureHandle = std::move(otherMaterial.m_AlbedoTextureHandle);
		m_MetallicTextureHandle = std::move(otherMaterial.m_MetallicTextureHandle);
		m_RoughnessTextureHandle = std::move(otherMaterial.m_RoughnessTextureHandle);
		m_NormalMapHandle = std::move(otherMaterial.m_NormalMapHandle);
		m_HeightMapHandle = std::move(otherMaterial.m_HeightMapHandle);
		m_AmbientOcclusionTextureHandle = std::move(otherMaterial.m_AmbientOcclusionTextureHandle);
		m_EmissionTextureHandle = std::move(otherMaterial.m_EmissionTextureHandle);
		m_UseAlbedoTexture = std::move(otherMaterial.m_UseAlbedoTexture);
		m_UseMetallicTexture = std::move(otherMaterial.m_UseMetallicTexture);
		m_UseRoughnessTexture = std::move(otherMaterial.m_UseRoughnessTexture);
		m_UseHeightMap = std::move(otherMaterial.m_UseHeightMap);
		m_UseNormalMap = std::move(otherMaterial.m_UseNormalMap);
		m_UseAmbientOcclusionTexture = std::move(otherMaterial.m_UseAmbientOcclusionTexture);
		m_UseEmissionTexture = std::move(otherMaterial.m_UseEmissionTexture);
		m_RenderMode = std::move(otherMaterial.m_RenderMode);
	}

	Material& Material::operator=(Material&& otherMaterial) noexcept
	{
		if (this != &otherMaterial)
		{
			m_AlbedoColor = std::move(otherMaterial.m_AlbedoColor);
			m_Metallic = std::move(otherMaterial.m_Metallic);
			m_Roughness = std::move(otherMaterial.m_Roughness);
			m_EmissionColor = std::move(otherMaterial.m_EmissionColor);
			m_EmissionIntensity = std::move(otherMaterial.m_EmissionIntensity);
			m_HeightScale = std::move(otherMaterial.m_HeightScale);
			m_AOScale = std::move(otherMaterial.m_AOScale);
			m_CombinedRoughnessMetallnesMap = std::move(otherMaterial.m_CombinedRoughnessMetallnesMap);
			m_AlbedoTextureIndex = std::move(otherMaterial.m_AlbedoTextureIndex);
			m_MetallicTextureIndex = std::move(otherMaterial.m_MetallicTextureIndex);
			m_RoughnessTextureIndex = std::move(otherMaterial.m_RoughnessTextureIndex);
			m_NormalMapIndex = std::move(otherMaterial.m_NormalMapIndex);
			m_HeightMapIndex = std::move(otherMaterial.m_HeightMapIndex);
			m_AmbientOcclusionTextureIndex = std::move(otherMaterial.m_AmbientOcclusionTextureIndex);
			m_EmissionTextureIndex = std::move(otherMaterial.m_EmissionTextureIndex);
			m_TilingFactor = std::move(otherMaterial.m_TilingFactor);
			m_Offset = std::move(otherMaterial.m_Offset);
			m_Name = std::move(otherMaterial.m_Name);
			m_AlbedoTextureHandle = std::move(otherMaterial.m_AlbedoTextureHandle);
			m_MetallicTextureHandle = std::move(otherMaterial.m_MetallicTextureHandle);
			m_RoughnessTextureHandle = std::move(otherMaterial.m_RoughnessTextureHandle);
			m_NormalMapHandle = std::move(otherMaterial.m_NormalMapHandle);
			m_HeightMapHandle = std::move(otherMaterial.m_HeightMapHandle);
			m_AmbientOcclusionTextureHandle = std::move(otherMaterial.m_AmbientOcclusionTextureHandle);
			m_EmissionTextureHandle = std::move(otherMaterial.m_EmissionTextureHandle);
			m_UseAlbedoTexture = std::move(otherMaterial.m_UseAlbedoTexture);
			m_UseMetallicTexture = std::move(otherMaterial.m_UseMetallicTexture);
			m_UseRoughnessTexture = std::move(otherMaterial.m_UseRoughnessTexture);
			m_UseHeightMap = std::move(otherMaterial.m_UseHeightMap);
			m_UseNormalMap = std::move(otherMaterial.m_UseNormalMap);
			m_UseAmbientOcclusionTexture = std::move(otherMaterial.m_UseAmbientOcclusionTexture);
			m_UseEmissionTexture = std::move(otherMaterial.m_UseEmissionTexture);
			m_RenderMode = std::move(otherMaterial.m_RenderMode);
		}

		return *this;
	}

	void Material::SetAlbedoTexture(const AssetHandle& albedoTextureHandle) noexcept
	{
		RLS_ASSERT(albedoTextureHandle != NULL_HANDLE, "Albedo texture handle is invalid.");
		Ref<TextureEx> albedoTexture = AssetManager::Get<TextureEx>(albedoTextureHandle);

		RLS_ASSERT(albedoTexture->GetResource(), "Albedo texture is invalid.");
		RLS_ASSERT(albedoTexture->GetSRVIndex() != static_cast<uint32_t>(-1), "Albedo texture descriptor index is invalid.");
		
		m_AlbedoTextureHandle = albedoTextureHandle;
		m_AlbedoTextureIndex = albedoTexture->GetSRVIndex();
	}

	void Material::SetMetallicTexture(const AssetHandle& metallicTextureHandle) noexcept
	{
		RLS_ASSERT(metallicTextureHandle != NULL_HANDLE, "Metallic texture handle is invalid.");
		Ref<TextureEx> metallicTexture = AssetManager::Get<TextureEx>(metallicTextureHandle);

		RLS_ASSERT(metallicTexture->GetResource(), "Metallic texture is invalid.");
		RLS_ASSERT(metallicTexture->GetSRVIndex() != static_cast<uint32_t>(-1), "Metallic texture descriptor index is invalid.");

		m_MetallicTextureHandle = metallicTextureHandle;
		m_MetallicTextureIndex = metallicTexture->GetSRVIndex();
	}

	void Material::SetRoughnessTexture(const AssetHandle& roughnessTextureHandle) noexcept
	{
		RLS_ASSERT(roughnessTextureHandle != NULL_HANDLE, "Roughness texture handle is invalid.");
		Ref<TextureEx> roughnessTexture = AssetManager::Get<TextureEx>(roughnessTextureHandle);

		RLS_ASSERT(roughnessTexture->GetResource(), "Roughness texture is invalid.");
		RLS_ASSERT(roughnessTexture->GetSRVIndex() != static_cast<uint32_t>(-1), "Roughness texture descriptor index is invalid.");

		m_RoughnessTextureHandle = roughnessTextureHandle;
		m_RoughnessTextureIndex = roughnessTexture->GetSRVIndex();
	}

	void Material::SetNormalMap(const AssetHandle& normalMapHandle) noexcept
	{
		RLS_ASSERT(normalMapHandle != NULL_HANDLE, "Normal map handle is invalid.");
		Ref<TextureEx> normalMap = AssetManager::Get<TextureEx>(normalMapHandle);

		RLS_ASSERT(normalMap->GetResource(), "Normal map is invalid.");
		RLS_ASSERT(normalMap->GetSRVIndex() != static_cast<uint32_t>(-1), "Normal map descriptor index is invalid.");

		m_NormalMapHandle = normalMapHandle;
		m_NormalMapIndex = normalMap->GetSRVIndex();
	}

	void Material::SetHeightMap(const AssetHandle& heightMapHandle) noexcept
	{
		RLS_ASSERT(heightMapHandle != NULL_HANDLE, "Height map handle is invalid.");
		Ref<TextureEx> heightMap = AssetManager::Get<TextureEx>(heightMapHandle);

		RLS_ASSERT(heightMap->GetResource(), "Height map is invalid.");
		RLS_ASSERT(heightMap->GetSRVIndex() != static_cast<uint32_t>(-1), "Height map descriptor index is invalid.");

		m_HeightMapHandle = heightMapHandle;
		m_HeightMapIndex = heightMap->GetSRVIndex();
	}

	void Material::SetAmbientOcclusionTexture(const AssetHandle& ambientOcclusionTextureHandle) noexcept
	{
		RLS_ASSERT(ambientOcclusionTextureHandle != NULL_HANDLE, "Ambient occlusion texture handle is invalid.");
		Ref<TextureEx> ambientOcclusionTexture = AssetManager::Get<TextureEx>(ambientOcclusionTextureHandle);

		RLS_ASSERT(ambientOcclusionTexture->GetResource(), "Ambient occlusion texture is invalid.");
		RLS_ASSERT(ambientOcclusionTexture->GetSRVIndex() != static_cast<uint32_t>(-1), "Ambient occlusion texture descriptor index is invalid.");

		m_AmbientOcclusionTextureHandle = ambientOcclusionTextureHandle;
		m_AmbientOcclusionTextureIndex = ambientOcclusionTexture->GetSRVIndex();
	}

	void Material::SetEmissionTexture(const AssetHandle& emissionTextureHandle) noexcept
	{
		RLS_ASSERT(emissionTextureHandle != NULL_HANDLE, "Emission texture handle is invalid.");
		Ref<TextureEx> emissionTexture = AssetManager::Get<TextureEx>(emissionTextureHandle);

		RLS_ASSERT(emissionTexture->GetResource(), "Emission texture is invalid.");
		RLS_ASSERT(emissionTexture->GetSRVIndex() != static_cast<uint32_t>(-1), "Emission texture descriptor index is invalid.");

		m_EmissionTextureHandle = emissionTextureHandle;
		m_EmissionTextureIndex = emissionTexture->GetSRVIndex();
	}

	void Material::RemoveAlbedoTexture() noexcept
	{
		RLS_ASSERT(HasAlbedoTexture(), "Material does not have an albedo texture.");
		m_AlbedoTextureHandle = NULL_HANDLE;
		m_AlbedoTextureIndex = 0xFFFFFFFF;
	}

	void Material::RemoveMetallicTexture() noexcept
	{
		RLS_ASSERT(HasMetallicTexture(), "Material does not have a metallic texture.");
		m_MetallicTextureHandle = NULL_HANDLE;
		m_MetallicTextureIndex = 0xFFFFFFFF;
	}

	void Material::RemoveRoughnessTexture() noexcept
	{
		RLS_ASSERT(HasRoughnessTexture(), "Material does not have a roughness texture.");
		m_RoughnessTextureHandle = NULL_HANDLE;
		m_RoughnessTextureIndex = 0xFFFFFFFF;
	}

	void Material::RemoveNormalMap() noexcept
	{
		RLS_ASSERT(HasNormalMap(), "Material does not have a normal map.");
		m_NormalMapHandle = NULL_HANDLE;
		m_NormalMapIndex = 0xFFFFFFFF;
	}

	void Material::RemoveHeightMap() noexcept
	{
		RLS_ASSERT(HasHeightMap(), "Material does not have a height map.");
		m_HeightMapHandle = NULL_HANDLE;
		m_HeightMapIndex = 0xFFFFFFFF;
	}

	void Material::RemoveAmbientOcclusionTexture() noexcept
	{
		RLS_ASSERT(HasAmbientOcclusionTexture(), "Material does not have an ambient occlusion texture.");
		m_AmbientOcclusionTextureHandle = NULL_HANDLE;
		m_AmbientOcclusionTextureIndex = 0xFFFFFFFF;
	}

	void Material::RemoveEmissionTexture() noexcept
	{
		RLS_ASSERT(HasAlbedoTexture(), "Material does not have an emission texture.");
		m_EmissionTextureHandle = NULL_HANDLE;
		m_EmissionTextureIndex = 0xFFFFFFFF;
	}

	void Material::SetName(const std::string& materialName) noexcept 
	{
		m_Name = materialName; 
	}

	void Material::SetRenderMode(const RenderMode renderMode) noexcept
	{
		RLS_ASSERT(renderMode < RenderMode::Count, "Unknown render mode encountered.");

		m_RenderMode = renderMode;
	}

	bool Material::HasAlbedoTexture() const noexcept
	{
		return m_AlbedoTextureHandle != NULL_HANDLE;
	}

	bool Material::HasMetallicTexture() const noexcept
	{
		return m_MetallicTextureHandle != NULL_HANDLE;
	}

	bool Material::HasRoughnessTexture() const noexcept
	{
		return m_RoughnessTextureHandle != NULL_HANDLE;
	}

	bool Material::HasNormalMap() const noexcept
	{
		return m_NormalMapHandle != NULL_HANDLE;
	}

	bool Material::HasHeightMap() const noexcept
	{
		return m_HeightMapHandle != NULL_HANDLE;
	}

	bool Material::HasAmbientOcclusionTexture() const noexcept
	{
		return m_AmbientOcclusionTextureHandle != NULL_HANDLE;
	}

	bool Material::HasEmissionTexture() const noexcept
	{
		return m_EmissionTextureHandle != NULL_HANDLE;
	}

	Ref<TextureEx> Material::GetAlbedoTexture() const noexcept
	{
		return AssetManager::Get<TextureEx>(m_AlbedoTextureHandle);
	}

	Ref<TextureEx> Material::GetMetallicTexture() const noexcept
	{
		return AssetManager::Get<TextureEx>(m_MetallicTextureHandle);
	}

	Ref<TextureEx> Material::GetRoughnessTexture() const noexcept
	{
		return AssetManager::Get<TextureEx>(m_RoughnessTextureHandle);
	}

	Ref<TextureEx> Material::GetNormalMap() const noexcept
	{
		return AssetManager::Get<TextureEx>(m_NormalMapHandle);
	}

	Ref<TextureEx> Material::GetHeightMap() const noexcept
	{
		return AssetManager::Get<TextureEx>(m_HeightMapHandle);
	}

	Ref<TextureEx> Material::GetAmbientOcclusionTexture() const noexcept
	{
		return AssetManager::Get<TextureEx>(m_AmbientOcclusionTextureHandle);
	}

	Ref<TextureEx> Material::GetEmissionTexture() const noexcept
	{
		return AssetManager::Get<TextureEx>(m_EmissionTextureHandle);
	}

	[[nodiscard]] const RenderMode Material::GetRenderMode() const noexcept
	{
		return m_RenderMode;
	}

	void Material::ToggleAlbedoTextureUsage() noexcept
	{
		m_UseAlbedoTexture = !m_UseAlbedoTexture;
		if (m_UseAlbedoTexture)
		{
			if (m_AlbedoTextureHandle != NULL_HANDLE)
			{
				m_AlbedoTextureIndex = AssetManager::Get<TextureEx>(m_AlbedoTextureHandle)->GetSRVIndex();
			}
		}
		else
		{
			m_AlbedoTextureIndex = 0xFFFFFFFF;
		}
	}

	void Material::ToggleMetallicTextureUsage() noexcept
	{
		m_UseMetallicTexture = !m_UseMetallicTexture;
		if (m_UseMetallicTexture)
		{
			if (m_MetallicTextureHandle != NULL_HANDLE)
			{
				m_MetallicTextureIndex = AssetManager::Get<TextureEx>(m_MetallicTextureHandle)->GetSRVIndex();
			}
		}
		else
		{
			m_MetallicTextureIndex = 0xFFFFFFFF;
		}
	}
	
	void Material::ToggleRoughnessTextureUsage() noexcept
	{
		m_UseRoughnessTexture = !m_UseRoughnessTexture;
		if (m_UseRoughnessTexture)
		{
			if (m_RoughnessTextureHandle != NULL_HANDLE)
			{
				m_RoughnessTextureIndex = AssetManager::Get<TextureEx>(m_RoughnessTextureHandle)->GetSRVIndex();
			}
		}
		else
		{
			m_RoughnessTextureIndex = 0xFFFFFFFF;
		}
	}

	void Material::ToggleNormalMapUsage() noexcept
	{
		m_UseNormalMap = !m_UseNormalMap;
		if (m_UseNormalMap)
		{
			if (m_NormalMapHandle != NULL_HANDLE)
			{
				m_NormalMapIndex = AssetManager::Get<TextureEx>(m_NormalMapHandle)->GetSRVIndex();
			}
		}
		else
		{
			m_NormalMapIndex = 0xFFFFFFFF;
		}
	}

	void Material::ToggleHeightMapUsage() noexcept
	{
		m_UseHeightMap = !m_UseHeightMap;
		if (m_UseHeightMap)
		{
			if (m_HeightMapHandle != NULL_HANDLE)
			{
				m_HeightMapIndex = AssetManager::Get<TextureEx>(m_HeightMapHandle)->GetSRVIndex();
			}
		}
		else
		{
			m_HeightMapIndex = 0xFFFFFFFF;
		}
	}

	void Material::ToggleEmissionTextureUsage() noexcept
	{
		m_UseEmissionTexture = !m_UseEmissionTexture;
		if (m_UseEmissionTexture)
		{
			if (m_EmissionTextureHandle != NULL_HANDLE)
			{
				m_EmissionTextureIndex = AssetManager::Get<TextureEx>(m_EmissionTextureHandle)->GetSRVIndex();
			}
		}
		else
		{
			m_EmissionTextureIndex = 0xFFFFFFFF;
		}
	}

	void Material::ToggleAmbientOcclusionTextureUsage() noexcept
	{
		m_UseAmbientOcclusionTexture = !m_UseAmbientOcclusionTexture;
		if (m_UseAmbientOcclusionTexture)
		{
			if (m_AmbientOcclusionTextureHandle != NULL_HANDLE)
			{
				m_AmbientOcclusionTextureIndex = AssetManager::Get<TextureEx>(m_AmbientOcclusionTextureHandle)->GetSRVIndex();
			}
		}
		else
		{
			m_AmbientOcclusionTextureIndex = 0xFFFFFFFF;
		}
	}
}