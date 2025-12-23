#pragma once
#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/RHI.h"
#include "Graphics/RHI/PipelineState.h"
#include "Graphics/RHI/Texture.h"

namespace Relentless
{
	class PostProcessing
	{
	public:
		PostProcessing(GraphicsDevice* pDevice) noexcept;
		~PostProcessing() noexcept = default;
		void Render(CommandContext& commandContext, const RenderView& renderView, SceneTextures& sceneTextures, Ref<Texture> pOutlinesSolidTexture, Ref<Texture> pOutlinesBlurredTexture, Ref<Buffer> pAverageLuminanceBuffer) noexcept;
	private:
		GraphicsDevice* m_pDevice = nullptr;

		Ref<Texture> m_pOutputTarget = nullptr;
		Ref<PipelineState> m_pPostProcessPSO = nullptr;
	};
}