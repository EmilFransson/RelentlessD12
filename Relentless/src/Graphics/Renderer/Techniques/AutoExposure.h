#pragma once
#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/RHI/RHI.h"

namespace Relentless
{
	class AutoExposure
	{
	public:
		AutoExposure(GraphicsDevice* pDevice) noexcept;
		void Render(CommandContext& commandContext, MAYBE_UNUSED const RenderView& aRenderView, SceneTextures& aSceneTextures, SceneBuffers& aSceneBuffers, float aMinLogLuminance, float aMinEV100, float aMaxEV100, float aExposureCompensation) noexcept;
	private:
		GraphicsDevice* m_pDevice = nullptr;
	};
}