#pragma once
#include "ECS/Component.h"

namespace Relentless
{
	class TextureCube;

	enum class EFogInscatterMode : uint8 { Uniform = 0u, Cubemap };

	struct RLS_API ExponentialHeightFogComponent : public ManagedComponent<ExponentialHeightFogComponent>
	{
	public:
		ExponentialHeightFogComponent() = default;
		ExponentialHeightFogComponent(ExponentialHeightFogComponent&&) noexcept;
		ExponentialHeightFogComponent& operator=(ExponentialHeightFogComponent&&) noexcept;
		virtual ~ExponentialHeightFogComponent() noexcept override;

		static constexpr uint8 NUM_FOG_LAYERS = 2u;
		
		struct DirtyRenderState{};

		struct FogLayer
		{
			float Density = 0.02f;
			float HeightFalloff = 0.2f;
			float StartDistance = 0.0f;
			float HeightOffset = 0.0f;
		};

		virtual void CopyFrom(const ExponentialHeightFogComponent& aOtherComponent, entity aThisEntity, EntityManager& aEntityManager) override final;

		NO_DISCARD const FogLayer& GetFogLayer(uint8 aLayerIndex) const noexcept;
		NO_DISCARD const Color& GetInscatteringColor() const noexcept;
		NO_DISCARD EFogInscatterMode GetInscatterMode() const noexcept;
		NO_DISCARD Ref<TextureCube> GetInscatterTexture() noexcept;
		NO_DISCARD const AssetHandle& GetInscatterTextureHandle() noexcept;
		NO_DISCARD float GetMaxOpacity() const noexcept;

		void OnBound() noexcept override final;

		void RemoveInscatterTexture() noexcept;

		void SetInscatteringColor(const Color& aColor) noexcept;
		void SetInscatterMode(EFogInscatterMode aInscatterMode) noexcept;
		void SetInscatterTexture(const AssetHandle& aCubemapAssetHandle) noexcept;
		void SetLayerDensity(uint8 aLayerIndex, float aDensity) noexcept;
		void SetLayerHeightFalloff(uint8 aLayerIndex, float aHeightFalloff) noexcept;
		void SetLayerStartDistance(uint8 aLayerIndex, float aStartDistance) noexcept;
		void SetLayerHeightOffset(uint8 aLayerIndex, float aHeightOffset) noexcept;
		void SetMaxOpacity(float aMaxOpacity) noexcept;
	private:
		void ConnectTextureCube() noexcept;

		void DetachTextureCube() noexcept;

		void OnTextureCubeAssetDestroy(MAYBE_UNUSED IAsset* aAsset) noexcept;
		void OnTextureCubeAssetPropertyChanged(MAYBE_UNUSED IAsset* aAsset, MAYBE_UNUSED uint64 aProperty) noexcept;
	private:
		std::array<FogLayer, NUM_FOG_LAYERS> m_FogLayers;
		AssetHandle m_InscatterCubemapHandle = AssetHandle::INVALID;
		Color m_InscatteringColor = Colors::Black;
		float m_MaxOpacity = 1.0f;
		EFogInscatterMode m_InscatterMode = EFogInscatterMode::Uniform;
	};
}