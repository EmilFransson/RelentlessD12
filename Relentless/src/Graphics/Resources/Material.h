#pragma once
#include "Assets/AssetMeta.h"
#include "ResourceManager.h"

namespace Relentless
{
	enum class RenderMode : uint8_t {None = 0, Opaque, CutOut, Transparent, Count};

	class Texture2D;
	class Material
	{
	public:
		explicit Material() noexcept;
		Material(const Material& otherMaterial) noexcept;
		Material& operator=(const Material& otherMaterial) noexcept;
		Material(Material&& otherMaterial) noexcept;
		Material& operator=(Material&& otherMaterial) noexcept;
		~Material() noexcept;
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
		[[nodiscard]] std::shared_ptr<Texture2D> GetAlbedoTexture() const noexcept;
		[[nodiscard]] std::shared_ptr<Texture2D> GetMetallicTexture() const noexcept;
		[[nodiscard]] std::shared_ptr<Texture2D> GetRoughnessTexture() const noexcept;
		[[nodiscard]] std::shared_ptr<Texture2D> GetNormalMap() const noexcept;
		[[nodiscard]] std::shared_ptr<Texture2D> GetHeightMap() const noexcept;
		[[nodiscard]] std::shared_ptr<Texture2D> GetAmbientOcclusionTexture() const noexcept;
		[[nodiscard]] std::shared_ptr<Texture2D> GetEmissionTexture() const noexcept;
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
		void Invalidate() noexcept;
	public:
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
	public:
		ResourceHandle m_ConstantBufferHandle = NULL_RESOURCE_HANDLE;
		//size_t m_ConstantBufferID;
		RenderMode m_RenderMode;

		friend class Serializer;
	};
}