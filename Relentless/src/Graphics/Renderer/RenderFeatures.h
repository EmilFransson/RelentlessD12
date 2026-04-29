#pragma once

namespace Relentless
{
	inline static constexpr uint32 MAX_RENDER_FEATURES = 128u;

	enum class ERenderFeature : uint32 { EntityPicking = 0u, Grid, HBAOPlus, Outlines, Skybox, Count };
	static_assert((uint32)ERenderFeature::Count <= MAX_RENDER_FEATURES, "ERenderFeature exceeds bitset capacity.");

	class RLS_API RenderFeatures
	{
	public:
		RenderFeatures() noexcept;

		void Disable(ERenderFeature aRenderFeature) noexcept;
		void DisableAll() noexcept;

		void Enable(ERenderFeature aRenderFeature) noexcept;
		void EnableAll() noexcept;

		NO_DISCARD bool IsEnabled(ERenderFeature aRenderFeature) const noexcept;
	private:
		std::bitset<MAX_RENDER_FEATURES> m_FeatureSet;
	};
}