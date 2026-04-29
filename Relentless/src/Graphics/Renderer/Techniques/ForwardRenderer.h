#pragma once
#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/RHI/RHI.h"

namespace Relentless
{
	class ForwardRenderer
	{
	public:
		ForwardRenderer(GraphicsDevice* pDevice) noexcept;
		
		void Render(CommandContext& commandContext, const RenderView& renderView, SceneTextures& sceneTextures) noexcept;
	private:
		GraphicsDevice* m_pDevice = nullptr;
	};
}