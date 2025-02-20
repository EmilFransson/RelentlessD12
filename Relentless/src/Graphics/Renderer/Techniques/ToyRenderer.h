#pragma once
#include "Graphics/RHI/RHI.h"
#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/RHI/PipelineState.h"

namespace Relentless
{
	class ToyRenderer
	{
	public:
		ToyRenderer(GraphicsDevice* pDevice) noexcept;
		~ToyRenderer() noexcept = default;
		void Render(CommandContext& commandContext, const RenderView& renderView, SceneTextures& sceneTextures) noexcept;
	private:
		void Initialize() noexcept;
	private:
		GraphicsDevice* m_pDevice = nullptr;

		Ref<PipelineState> m_pToyPSO = nullptr;
	};
}