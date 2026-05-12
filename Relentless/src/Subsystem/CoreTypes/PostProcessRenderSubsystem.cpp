#include "PostProcessRenderSubsystem.h"

#include "Graphics/Scene/RenderScene.h"

namespace Relentless
{
	const PostProcessRenderProxy& PostProcessRenderSubsystem::GetRenderProxy() const noexcept
	{
		if (m_RenderData.empty())
			return m_DefaultRenderProxy;

		for (auto& [id, renderProxy] : m_RenderData)
			return renderProxy;
	}

	bool PostProcessRenderSubsystem::ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept
	{
		return dynamic_cast<RenderScene*>(aSystemManager) != nullptr;
	}

	void PostProcessRenderSubsystem::Patch(std::vector<PostProcessRenderProxy> someRenderProxyUpdates) noexcept
	{
		for (auto& renderProxy : someRenderProxyUpdates)
			m_RenderData[renderProxy.ID] = renderProxy;
	}

	void PostProcessRenderSubsystem::Remove(std::vector<uint32> someIDs) noexcept
	{
		for (auto& id : someIDs)
			m_RenderData.erase(id);
	}
}