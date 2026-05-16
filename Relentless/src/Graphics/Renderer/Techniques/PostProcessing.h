#pragma once
#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/RHI/RHI.h"

namespace Relentless
{
	class PostProcessing
	{
	public:
		PostProcessing(GraphicsDevice* aGraphicsDevice) noexcept;
		void Render(CommandContext& aCommandContext, const RenderView& aRenderView, SceneTextures& aSceneTextures, Ref<Buffer> aAverageLuminanceBuffer) noexcept;
	private:
		GraphicsDevice* m_pDevice = nullptr;
	};
}