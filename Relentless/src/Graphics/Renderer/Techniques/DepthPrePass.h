#pragma once
#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/RHI/RHI.h"

namespace Relentless
{
	class DepthPrePass
	{
	public:
		DepthPrePass(GraphicsDevice* pDevice) noexcept;
		void Render(CommandContext& commandContext, const RenderView& renderView, SceneTextures& sceneTextures) noexcept;
	private:
		GraphicsDevice* m_pDevice = nullptr;
	};
}