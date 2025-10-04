#pragma once
#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/PipelineState.h"
#include "Graphics/RHI/RHI.h"
#include "Graphics/RHI/Texture.h"

namespace Relentless
{
	class DepthPrePass
	{
	public:
		DepthPrePass(GraphicsDevice* pDevice) noexcept;
		void Render(CommandContext& commandContext, const RenderView& renderView, SceneTextures& sceneTextures) noexcept;
	private:
		GraphicsDevice* m_pDevice = nullptr;

		Ref<Texture> m_pDepthTarget = nullptr;
		Ref<PipelineState> m_pOpaquePSO = nullptr;
	};
}