#include "Material.h"
#include "../MemoryManager.h"
#include "AssetManager.h"
#include "../D3D12Core.h"

namespace Relentless
{
	Material::Material() noexcept
		: m_AlbedoColor{1.0f, 1.0f, 1.0f},
		  m_Metallic{0.0f},
		  m_Roughness{0.5f},
		  m_EmissionColor{0.0f, 0.0f, 0.0f},
		  m_EmissionIntensity{0.0f},
		  m_HeightScale{0.02f},
		  m_AOScale{1.0f},
		  m_AlbedoTextureIndex{0xFFFFFFFF},
		  m_MetallicTextureIndex{0xFFFFFFFF},
		  m_RoughnessTextureIndex{0xFFFFFFFF},
		  m_NormalMapIndex{0xFFFFFFFF},
		  m_HeightMapIndex{0xFFFFFFFF},
		  m_AmbientOcclusionTextureIndex{ 0xFFFFFFFF },
		  m_EmissionTextureIndex{ 0xFFFFFFFF },
		  m_TilingFactor{1.0f, 1.0f},
		  m_Offset{ 0.0f, 0.0f },
		  m_Name{"Unnamed Material"},
		  m_AlbedoTextureHandle{0},
		  m_MetallicTextureHandle{0},
		  m_RoughnessTextureHandle{0},
		  m_NormalMapHandle{0},
		  m_HeightMapHandle{0},
		  m_AmbientOcclusionTextureHandle{ 0 },
		  m_EmissionTextureHandle{ 0 },
		  m_UseAlbedoTexture{true},
		  m_UseMetallicTexture{true},
		  m_UseRoughnessTexture{true},
		  m_UseHeightMap{true},
		  m_UseNormalMap{true},
		  m_UseAmbientOcclusionTexture{ true },
	 	  m_UseEmissionTexture{ true },
		  m_ConstantBufferID{static_cast<uint32_t>(-1)}
	{
	}

	void Material::SetAlbedoTexture(const ResourceID& albedoTextureHandle) noexcept
	{
		RLS_ASSERT(albedoTextureHandle != NULL_RESOURCEID, "Albedo texture handle is invalid.");
		Texture2D* pAlbedoTexture = AssetManager::Get().GetAsset<Texture2D>(albedoTextureHandle);

		RLS_ASSERT(pAlbedoTexture, "Albedo texture is invalid.");
		RLS_ASSERT(pAlbedoTexture->GetSRVDescriptorHandle().Index != static_cast<uint32_t>(-1), "Albedo texture descriptor index is invalid.");
		
		m_AlbedoTextureHandle = albedoTextureHandle;
		m_AlbedoTextureIndex = pAlbedoTexture->GetSRVDescriptorHandle().Index;
	}

	void Material::SetMetallicTexture(const ResourceID& metallicTextureHandle) noexcept
	{
		RLS_ASSERT(metallicTextureHandle != NULL_RESOURCEID, "Metallic texture handle is invalid.");
		Texture2D* pMetallicTexture = AssetManager::Get().GetAsset<Texture2D>(metallicTextureHandle);

		RLS_ASSERT(pMetallicTexture, "Metallic texture is invalid.");
		RLS_ASSERT(pMetallicTexture->GetSRVDescriptorHandle().Index != static_cast<uint32_t>(-1), "Metallic texture descriptor index is invalid.");

		m_MetallicTextureHandle = metallicTextureHandle;
		m_MetallicTextureIndex = pMetallicTexture->GetSRVDescriptorHandle().Index;
	}

	void Material::SetRoughnessTexture(const ResourceID& roughnessTextureHandle) noexcept
	{
		RLS_ASSERT(roughnessTextureHandle != NULL_RESOURCEID, "Roughness texture handle is invalid.");
		Texture2D* pRoughnessTexture = AssetManager::Get().GetAsset<Texture2D>(roughnessTextureHandle);

		RLS_ASSERT(pRoughnessTexture, "Roughness texture is invalid.");
		RLS_ASSERT(pRoughnessTexture->GetSRVDescriptorHandle().Index != static_cast<uint32_t>(-1), "Roughness texture descriptor index is invalid.");

		m_RoughnessTextureHandle = roughnessTextureHandle;
		m_RoughnessTextureIndex = pRoughnessTexture->GetSRVDescriptorHandle().Index;
	}

	void Material::SetNormalMap(const ResourceID& normalMapHandle) noexcept
	{
		RLS_ASSERT(normalMapHandle != NULL_RESOURCEID, "Normal map handle is invalid.");
		Texture2D* pNormalMap = AssetManager::Get().GetAsset<Texture2D>(normalMapHandle);

		RLS_ASSERT(pNormalMap, "Normal map is invalid.");
		RLS_ASSERT(pNormalMap->GetSRVDescriptorHandle().Index != static_cast<uint32_t>(-1), "Normal map descriptor index is invalid.");

		m_NormalMapHandle = normalMapHandle;
		m_NormalMapIndex = pNormalMap->GetSRVDescriptorHandle().Index;
	}

	void Material::SetHeightMap(const ResourceID& heightMapHandle) noexcept
	{
		RLS_ASSERT(heightMapHandle != NULL_RESOURCEID, "Height map handle is invalid.");
		Texture2D* pHeightMap = AssetManager::Get().GetAsset<Texture2D>(heightMapHandle);

		RLS_ASSERT(pHeightMap, "Height map is invalid.");
		RLS_ASSERT(pHeightMap->GetSRVDescriptorHandle().Index != static_cast<uint32_t>(-1), "Height map descriptor index is invalid.");

		m_HeightMapHandle = heightMapHandle;
		m_HeightMapIndex = pHeightMap->GetSRVDescriptorHandle().Index;
	}

	void Material::SetAmbientOcclusionTexture(const ResourceID& ambientOcclusionTextureHandle) noexcept
	{
		RLS_ASSERT(ambientOcclusionTextureHandle != NULL_RESOURCEID, "Ambient occlusion texture handle is invalid.");
		Texture2D* pAmbientOcclusionTexture = AssetManager::Get().GetAsset<Texture2D>(ambientOcclusionTextureHandle);

		RLS_ASSERT(pAmbientOcclusionTexture, "Ambient occlusion texture is invalid.");
		RLS_ASSERT(pAmbientOcclusionTexture->GetSRVDescriptorHandle().Index != static_cast<uint32_t>(-1), "Ambient occlusion texture descriptor index is invalid.");

		m_AmbientOcclusionTextureHandle = ambientOcclusionTextureHandle;
		m_AmbientOcclusionTextureIndex = pAmbientOcclusionTexture->GetSRVDescriptorHandle().Index;
	}

	void Material::SetEmissionTexture(const ResourceID& emissionTextureHandle) noexcept
	{
		RLS_ASSERT(emissionTextureHandle != NULL_RESOURCEID, "Emission texture handle is invalid.");
		Texture2D* pEmissionTexture = AssetManager::Get().GetAsset<Texture2D>(emissionTextureHandle);

		RLS_ASSERT(pEmissionTexture, "Emission texture is invalid.");
		RLS_ASSERT(pEmissionTexture->GetSRVDescriptorHandle().Index != static_cast<uint32_t>(-1), "Emission texture descriptor index is invalid.");

		m_EmissionTextureHandle = emissionTextureHandle;
		m_EmissionTextureIndex = pEmissionTexture->GetSRVDescriptorHandle().Index;
	}

	bool Material::HasAlbedoTexture() const noexcept
	{
		return m_AlbedoTextureHandle != NULL_RESOURCEID;
	}

	bool Material::HasMetallicTexture() const noexcept
	{
		return m_MetallicTextureHandle != NULL_RESOURCEID;
	}

	bool Material::HasRoughnessTexture() const noexcept
	{
		return m_RoughnessTextureHandle != NULL_RESOURCEID;
	}

	bool Material::HasNormalMap() const noexcept
	{
		return m_NormalMapHandle != NULL_RESOURCEID;
	}

	bool Material::HasHeightMap() const noexcept
	{
		return m_HeightMapHandle != NULL_RESOURCEID;
	}

	bool Material::HasAmbientOcclusionTexture() const noexcept
	{
		return m_AmbientOcclusionTextureHandle != NULL_RESOURCEID;
	}

	bool Material::HasEmissionTexture() const noexcept
	{
		return m_EmissionTextureHandle != NULL_RESOURCEID;
	}

	bool Material::ShouldUseAlbedoTexture() const noexcept
	{
		return m_UseAlbedoTexture;
	}

	bool Material::ShouldUseMetallicTexture() const noexcept
	{
		return m_UseMetallicTexture;
	}

	bool Material::ShouldUseRoughnessTexture() const noexcept
	{
		return m_UseRoughnessTexture;
	}

	bool Material::ShouldUseNormalMap() const noexcept
	{
		return m_UseNormalMap;
	}

	bool Material::ShouldUseHeightMap() const noexcept
	{
		return m_UseHeightMap;
	}

	bool Material::ShouldUseAmbientOcclusionTexture() const noexcept
	{
		return m_UseAmbientOcclusionTexture;
	}

	bool Material::ShouldUseEmissionTexture() const noexcept
	{
		return m_UseEmissionTexture;
	}

	Texture2D* Material::GetAlbedoTexture() const noexcept
	{
		return AssetManager::Get().GetAsset<Texture2D>(m_AlbedoTextureHandle);
	}

	Texture2D* Material::GetMetallicTexture() const noexcept
	{
		return AssetManager::Get().GetAsset<Texture2D>(m_MetallicTextureHandle);
	}

	Texture2D* Material::GetRoughnessTexture() const noexcept
	{
		return AssetManager::Get().GetAsset<Texture2D>(m_RoughnessTextureHandle);
	}

	Texture2D* Material::GetNormalMap() const noexcept
	{
		return AssetManager::Get().GetAsset<Texture2D>(m_NormalMapHandle);
	}

	Texture2D* Material::GetHeightMap() const noexcept
	{
		return AssetManager::Get().GetAsset<Texture2D>(m_HeightMapHandle);
	}

	Texture2D* Material::GetAmbientOcclusionTexture() const noexcept
	{
		return AssetManager::Get().GetAsset<Texture2D>(m_AmbientOcclusionTextureHandle);
	}

	Texture2D* Material::GetEmissionTexture() const noexcept
	{
		return AssetManager::Get().GetAsset<Texture2D>(m_EmissionTextureHandle);
	}

	size_t Material::GetConstantBufferIndex() const noexcept
	{
		return MemoryManager::Get().GetCBDescriptorIndex(m_ConstantBufferID);
	}

	void Material::UploadToGPU(const MaterialHandle& materialHandle) noexcept
	{
		AssetManager::Get().Upload<Material>(materialHandle);
	}

	void Material::ToggleAlbedoTextureUsage() noexcept
	{
		m_UseAlbedoTexture = !m_UseAlbedoTexture;
		if (m_UseAlbedoTexture)
		{
			if (m_AlbedoTextureHandle != NULL_RESOURCEID)
			{

				m_AlbedoTextureIndex = AssetManager::Get().GetAsset<Texture2D>(m_AlbedoTextureHandle)->GetSRVDescriptorHandle().Index;
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
			if (m_MetallicTextureHandle != NULL_RESOURCEID)
			{

				m_MetallicTextureIndex = AssetManager::Get().GetAsset<Texture2D>(m_MetallicTextureHandle)->GetSRVDescriptorHandle().Index;
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
			if (m_RoughnessTextureHandle != NULL_RESOURCEID)
			{

				m_RoughnessTextureIndex = AssetManager::Get().GetAsset<Texture2D>(m_RoughnessTextureHandle)->GetSRVDescriptorHandle().Index;
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
			if (m_NormalMapHandle != NULL_RESOURCEID)
			{

				m_NormalMapIndex = AssetManager::Get().GetAsset<Texture2D>(m_NormalMapHandle)->GetSRVDescriptorHandle().Index;
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
			if (m_HeightMapHandle != NULL_RESOURCEID)
			{

				m_HeightMapIndex = AssetManager::Get().GetAsset<Texture2D>(m_HeightMapHandle)->GetSRVDescriptorHandle().Index;
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
			if (m_EmissionTextureHandle != NULL_RESOURCEID)
			{

				m_EmissionTextureIndex = AssetManager::Get().GetAsset<Texture2D>(m_EmissionTextureHandle)->GetSRVDescriptorHandle().Index;
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
			if (m_AmbientOcclusionTextureHandle != NULL_RESOURCEID)
			{

				m_AmbientOcclusionTextureIndex = AssetManager::Get().GetAsset<Texture2D>(m_AmbientOcclusionTextureHandle)->GetSRVDescriptorHandle().Index;
			}
		}
		else
		{
			m_AmbientOcclusionTextureIndex = 0xFFFFFFFF;
		}
	}

	Material& MaterialManager::Get(const MaterialHandle& materialHandle) noexcept
	{
		RLS_ASSERT(m_IDToMaterialMap.contains(materialHandle), "Material handle is invalid.");
		return m_IDToMaterialMap[materialHandle];
	}

	MaterialHandle MaterialManager::Create(const std::string& name, const Material& material) noexcept
	{
		UUID uuID;
		#if defined RLS_DEBUG
		RLS_ASSERT(::UuidCreate(&uuID) == RPC_S_OK, "Failed to generate UUID.");
		#else
		::UuidCreate(&uuID);
		#endif

		m_IDToMaterialMap[uuID] = material;
		m_IDToMaterialMap[uuID].m_ConstantBufferID = MemoryManager::Get().CreateConstantBuffer(88u);
		m_IDToMaterialMap[uuID].m_Name = name;

		SetDirty(uuID);
		return uuID;
	}

	void MaterialManager::Upload(const MaterialHandle& materialHandle) noexcept
	{
		Material& material = m_IDToMaterialMap[materialHandle];
		MemoryManager::Get().UpdateConstantBuffer(material.m_ConstantBufferID, &material);
	}

	void MaterialManager::SetDirty(const MaterialHandle& materialHandle) noexcept
	{
		bool foundMaterial = false;
		for (auto& [handle, remainingUpdates] : m_DirtyMaterials)
		{
			const bool alreadyDirty = (materialHandle == handle);
			if (alreadyDirty)
			{
				remainingUpdates = D3D12Core::GetNrOfBufferedFrames();
				foundMaterial = true;
				break;
			}
		}

		if (!foundMaterial)
		{
			m_DirtyMaterials.push_back({materialHandle, D3D12Core::GetNrOfBufferedFrames()});
		}
	}
}