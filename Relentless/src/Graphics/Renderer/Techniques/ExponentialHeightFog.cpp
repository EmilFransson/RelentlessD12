#include "ExponentialHeightFog.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"

#include "Subsystem/CoreTypes/FogRenderSubsystem.h"

namespace Relentless
{
	ExponentialHeightFog::ExponentialHeightFog(GraphicsDevice* aDevice) noexcept
		: m_pDevice{ aDevice }
	{
	}

	void ExponentialHeightFog::Render(CommandContext& aCommandContext, const RenderView& aRenderView, SceneTextures& aSceneTextures) noexcept
	{
		FogRenderSubsystem* pFogRenderSubsystem = aRenderView.pRenderScene->GetSubsystem<FogRenderSubsystem>();

		aCommandContext.InsertResourceBarrier(aSceneTextures.pDepthTarget, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		aCommandContext.InsertResourceBarrier(aSceneTextures.pHDRColorTarget, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		aCommandContext.SetComputeRootSignature(m_pDevice->GetGlobalRootSignature());

		if (pFogRenderSubsystem->IsInscatteringTextureUsed())
			aCommandContext.SetPipelineState(m_pDevice->GetOrCreateComputePipeline(m_pDevice->GetGlobalRootSignature(), "ExponentialHeightFogComputeShader", "cs_main", { "USE_INSCATTERING_TEXTURE" }));
		else
			aCommandContext.SetPipelineState(m_pDevice->GetOrCreateComputePipeline(m_pDevice->GetGlobalRootSignature(), "ExponentialHeightFogComputeShader", "cs_main"));

		const uint32 width = aSceneTextures.pHDRColorTarget->GetWidth();
		const uint32 height = aSceneTextures.pHDRColorTarget->GetHeight();

		struct  
		{
			uint32 DepthIndex;
			uint32 ColorIndex;
			float Padding[2];
		} parameters;

		parameters.DepthIndex = aSceneTextures.pDepthTarget->GetSRVIndex();
		parameters.ColorIndex = aSceneTextures.pHDRColorTarget->GetUAVIndex();
		aCommandContext.BindRootCBV(BindingSlot::PerPass, &parameters, sizeof(parameters));
		Renderer::BindViewData(aCommandContext, aRenderView);

		aCommandContext.Dispatch(ComputeUtils::GetNumThreadGroups(width, 16, height, 16));
	}
}