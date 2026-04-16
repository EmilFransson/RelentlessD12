#pragma once
#include "Graphics/RHI/PipelineState.h"

namespace Relentless
{
	class GraphicsDevice;
	struct RenderView;
	struct SceneTextures;

	class SkyBoxRenderer
	{
	public:
		SkyBoxRenderer(GraphicsDevice* aGraphicsDevice) noexcept;
		void Render(const RenderView& aRenderView, SceneTextures& aSceneTextures) noexcept;
	private:
		GraphicsDevice* m_pGraphicsDevice = nullptr;
		Ref<PipelineState> m_pSkyBoxPSO = nullptr;
		Ref<PipelineState> m_pSkyBoxBlendPSO = nullptr;
	};
}