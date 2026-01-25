#pragma once
#include "Assets/AssetMeta.h"

#include "Core/DLLExport.h"
#include "Core/IAsset.h"
#include "Core/Ref.h"

namespace Relentless
{
	enum class EBlendMode	: uint8 { Opaque = 0u, AlphaMask, AlphaBlend };
	enum class ETextureType : uint8 { Albedo = 0u, Metallic, Roughness, NormalMap, DisplacementMap, AmbientOcclusion, Emission, Count };

	struct UVTransform
	{
		Vector2 TilingFactor	= Vector2::One;
		Vector2 Offset			= Vector2::Zero;
	};

	struct MaterialTextureEntry
	{
		AssetHandle TextureHandle	= NULL_HANDLE;
		UVTransform UVTransform;
		bool IsEnabled				= true;

		bool Serialize(IArchive& aArchive)
		{
			return aArchive.Process(TextureHandle)
				&& aArchive.Process(UVTransform)
				&& aArchive.Process(IsEnabled);
		}
	};

	class Texture2D;
	class RLS_API Material : public AssetBase<Material>
	{
	public:
		Material(const UUID& aUUID) noexcept;
		Material() noexcept = default;

		NO_DISCARD const Vector4& GetAlbedoColor() const noexcept;
		NO_DISCARD float GetAmbientOcclusionIntensity() const noexcept;
		NO_DISCARD EBlendMode GetBlendMode() const noexcept;
		NO_DISCARD float GetDisplacementIntensity() const noexcept;
		NO_DISCARD const Vector4& GetEmissiveColor() const noexcept;
		NO_DISCARD float GetEmissiveIntensity() const noexcept;
		NO_DISCARD const Vector2& GetGlobalOffset() const noexcept;
		NO_DISCARD const Vector2& GetGlobalTilingFactor() const noexcept;
		NO_DISCARD float GetMetalness() const noexcept;
		NO_DISCARD float GetRoughness() const noexcept;
		NO_DISCARD Ref<Texture2D> GetTexture(ETextureType aTextureType) const noexcept;

		NO_DISCARD bool HasTexture(ETextureType aTextureType) const noexcept;

		NO_DISCARD bool IsTwoSided() const noexcept;

		static constexpr const UUID& PersistentType()
		{
			static constexpr UUID uid = UUID{ 0xb881963d, 0xe495, 0x4d1f, { 0xaf, 0x88, 0x8, 0x72, 0xf9, 0xb6, 0x1, 0x5 } };
			return uid;
		}

		void RemoveTexture(ETextureType aTextureType) noexcept;

		virtual bool SerializeCore(IArchive& aArchive) noexcept override;
		void SetAlbedoColor(const Vector4& aColor) noexcept;
		void SetAlbedoColor(const Color& aColor) noexcept;
		void SetAmbientOcclusionIntensity(float aAmbientOcclusionIntensity) noexcept;
		void SetBlendMode(const EBlendMode aBlendMode) noexcept;
		void SetDisplacementIntensity(float aDisplacementIntensity) noexcept;
		void SetEmissiveColor(const Vector4& aColor) noexcept;
		void SetEmissiveColor(const Color& aColor) noexcept;
		void SetEmissiveIntensity(float aEmissiveIntensity) noexcept;
		void SetGlobalOffset(const Vector2& aOffset) noexcept;
		void SetGlobalTilingFactor(const Vector2& aTilingFactor) noexcept;
		void SetIsTwoSided(bool aIsTwoSided) noexcept;
		void SetMetalness(float aMetalness) noexcept;
		void SetRoughness(float aRoughness) noexcept;
		void SetTexture(ETextureType aTextureType, const AssetHandle& aTextureHandle) noexcept;
		void SetTextureEnabled(ETextureType aTextureType, bool aEnable) noexcept;
	private:
		std::array<MaterialTextureEntry, (size_t)ETextureType::Count> m_Textures;

		Vector4 m_AlbedoColor				= Vector4(0.5f, 0.5f, 0.5f, 1.0f);
		Vector4 m_EmissiveColor				= Vector4::Zero;

		UVTransform m_GlobalUVTransform;

		EBlendMode m_BlendMode				= EBlendMode::Opaque;
		float m_Metallic					= 0.0f;
		float m_EmissionIntensity			= 0.0f;
		float m_Roughness					= 0.5f;
		float m_DisplacementIntensity		= 1.0f;
		float m_AmbientOcclusionIntensity	= 1.0f;
		bool m_IsTwoSided					= false;
	};
}