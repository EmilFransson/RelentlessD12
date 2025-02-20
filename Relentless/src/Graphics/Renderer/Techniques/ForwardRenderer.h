#pragma once
#include "Graphics/RHI/RHI.h"
#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/RHI/PipelineState.h"


namespace Relentless
{
	class ForwardRenderer
	{
	public:
		ForwardRenderer(GraphicsDevice* pDevice) noexcept;
		~ForwardRenderer() noexcept = default;
		void Render(CommandContext& commandContext, const RenderView& renderView, SceneTextures& sceneTextures) noexcept;
	private:
		void Initialize(uint32 samples) noexcept;
	private:
		GraphicsDevice* m_pDevice = nullptr;

		Ref<PipelineState> m_pForwardPSO = nullptr;
	};
}