#pragma once
#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/RHI/RHI.h"

namespace Relentless
{
	class EditorGrid
	{
	public:
		EditorGrid(GraphicsDevice* pDevice) noexcept;
		~EditorGrid() noexcept = default;
		void Render(CommandContext& commandContext, const RenderView& renderView, SceneTextures& sceneTextures) noexcept;
	private:
		GraphicsDevice* m_pDevice = nullptr;

		Ref<PipelineState> m_pGridPSO = nullptr;
		Ref<BufferEx> m_pInstancesStructuredBuffer = nullptr;
	};
}