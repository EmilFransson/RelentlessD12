#include "PostProcessRenderDispatchSystem.h"

#include "ECS/Components/PostProcessVolumeComponent.h"

#include "Graphics/Renderer/Renderer.h"

#include "Scene/Scene.h"
#include "Subsystem/CoreTypes/PostProcessRenderSubsystem.h"

namespace Relentless
{
	void PostProcessRenderDispatchSystem::Execute(SceneState& aSceneState) noexcept
	{
		PROFILE_FUNC;

		auto postProcessVolumeDirtyCollection = aSceneState.EntityManager.Collect<PostProcessVolumeComponent, PostProcessVolumeComponent::DirtyRenderState>();
		const uint32 size = postProcessVolumeDirtyCollection.Size();
		if (size == 0u)
			return;

		std::vector<PostProcessRenderProxy> postProcessRenderProxies;
		postProcessRenderProxies.reserve(size);

		postProcessVolumeDirtyCollection.Do([&postProcessRenderProxies, &aSceneState](entity aEntity, PostProcessVolumeComponent& aPostProcessVolumeComponent)
			{
				const ExposureSettings& exposureSettings = aPostProcessVolumeComponent.GetExposure();
				const AmbientOcclusionSettings& ambientOcclusionSettings = aPostProcessVolumeComponent.GetAmbientOcclusion();

				PostProcessRenderProxy& renderProxy = postProcessRenderProxies.emplace_back();
				renderProxy.ID = aEntity;

				//Ambient Occlusion:
				renderProxy.AmbientOcclusionProxySettings.IsEnabled = ambientOcclusionSettings.IsEnabled();
				renderProxy.AmbientOcclusionProxySettings.Bias = ambientOcclusionSettings.GetBias();
				renderProxy.AmbientOcclusionProxySettings.BlurEnabled = ambientOcclusionSettings.IsBlurEnabled();
				renderProxy.AmbientOcclusionProxySettings.BlurRadius = static_cast<uint8>(ambientOcclusionSettings.GetBlurRadius());
				renderProxy.AmbientOcclusionProxySettings.BlurSharpness = ambientOcclusionSettings.GetBlurSharpness();
				renderProxy.AmbientOcclusionProxySettings.DepthPrecision = static_cast<uint8>(ambientOcclusionSettings.GetDepthPrecision());
				renderProxy.AmbientOcclusionProxySettings.PowerExponent = ambientOcclusionSettings.GetPowerExponent();
				renderProxy.AmbientOcclusionProxySettings.Radius = ambientOcclusionSettings.GetRadius();
				renderProxy.AmbientOcclusionProxySettings.StepCount = static_cast<uint8>(ambientOcclusionSettings.GetStepCount());

				//Exposure:
				renderProxy.ExposureRenderProxySettings.Compensation = exposureSettings.GetCompensation();
				renderProxy.ExposureRenderProxySettings.MinEV100 = exposureSettings.GetMinEV100();
				renderProxy.ExposureRenderProxySettings.MaxEV100 = exposureSettings.GetMaxEV100();
				renderProxy.ExposureRenderProxySettings.SpeedUp = exposureSettings.GetSpeedUp();
				renderProxy.ExposureRenderProxySettings.SpeedDown = exposureSettings.GetSpeedDown();
				renderProxy.ExposureRenderProxySettings.LowPercent = exposureSettings.GetLowPercent();
				renderProxy.ExposureRenderProxySettings.HighPercent = exposureSettings.GetHighPercent();
				renderProxy.ExposureRenderProxySettings.HistogramMinEV100 = exposureSettings.GetHistogramMinEV100();
				renderProxy.ExposureRenderProxySettings.HistogramMaxEV100 = exposureSettings.GetHistogramMaxEV100();

				aSceneState.EntityManager.Remove<PostProcessVolumeComponent::DirtyRenderState>(aEntity);
			});

		Renderer::Dispatch([renderProxies = std::move(postProcessRenderProxies), sceneUUID = aSceneState.Scene.GetUUID()](Renderer* aRenderer) 
			{
				RenderScene* pRenderScene = aRenderer->GetRenderScene(sceneUUID);
				PostProcessRenderSubsystem* pPostProcessRenderSubsystem = pRenderScene->GetSubsystem<PostProcessRenderSubsystem>();
				pPostProcessRenderSubsystem->Patch(std::move(renderProxies));
			});
	}
}