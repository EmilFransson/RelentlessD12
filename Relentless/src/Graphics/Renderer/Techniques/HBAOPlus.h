#pragma once
#include "Graphics/RHI/RHI.h"
#include "Graphics/Renderer/RenderTypes.h"

class GFSDK_SSAO_Context_D3D12;

namespace Relentless
{
	struct HBAOPlusContextData;

	class HBAOPlus
	{
	public:
		HBAOPlus(GraphicsDevice* aGraphicsDevice) noexcept;

		void Render(CommandContext& aCommandContext, const RenderView& aRenderView, SceneTextures& aSceneTextures) noexcept;
	private:
		std::vector<DescriptorHandle> m_ShaderBindableHandles;
		std::vector<DescriptorHandle> m_RTVHandles;

		GraphicsDevice* m_pDevice = nullptr;
		GFSDK_SSAO_Context_D3D12* m_pSSAOContext = nullptr;
		bool m_FirstFrameDone = false;
	};
}