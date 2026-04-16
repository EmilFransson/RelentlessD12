// #pragma once
// #include "Graphics/Renderer/RenderPass.h"
// 
// namespace Relentless
// {
// 	struct SkyLightComponent;
// 
// 	class SkyLightRenderer
// 	{
// 	public:
// 		explicit SkyLightRenderer(GraphicsDevice* aGraphicsDevice) noexcept;
// 		void Render(const RenderView& aRenderView) noexcept;
// 	private:
// 		void ExecuteIrradianceConvolutionPass(CommandContext& aCommandContext, const RenderView& aRenderView, SkyLightComponent& aSkyLightComponent) noexcept;
// 		void ExecuteRadiancePrefilterPass(CommandContext& aCommandContext, const RenderView& aRenderView, SkyLightComponent& aSkyLightComponent) noexcept;
// 
// 		NO_DISCARD bool ShouldProcessSkyLight(SkyLightComponent& aSkyLightComponent, uint32 aCurrentFrameIndex) noexcept;
// 	private:
// 		GraphicsDevice* m_pDevice = nullptr;
// 
// 		Ref<PipelineState> m_pIrradiancePSO = nullptr;
// 		Ref<PipelineState> m_pIrradianceLowerHemisphereColorPSO = nullptr;
// 		Ref<PipelineState> m_pRadiancePSO = nullptr;
// 		Ref<PipelineState> m_pRadianceLowerHemisphereColorPSO = nullptr;
// 		Ref<PipelineState> m_pCubemapResamplePSO = nullptr;
// 
// 		uint32 m_LastCapturedFrame = 0u;
// 	};
// }