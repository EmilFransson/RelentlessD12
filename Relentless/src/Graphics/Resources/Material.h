#pragma once
#include "../../Assets/AssetMeta.h"

namespace Relentless
{
	enum class RenderMode : uint8_t {None = 0, Opaque, CutOut, Transparent, Count};

	class Texture2D;
	class Material
	{
	public:
		explicit Material() noexcept;
		~Material() noexcept = default;
		void SetAlbedoTexture(const AssetHandle& albedoTextureHandle) noexcept;
		void SetMetallicTexture(const AssetHandle& metallicTextureHandle) noexcept;
		void SetRoughnessTexture(const AssetHandle& roughnessTextureHandle) noexcept;
		void SetNormalMap(const AssetHandle& normalMapHandle) noexcept;
		void SetHeightMap(const AssetHandle& heightMapHandle) noexcept;
		void SetAmbientOcclusionTexture(const AssetHandle& ambientOcclusionTextureHandle) noexcept;
		void SetEmissionTexture(const AssetHandle& emissionTextureHandle) noexcept;
		void RemoveAlbedoTexture() noexcept;
		void RemoveMetallicTexture() noexcept;
		void RemoveRoughnessTexture() noexcept;
		void RemoveNormalMap() noexcept;
		void RemoveHeightMap() noexcept;
		void RemoveAmbientOcclusionTexture() noexcept;
		void RemoveEmissionTexture() noexcept;
		void SetName(const std::string& materialName) noexcept;
		void SetRenderMode(const RenderMode renderMode) noexcept;
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
		[[nodiscard]] Texture2D& GetAlbedoTexture() const noexcept;
		[[nodiscard]] Texture2D& GetMetallicTexture() const noexcept;
		[[nodiscard]] Texture2D& GetRoughnessTexture() const noexcept;
		[[nodiscard]] Texture2D& GetNormalMap() const noexcept;
		[[nodiscard]] Texture2D& GetHeightMap() const noexcept;
		[[nodiscard]] Texture2D& GetAmbientOcclusionTexture() const noexcept;
		[[nodiscard]] Texture2D& GetEmissionTexture() const noexcept;
		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }
		[[nodiscard]] const RenderMode GetRenderMode() const noexcept;
		[[nodiscard]] uint32_t GetConstantBufferIndex() const noexcept;
		static void UploadToGPU(const AssetHandle& materialHandle) noexcept;
		void ToggleAlbedoTextureUsage() noexcept;
		void ToggleMetallicTextureUsage() noexcept;
		void ToggleRoughnessTextureUsage() noexcept;
		void ToggleNormalMapUsage() noexcept;
		void ToggleHeightMapUsage() noexcept;
		void ToggleAmbientOcclusionTextureUsage() noexcept;
		void ToggleEmissionTextureUsage() noexcept;
	public:
		friend class MaterialManager;
		
		DirectX::XMFLOAT4 m_AlbedoColor;
		DirectX::XMFLOAT4 m_EmissionColor;
		float m_Metallic;
	private:
		float m_Padding[3];
	public:
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
		float m_HeightScale;
		float m_AOScale;
		uint32_t m_CombinedRoughnessMetallnesMap;
		DirectX::XMFLOAT2 m_TilingFactor;
		DirectX::XMFLOAT2 m_Offset;
	private:
		std::string m_Name;
		AssetHandle m_AlbedoTextureHandle;
		AssetHandle m_MetallicTextureHandle;
		AssetHandle m_RoughnessTextureHandle;
		AssetHandle m_NormalMapHandle;
		AssetHandle m_HeightMapHandle;
		AssetHandle m_AmbientOcclusionTextureHandle;
		AssetHandle m_EmissionTextureHandle;

		bool m_UseAlbedoTexture;
		bool m_UseMetallicTexture;
		bool m_UseRoughnessTexture;
		bool m_UseNormalMap;
		bool m_UseHeightMap;
		bool m_UseAmbientOcclusionTexture;
		bool m_UseEmissionTexture;

		size_t m_ConstantBufferID;
		RenderMode m_RenderMode;

		friend class MaterialSerializer;
		friend class Serializer;
	};

	//class MaterialManager
	//{
	//public:
	//	MaterialManager() noexcept = default;
	//	~MaterialManager() noexcept = default;
	//	void Intitialize() noexcept;
	//	[[nodiscard]] Material& GetMaterial(const MaterialHandle& materialHandle) noexcept;
	//	[[nodiscard]] MaterialHandle& GetMaterialHandleByName(const std::string& materialname) noexcept;
	//	[[nodiscard]] MaterialHandle PromoteToHandle(const UUID& uuid) noexcept;
	//	void OnMaterialNameChange(const std::string& previousName, const std::string& newName) noexcept;
	//	[[nodiscard]] MaterialHandle Create(const std::string& name, const Material & = Material()) noexcept;
	//	MaterialHandle CreateWithUUID(const UUID& materialHandle, const std::string& name, const Material & = Material()) noexcept;
	//	__forceinline void Upload(const MaterialHandle& materialHandle) noexcept;
	//	void SetDirty(const MaterialHandle& materialHandle) noexcept;
	//	[[nodiscard]] std::vector<std::pair<MaterialHandle, uint8_t>>& GetDirtyMaterials() noexcept { return m_DirtyMaterials; }
	//	[[nodiscard]] AssetHandle GetDefaultMaterialHandle() noexcept { return m_DefaultMaterialHandle; }
	//	[[nodiscard]] bool Exists(const std::string& materialName) noexcept;
	//private:
	//	std::queue<uint16_t> m_FreeList;
	//	std::vector<Material> m_Materials;
	//	std::unordered_map<std::string, MaterialHandle> m_StringToMaterialHandleMap;
	//	std::vector<std::pair<MaterialHandle, uint8_t>> m_DirtyMaterials;
	//
	//	AssetHandle m_DefaultMaterialHandle;
	//};
}