#pragma once
#include "Subsystem/ISubsystem.h"

#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/RenderProxy/PostProcessRenderProxy.h"

namespace Relentless
{
	class RLS_API PostProcessRenderSubsystem : public ISubsystem
	{
	public:
		NO_DISCARD const PostProcessRenderProxy& GetRenderProxy() const noexcept;

		static bool ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept;

		void Patch(std::vector<PostProcessRenderProxy> someRenderProxyUpdates) noexcept;

		void Remove(std::vector<uint32> someIDs) noexcept;
	private:
		std::unordered_map<uint32, PostProcessRenderProxy> m_RenderData;
		PostProcessRenderProxy m_DefaultRenderProxy{};
		GraphicsDevice* m_pGraphicsDevice = nullptr;
	};
}
