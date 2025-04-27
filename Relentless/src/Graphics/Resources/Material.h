#pragma once
#include "Assets/AssetMeta.h"
#include "Core/IAsset.h"
#include "Core/Ref.h"

namespace Relentless
{
	enum class EBlendMode : uint8_t {Opaque, AlphaMask, AlphaBlend};

	class Texture;
	class Material : public IAsset, public RefCounted<Material>
	{
	public:
		explicit Material() noexcept;
		Material(const Material& otherMaterial) noexcept;
		Material& operator=(const Material& otherMaterial) noexcept;
		Material(Material&& otherMaterial) noexcept;
		Material& operator=(Material&& otherMaterial) noexcept;
		virtual ~Material() noexcept override = default;
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
		void SetBlendMode(const EBlendMode blendMode) noexcept;
		void SetIsTwoSided(bool state) noexcept;
		[[nodiscard]] bool HasAlbedoTexture() const noexcept;
		[[nodiscard]] bool HasMetallicTexture() const noexcept;
		[[nodiscard]] bool HasRoughnessTexture() const noexcept;
		[[nodiscard]] bool HasNormalMap() const noexcept;
		[[nodiscard]] bool HasHeightMap() const noexcept;
		[[nodiscard]] bool HasAmbientOcclusionTexture() const noexcept;
		[[nodiscard]] bool HasEmissionTexture() const noexcept;
		[[nodiscard]] Ref<Texture> GetAlbedoTexture() const noexcept;
		[[nodiscard]] Ref<Texture> GetMetallicTexture() const noexcept;
		[[nodiscard]] Ref<Texture> GetRoughnessTexture() const noexcept;
		[[nodiscard]] Ref<Texture> GetNormalMap() const noexcept;
		[[nodiscard]] Ref<Texture> GetHeightMap() const noexcept;
		[[nodiscard]] Ref<Texture> GetAmbientOcclusionTexture() const noexcept;
		[[nodiscard]] Ref<Texture> GetEmissionTexture() const noexcept;
		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }
		[[nodiscard]] const EBlendMode GetBlendMode() const noexcept;
		void ToggleAlbedoTextureUsage() noexcept;
		void ToggleMetallicTextureUsage() noexcept;
		void ToggleRoughnessTextureUsage() noexcept;
		void ToggleNormalMapUsage() noexcept;
		void ToggleHeightMapUsage() noexcept;
		void ToggleAmbientOcclusionTextureUsage() noexcept;
		void ToggleEmissionTextureUsage() noexcept;
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
		uint32 m_AlbedoTextureIndex;
		uint32 m_MetallicTextureIndex;
		uint32 m_RoughnessTextureIndex;
		uint32 m_NormalMapIndex;
		uint32 m_HeightMapIndex;
		uint32 m_AmbientOcclusionTextureIndex;
		uint32 m_EmissionTextureIndex;
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

		bool m_IsTwoSided = false;
	public:
		EBlendMode m_BlendMode = EBlendMode::Opaque;

		friend class Serializer;
	};
}