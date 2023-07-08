#include "Material.h"
#include "../MemoryManager.h"
#include "AssetManager.h"

namespace Relentless
{
	Material::Material() noexcept
		: m_AlbedoColor{1.0f, 1.0f, 1.0f},
		  m_Metallic{0.0f},
		  m_Roughness{0.5f},
		  m_AlbedoTextureIndex{0xFFFFFFFF},
		  m_MetallicTextureIndex{0xFFFFFFFF},
		  m_RoughnessTextureIndex{0xFFFFFFFF},
		  m_NormalMapIndex{0xFFFFFFFF},
		  m_Name{"Unnamed Material"},
		  m_AlbedoTextureHandle{0},
		  m_MetallicTextureHandle{0},
		  m_RoughnessTextureHandle{0},
		  m_NormalMapHandle{0},
		  m_UseAlbedoTexture{true},
		  m_UseMetallicTexture{true},
		  m_UseRoughnessTexture{true},
		  m_UseNormalMap{true},
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

		RLS_ASSERT(pMetallicTexture, "Albedo texture is invalid.");
		RLS_ASSERT(pMetallicTexture->GetSRVDescriptorHandle().Index != static_cast<uint32_t>(-1), "Albedo texture descriptor index is invalid.");

		m_MetallicTextureHandle = metallicTextureHandle;
		m_MetallicTextureIndex = pMetallicTexture->GetSRVDescriptorHandle().Index;
	}

	void Material::SetRoughnessTexture(const ResourceID& roughnessTextureHandle) noexcept
	{
		RLS_ASSERT(roughnessTextureHandle != NULL_RESOURCEID, "Roughness texture handle is invalid.");
		Texture2D* pRoughnessTexture = AssetManager::Get().GetAsset<Texture2D>(roughnessTextureHandle);

		RLS_ASSERT(pRoughnessTexture, "Albedo texture is invalid.");
		RLS_ASSERT(pRoughnessTexture->GetSRVDescriptorHandle().Index != static_cast<uint32_t>(-1), "Albedo texture descriptor index is invalid.");

		m_MetallicTextureHandle = roughnessTextureHandle;
		m_MetallicTextureIndex = pRoughnessTexture->GetSRVDescriptorHandle().Index;
	}

	void Material::SetNormalMap(const ResourceID& normalMapHandle) noexcept
	{
		RLS_ASSERT(normalMapHandle != NULL_RESOURCEID, "Roughness texture handle is invalid.");
		Texture2D* pNormalMap = AssetManager::Get().GetAsset<Texture2D>(normalMapHandle);

		RLS_ASSERT(pNormalMap, "Normal map is invalid.");
		RLS_ASSERT(pNormalMap->GetSRVDescriptorHandle().Index != static_cast<uint32_t>(-1), "Normal map descriptor index is invalid.");

		m_NormalMapHandle = normalMapHandle;
		m_NormalMapIndex = pNormalMap->GetSRVDescriptorHandle().Index;
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
	}
	
	void Material::ToggleRoughnessTextureUsage() noexcept
	{
		m_UseRoughnessTexture = !m_UseRoughnessTexture;
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
		m_IDToMaterialMap[uuID].m_ConstantBufferID = MemoryManager::Get().CreateConstantBuffer(36u);
		m_IDToMaterialMap[uuID].m_Name = name;
		return uuID;
	}

	void MaterialManager::Upload(const MaterialHandle& materialHandle) noexcept
	{
		Material& material = m_IDToMaterialMap[materialHandle];
		MemoryManager::Get().UpdateConstantBuffer(material.m_ConstantBufferID, &material);
	}

	
}