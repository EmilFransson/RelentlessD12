#include "RenderFeatures.h"

namespace Relentless
{
	RenderFeatures::RenderFeatures() noexcept
	{
		EnableAll();
	}

	void RenderFeatures::Disable(ERenderFeature aRenderFeature) noexcept
	{
		m_FeatureSet.reset(static_cast<size_t>(aRenderFeature));
	}

	void RenderFeatures::DisableAll() noexcept
	{
		m_FeatureSet.reset();
	}

	void RenderFeatures::Enable(ERenderFeature aRenderFeature) noexcept
	{
		m_FeatureSet.set(static_cast<size_t>(aRenderFeature));
	}

	void RenderFeatures::EnableAll() noexcept
	{
		m_FeatureSet.set();
	}

	bool RenderFeatures::IsEnabled(ERenderFeature aRenderFeature) const noexcept
	{
		return m_FeatureSet.test(static_cast<size_t>(aRenderFeature));
	}
}