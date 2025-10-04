#pragma once
#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/RHI.h"
#include "Graphics/RHI/PipelineState.h"
#include "Graphics/RHI/Texture.h"

namespace Relentless
{
	class ForwardRenderer
	{
	public:
		ForwardRenderer(GraphicsDevice* pDevice) noexcept;
		~ForwardRenderer() noexcept = default;
		void Render(CommandContext& commandContext, const RenderView& renderView, SceneTextures& sceneTextures, RenderModeEx renderMode) noexcept;
	private:
		GraphicsDevice* m_pDevice = nullptr;

		Ref<PipelineState> m_pForwardSolidPSO = nullptr;
		Ref<PipelineState> m_pForwardWireframePSO = nullptr;
	
		Ref<Texture> m_pColorTarget = nullptr;
	};
}