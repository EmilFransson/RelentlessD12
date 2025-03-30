#pragma once
#include "Graphics/RHI/RHI.h"
#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/RHI/PipelineState.h"

namespace Relentless
{
	class PostProcessing
	{
	public:
		PostProcessing(GraphicsDevice* pDevice) noexcept;
		~PostProcessing() noexcept = default;
		void Render(CommandContext& commandContext, const RenderView& renderView, SceneTextures& sceneTextures) noexcept;
	private:
		GraphicsDevice* m_pDevice = nullptr;

		Ref<PipelineState> m_pPostProcessPSO = nullptr;
	};
}