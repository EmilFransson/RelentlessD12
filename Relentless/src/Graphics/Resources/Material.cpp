#include "Material.h"
#include "../MemoryManager.h"
#include "AssetManager.h"
#include "MaterialSerializer.h"
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
		  m_CombinedRoughnessMetallnesMap{false},
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
		  m_ConstantBufferID{static_cast<uint32_t>(-1)},
		  m_RenderMode{ RenderMode::Opaque }
	{
	}

	void Material::SetAlbedoTexture(const AssetHandle& albedoTextureHandle) noexcept
	{
		RLS_ASSERT(albedoTextureHandle != NULL_HANDLE, "Albedo texture handle is invalid.");
		Texture2D& albedoTexture = AssetManager::Get<Texture2D>(albedoTextureHandle);

		RLS_ASSERT(albedoTexture.GetInterface(), "Albedo texture is invalid.");
		RLS_ASSERT(albedoTexture.GetSRVDescriptorHandle().Index != static_cast<uint32_t>(-1), "Albedo texture descriptor index is invalid.");
		
		m_AlbedoTextureHandle = albedoTextureHandle;
		m_AlbedoTextureIndex = albedoTexture.GetSRVDescriptorHandle().Index;
	}

	void Material::SetMetallicTexture(const AssetHandle& metallicTextureHandle) noexcept
	{
		RLS_ASSERT(metallicTextureHandle != NULL_HANDLE, "Metallic texture handle is invalid.");
		Texture2D& metallicTexture = AssetManager::Get<Texture2D>(metallicTextureHandle);

		RLS_ASSERT(metallicTexture.GetInterface(), "Metallic texture is invalid.");
		RLS_ASSERT(metallicTexture.GetSRVDescriptorHandle().Index != static_cast<uint32_t>(-1), "Metallic texture descriptor index is invalid.");

		m_MetallicTextureHandle = metallicTextureHandle;
		m_MetallicTextureIndex = metallicTexture.GetSRVDescriptorHandle().Index;
	}

	void Material::SetRoughnessTexture(const AssetHandle& roughnessTextureHandle) noexcept
	{
		RLS_ASSERT(roughnessTextureHandle != NULL_HANDLE, "Roughness texture handle is invalid.");
		Texture2D& roughnessTexture = AssetManager::Get<Texture2D>(roughnessTextureHandle);

		RLS_ASSERT(roughnessTexture.GetInterface(), "Roughness texture is invalid.");
		RLS_ASSERT(roughnessTexture.GetSRVDescriptorHandle().Index != static_cast<uint32_t>(-1), "Roughness texture descriptor index is invalid.");

		m_RoughnessTextureHandle = roughnessTextureHandle;
		m_RoughnessTextureIndex = roughnessTexture.GetSRVDescriptorHandle().Index;
	}

	void Material::SetNormalMap(const AssetHandle& normalMapHandle) noexcept
	{
		RLS_ASSERT(normalMapHandle != NULL_HANDLE, "Normal map handle is invalid.");
		Texture2D& normalMap = AssetManager::Get<Texture2D>(normalMapHandle);

		RLS_ASSERT(normalMap.GetInterface(), "Normal map is invalid.");
		RLS_ASSERT(normalMap.GetSRVDescriptorHandle().Index != static_cast<uint32_t>(-1), "Normal map descriptor index is invalid.");

		m_NormalMapHandle = normalMapHandle;
		m_NormalMapIndex = normalMap.GetSRVDescriptorHandle().Index;
	}

	void Material::SetHeightMap(const AssetHandle& heightMapHandle) noexcept
	{
		RLS_ASSERT(heightMapHandle != NULL_HANDLE, "Height map handle is invalid.");
		Texture2D& heightMap = AssetManager::Get<Texture2D>(heightMapHandle);

		RLS_ASSERT(heightMap.GetInterface(), "Height map is invalid.");
		RLS_ASSERT(heightMap.GetSRVDescriptorHandle().Index != static_cast<uint32_t>(-1), "Height map descriptor index is invalid.");

		m_HeightMapHandle = heightMapHandle;
		m_HeightMapIndex = heightMap.GetSRVDescriptorHandle().Index;
	}

	void Material::SetAmbientOcclusionTexture(const AssetHandle& ambientOcclusionTextureHandle) noexcept
	{
		RLS_ASSERT(ambientOcclusionTextureHandle != NULL_HANDLE, "Ambient occlusion texture handle is invalid.");
		Texture2D& ambientOcclusionTexture = AssetManager::Get<Texture2D>(ambientOcclusionTextureHandle);

		RLS_ASSERT(ambientOcclusionTexture.GetInterface(), "Ambient occlusion texture is invalid.");
		RLS_ASSERT(ambientOcclusionTexture.GetSRVDescriptorHandle().Index != static_cast<uint32_t>(-1), "Ambient occlusion texture descriptor index is invalid.");

		m_AmbientOcclusionTextureHandle = ambientOcclusionTextureHandle;
		m_AmbientOcclusionTextureIndex = ambientOcclusionTexture.GetSRVDescriptorHandle().Index;
	}

	void Material::SetEmissionTexture(const AssetHandle& emissionTextureHandle) noexcept
	{
		RLS_ASSERT(emissionTextureHandle != NULL_HANDLE, "Emission texture handle is invalid.");
		Texture2D& emissionTexture = AssetManager::Get<Texture2D>(emissionTextureHandle);

		RLS_ASSERT(emissionTexture.GetInterface(), "Emission texture is invalid.");
		RLS_ASSERT(emissionTexture.GetSRVDescriptorHandle().Index != static_cast<uint32_t>(-1), "Emission texture descriptor index is invalid.");

		m_EmissionTextureHandle = emissionTextureHandle;
		m_EmissionTextureIndex = emissionTexture.GetSRVDescriptorHandle().Index;
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

	Texture2D& Material::GetAlbedoTexture() const noexcept
	{
		return AssetManager::Get<Texture2D>(m_AlbedoTextureHandle);
	}

	Texture2D& Material::GetMetallicTexture() const noexcept
	{
		return AssetManager::Get<Texture2D>(m_MetallicTextureHandle);
	}

	Texture2D& Material::GetRoughnessTexture() const noexcept
	{
		return AssetManager::Get<Texture2D>(m_RoughnessTextureHandle);
	}

	Texture2D& Material::GetNormalMap() const noexcept
	{
		return AssetManager::Get<Texture2D>(m_NormalMapHandle);
	}

	Texture2D& Material::GetHeightMap() const noexcept
	{
		return AssetManager::Get<Texture2D>(m_HeightMapHandle);
	}

	Texture2D& Material::GetAmbientOcclusionTexture() const noexcept
	{
		return AssetManager::Get<Texture2D>(m_AmbientOcclusionTextureHandle);
	}

	Texture2D& Material::GetEmissionTexture() const noexcept
	{
		return AssetManager::Get<Texture2D>(m_EmissionTextureHandle);
	}

	[[nodiscard]] const RenderMode Material::GetRenderMode() const noexcept
	{
		return m_RenderMode;
	}


	uint32_t Material::GetConstantBufferIndex() const noexcept
	{
		return MemoryManager::Get().GetCBDescriptorIndex(m_ConstantBufferID);
	}

	void Material::UploadToGPU(const MaterialHandle& materialHandle) noexcept
	{
		AssetManager::GetMaterialManager().Upload(materialHandle);
	}

	void Material::ToggleAlbedoTextureUsage() noexcept
	{
		m_UseAlbedoTexture = !m_UseAlbedoTexture;
		if (m_UseAlbedoTexture)
		{
			if (m_AlbedoTextureHandle != NULL_HANDLE)
			{
				m_AlbedoTextureIndex = AssetManager::Get<Texture2D>(m_AlbedoTextureHandle).GetSRVDescriptorHandle().Index;
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
				m_MetallicTextureIndex = AssetManager::Get<Texture2D>(m_MetallicTextureHandle).GetSRVDescriptorHandle().Index;
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
				m_RoughnessTextureIndex = AssetManager::Get<Texture2D>(m_RoughnessTextureHandle).GetSRVDescriptorHandle().Index;
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
				m_NormalMapIndex = AssetManager::Get<Texture2D>(m_NormalMapHandle).GetSRVDescriptorHandle().Index;
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
				m_HeightMapIndex = AssetManager::Get<Texture2D>(m_HeightMapHandle).GetSRVDescriptorHandle().Index;
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
				m_EmissionTextureIndex = AssetManager::Get<Texture2D>(m_EmissionTextureHandle).GetSRVDescriptorHandle().Index;
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
				m_AmbientOcclusionTextureIndex = AssetManager::Get<Texture2D>(m_AmbientOcclusionTextureHandle).GetSRVDescriptorHandle().Index;
			}
		}
		else
		{
			m_AmbientOcclusionTextureIndex = 0xFFFFFFFF;
		}
	}

	void MaterialManager::Intitialize() noexcept
	{
		m_DefaultMaterialHandle = MaterialSerializer::Deserialize(std::string(ENGINE_ASSET_DIRECTORY) + "Materials/Default-Material.rmat");
	}

	inline static std::mutex g_CreateMutex;
	
	Material& MaterialManager::GetMaterial(const MaterialHandle& materialHandle) noexcept
	{
		const std::lock_guard<std::mutex> lock(g_CreateMutex);

		RLS_ASSERT(m_Materials.size() > materialHandle.Index, "Material handle is invalid.");
		
		return m_Materials[materialHandle.Index];
	}

	MaterialHandle& MaterialManager::GetMaterialHandleByName(const std::string& materialName) noexcept
	{
		const std::lock_guard<std::mutex> lock(g_CreateMutex);

		RLS_ASSERT(Exists(materialName), "Material does not exist.");
		return m_StringToMaterialHandleMap[materialName];
	}

	void MaterialManager::OnMaterialNameChange(const std::string& previousName, const std::string& newName) noexcept
	{
		RLS_ASSERT(Exists(previousName), "Material does not exist.");
		auto it = m_StringToMaterialHandleMap.find(previousName);
		m_StringToMaterialHandleMap[newName] = it->second;
		m_StringToMaterialHandleMap.erase(it);
	}

	MaterialHandle MaterialManager::Create(const std::string& name, const Material& material) noexcept
	{
		return CreateWithUUID(CreateUUID(), name, material);
	}

	MaterialHandle MaterialManager::CreateWithUUID(const UUID& uuid, const std::string& name, const Material& material) noexcept
	{
		const std::lock_guard<std::mutex> lock(g_CreateMutex);

		if (Exists(name))
		{
			return m_StringToMaterialHandleMap[name];
		}
		
		//A new material handle should be created, using the uuid:
		MaterialHandle materialHandle;
		materialHandle.UUID = uuid;

		if (!m_FreeList.empty())
		{
			materialHandle.Index = m_FreeList.front();
			m_FreeList.pop();
			m_Materials[materialHandle.Index] = material;
		}
		else
		{
			materialHandle.Index = m_Materials.size();
			m_Materials.emplace_back(material);
		}
		m_Materials[materialHandle.Index].m_Name = name;
		m_Materials[materialHandle.Index].m_ConstantBufferID = MemoryManager::Get().CreateConstantBuffer(92u);

		m_StringToMaterialHandleMap[name] = materialHandle;

		SetDirty(materialHandle);
		return materialHandle;
	}

	void MaterialManager::Upload(const MaterialHandle& materialHandle) noexcept
	{
		RLS_ASSERT(materialHandle != NULL_HANDLE, "Material handle is invalid.");
		const std::lock_guard<std::mutex> lock(g_CreateMutex);
		RLS_ASSERT(m_Materials.size() > materialHandle.Index, "Material index is invalid.");

		Material& material = m_Materials[materialHandle.Index];
		MemoryManager::Get().UpdateConstantBuffer(material.m_ConstantBufferID, &material);
	}

	void MaterialManager::SetDirty(const MaterialHandle& materialHandle) noexcept
	{
		bool foundMaterial = false;
		for (auto& [handle, remainingUpdates] : m_DirtyMaterials)
		{
			const bool alreadyDirty = (materialHandle.UUID == handle.UUID);
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

	bool MaterialManager::Exists(const std::string& materialName) noexcept
	{
		return m_StringToMaterialHandleMap.contains(materialName);
	}
}