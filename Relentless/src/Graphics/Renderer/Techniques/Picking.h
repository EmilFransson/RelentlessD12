#pragma once
#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/RHI/RHI.h"

namespace Relentless
{
	class Picking
	{
	public:
		Picking(GraphicsDevice* aDevice) noexcept;
		void Render(CommandContext& aCommandContext, const RenderView& aRenderView, SceneTextures& aSceneTextures, SceneBuffers& aSceneBuffers) noexcept;
	private:
		GraphicsDevice* m_pDevice = nullptr;
	};
}