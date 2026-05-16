#include "PostProcessRenderSubsystem.h"

#include "Graphics/Scene/RenderScene.h"

namespace Relentless
{
	const PostProcessRenderProxy& PostProcessRenderSubsystem::GetRenderProxy() const noexcept
	{
		//TODO: This is a temporary solution until proper post process volume resolving is implemented.

		for (auto& [id, renderProxy] : m_RenderData)
			return renderProxy;

		return m_DefaultRenderProxy;
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