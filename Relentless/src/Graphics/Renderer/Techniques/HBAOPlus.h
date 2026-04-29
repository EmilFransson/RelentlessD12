#pragma once

#include "Graphics/RHI/RHI.h"
#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/RHI/PipelineState.h"

class GFSDK_SSAO_Context_D3D12;

namespace Relentless
{
	class HBAOPlus
	{
	public:
		HBAOPlus(GraphicsDevice* pDevice) noexcept;
		void Render(CommandContext& commandContext, const RenderView& renderView, SceneTextures& sceneTextures) noexcept;
	private:
		std::vector<DescriptorHandle> m_ShaderBindableHandles;
		std::vector<DescriptorHandle> m_RTVHandles;

		GraphicsDevice* m_pDevice = nullptr;
		GFSDK_SSAO_Context_D3D12* m_pSSAOContext = nullptr;

		//#if defined(RLS_DEBUG) || defined(RLS_RELWITHDEBINFO)
		bool m_FirstFrameDone = false;
		//#endif
	};
}