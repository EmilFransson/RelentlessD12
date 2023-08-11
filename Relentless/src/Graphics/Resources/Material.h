#pragma once

#include "Helper.h"
namespace Relentless
{
	typedef UUID ResourceID;
	typedef ResourceID MaterialHandle;
	#define NULL_RESOURCEID UUID(0)

	class Texture2D;
	class Material
	{
	public:
		explicit Material() noexcept;
		~Material() noexcept = default;
		void SetAlbedoTexture(const ResourceID& albedoTextureHandle) noexcept;
		void SetMetallicTexture(const ResourceID& metallicTextureHandle) noexcept;
		void SetRoughnessTexture(const ResourceID& roughnessTextureHandle) noexcept;
		void SetNormalMap(const ResourceID& normalMapHandle) noexcept;
		void SetHeightMap(const ResourceID& heightMapHandle) noexcept;
		void SetAmbientOcclusionTexture(const ResourceID& ambientOcclusionTextureHandle) noexcept;
		void SetEmissionTexture(const ResourceID& emissionTextureHandle) noexcept;
		void SetName(const std::string& materialName) noexcept { m_Name = materialName; }
		[[nodiscard]] bool HasAlbedoTexture() const noexcept;
		[[nodiscard]] bool HasMetallicTexture() const noexcept;
		[[nodiscard]] bool HasRoughnessTexture() const noexcept;
		[[nodiscard]] bool HasNormalMap() const noexcept;
		[[nodiscard]] bool HasHeightMap() const noexcept;
		[[nodiscard]] bool HasAmbientOcclusionTexture() const noexcept;
		[[nodiscard]] bool HasEmissionTexture() const noexcept;
		[[nodiscard]] bool ShouldUseAlbedoTexture() const noexcept;
		[[nodiscard]] bool ShouldUseMetallicTexture() const noexcept;
		[[nodiscard]] bool ShouldUseRoughnessTexture() const noexcept;
		[[nodiscard]] bool ShouldUseNormalMap() const noexcept;
		[[nodiscard]] bool ShouldUseHeightMap() const noexcept;
		[[nodiscard]] bool ShouldUseAmbientOcclusionTexture() const noexcept;
		[[nodiscard]] bool ShouldUseEmissionTexture() const noexcept;
		[[nodiscard]] Texture2D* GetAlbedoTexture() const noexcept;
		[[nodiscard]] Texture2D* GetMetallicTexture() const noexcept;
		[[nodiscard]] Texture2D* GetRoughnessTexture() const noexcept;
		[[nodiscard]] Texture2D* GetNormalMap() const noexcept;
		[[nodiscard]] Texture2D* GetHeightMap() const noexcept;
		[[nodiscard]] Texture2D* GetAmbientOcclusionTexture() const noexcept;
		[[nodiscard]] Texture2D* GetEmissionTexture() const noexcept;
		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }
		[[nodiscard]] uint32_t GetConstantBufferIndex() const noexcept;
		static void UploadToGPU(const MaterialHandle& materialHandle) noexcept;
		void ToggleAlbedoTextureUsage() noexcept;
		void ToggleMetallicTextureUsage() noexcept;
		void ToggleRoughnessTextureUsage() noexcept;
		void ToggleNormalMapUsage() noexcept;
		void ToggleHeightMapUsage() noexcept;
		void ToggleAmbientOcclusionTextureUsage() noexcept;
		void ToggleEmissionTextureUsage() noexcept;

	public:
		friend class MaterialManager;
		
		DirectX::XMFLOAT3 m_AlbedoColor;
		float m_Metallic;
		DirectX::XMFLOAT3 m_EmissionColor;
		float m_EmissionIntensity;
		float m_Roughness;
	private:
		uint32_t m_AlbedoTextureIndex;
		uint32_t m_MetallicTextureIndex;
		uint32_t m_RoughnessTextureIndex;
		uint32_t m_NormalMapIndex;
		uint32_t m_HeightMapIndex;
		uint32_t m_AmbientOcclusionTextureIndex;
		uint32_t m_EmissionTextureIndex;
	public:
		DirectX::XMFLOAT2 m_TilingFactor;
		DirectX::XMFLOAT2 m_Offset;
		float m_HeightScale;
		float m_AOScale;
		uint32_t m_CombinedRoughnessMetallnesMap;
	private:
		std::string m_Name;
		ResourceID m_AlbedoTextureHandle;
		ResourceID m_MetallicTextureHandle;
		ResourceID m_RoughnessTextureHandle;
		ResourceID m_NormalMapHandle;
		ResourceID m_HeightMapHandle;
		ResourceID m_AmbientOcclusionTextureHandle;
		ResourceID m_EmissionTextureHandle;

		bool m_UseAlbedoTexture;
		bool m_UseMetallicTexture;
		bool m_UseRoughnessTexture;
		bool m_UseNormalMap;
		bool m_UseHeightMap;
		bool m_UseAmbientOcclusionTexture;
		bool m_UseEmissionTexture;

		size_t m_ConstantBufferID;
	};
	
	class MaterialManager
	{
	public:
		explicit MaterialManager() noexcept = default;
		~MaterialManager() noexcept = default;
		[[nodiscard]] Material& Get(const MaterialHandle& materialHandle) noexcept;
		[[nodiscard]] MaterialHandle Create(const std::string& name = std::string("Unnamed Material"), const Material& = Material()) noexcept;
		__forceinline void Upload(const MaterialHandle& materialHandle) noexcept;
		void SetDirty(const MaterialHandle& materialHandle) noexcept;
		[[nodiscard]] std::vector<std::pair<MaterialHandle, uint8_t>>& GetDirtyMaterials() noexcept { return m_DirtyMaterials; }

	private:
		std::unordered_map<MaterialHandle, Material> m_IDToMaterialMap;
		std::vector<std::pair<MaterialHandle, uint8_t>> m_DirtyMaterials;
	};
}