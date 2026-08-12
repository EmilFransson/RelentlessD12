#pragma once
//Need to be kept in sync with CommonBindings.hlsli

namespace Relentless
{
	inline static constexpr uint32 MAX_RENDER_FEATURES = 128u;

	enum class ERenderFeature : uint32 { EntityPicking = 0u, Grid, HBAOPlus, Outlines, Skybox, ExponentialHeightFog, Bloom, AutoExposure, ToneMap, Count };
	static_assert((uint32)ERenderFeature::Count <= MAX_RENDER_FEATURES, "ERenderFeature exceeds bitset capacity.");

	class RLS_API RenderFeatures
	{
	public:
		RenderFeatures() noexcept;

		NO_DISCARD static RenderFeatures Disabled() noexcept;
		void Disable(ERenderFeature aRenderFeature) noexcept;
		void DisableAll() noexcept;

		NO_DISCARD static RenderFeatures Enabled() noexcept;
		void Enable(ERenderFeature aRenderFeature) noexcept;
		void EnableAll() noexcept;

		NO_DISCARD bool IsEnabled(ERenderFeature aRenderFeature) const noexcept;

		NO_DISCARD uint32 ToShaderMask() const noexcept
		{
			static_assert((uint32)ERenderFeature::Count <= 32u, "ERenderFeature exceeds the uint32 shader mask; widen cView.RenderFeatures to uint4.");

			uint32 mask = 0u;
			for (uint32 i = 0u; i < (uint32)ERenderFeature::Count; ++i)
				mask |= (uint32)m_FeatureSet.test(i) << i;
			return mask;
		}
	private:
		std::bitset<MAX_RENDER_FEATURES> m_FeatureSet;
	};
}