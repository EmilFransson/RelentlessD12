#include "SkyLightRenderDispatchSystem.h"

#include "Assets/CoreTypes/TextureCube.h"

#include "ECS/Components/TransformComponent.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RenderProxy/SkyLightRenderProxy.h"

#include "Scene/Scene.h"
#include "Subsystem/CoreTypes/SkyLightRenderSubsystem.h"

namespace Relentless
{
	void SkyLightRenderDispatchSystem::Execute(SceneState& aSceneState) noexcept
	{
		auto skyLightDirtyCollection = aSceneState.EntityManager.Collect<SkyLightComponent, SkyLightComponent::DirtyRenderState, TransformComponent>();
		auto transformDirtyCollection = aSceneState.EntityManager.Collect<SkyLightComponent, TransformComponent::DirtyRenderState, TransformComponent>();
		const uint32 skyLightDirtyCollectionSize = skyLightDirtyCollection.Size();
		const uint32 transformDirtyCollectionSize = transformDirtyCollection.Size();

		if (skyLightDirtyCollectionSize == 0u && transformDirtyCollectionSize == 0u)
			return;

		std::unordered_set<entity> dirtyEntities;
		dirtyEntities.reserve(skyLightDirtyCollectionSize + transformDirtyCollectionSize);

		skyLightDirtyCollection.Do([&dirtyEntities](entity aEntity) { dirtyEntities.insert(aEntity); });
		transformDirtyCollection.Do([&dirtyEntities](entity aEntity) { dirtyEntities.insert(aEntity); });

		std::vector<SkyLightRenderProxy> skyLightRenderProxies;
		skyLightRenderProxies.reserve(dirtyEntities.size());

		for (entity aEntity : dirtyEntities)
		{
			const auto& [skyLightComponent, transformComponent] = aSceneState.EntityManager.Get<SkyLightComponent, TransformComponent>(aEntity);

			const bool hasValidPrimaryEnvironment = skyLightComponent.HasAssignedPrimaryEnvironment() && skyLightComponent.GetPrimaryEnvironment()->HasValidEnvironmentMap();
			const bool hasValidBlendEnvironment = skyLightComponent.HasAssignedBlendEnvironment() && skyLightComponent.GetBlendEnvironment()->HasValidEnvironmentMap();

			if ((!hasValidPrimaryEnvironment && !hasValidBlendEnvironment) || !aSceneState.Scene.IsEntityVisible(aEntity))
			{
				SkyLightRenderProxy& deactivationProxy = skyLightRenderProxies.emplace_back();
				deactivationProxy.ID = aEntity;
				deactivationProxy.IsActive = false;

				if (aSceneState.EntityManager.Has<SkyLightComponent::DirtyRenderState>(aEntity))
					aSceneState.EntityManager.Remove<SkyLightComponent::DirtyRenderState>(aEntity);

				return;
			}

			SkyLightRenderProxy& renderProxy = skyLightRenderProxies.emplace_back();
			renderProxy.ID = aEntity;
			renderProxy.IsActive = aSceneState.Scene.GetActiveSkyLight() == aEntity;
			renderProxy.RadianceMapSize = skyLightComponent.GetRadianceMapSize();
			renderProxy.RealtimeMipsPerFrame = skyLightComponent.GetRealtimeMipsPerFrame();
			renderProxy.CaptureMode = skyLightComponent.GetCaptureMode();
			renderProxy.LowerHemisphereMode = skyLightComponent.GetLowerHemisphereMode();
			renderProxy.WorldRotation = transformComponent.GetWorldRotation();
			renderProxy.Intensity = skyLightComponent.GetIntensity();
			renderProxy.BlendFactor = skyLightComponent.GetBlendFactor();
			renderProxy.LowerHemisphereColor = skyLightComponent.GetLowerHemisphereColor();
			renderProxy.TintColor = skyLightComponent.GetTintColor();
			
			if (hasValidPrimaryEnvironment)
			{
				const Ref<Environment> pBackingPrimaryEnvironment = skyLightComponent.GetPrimaryEnvironment();
				renderProxy.PrimaryEnvironmentMap = pBackingPrimaryEnvironment->HasValidEnvironmentMap() ? pBackingPrimaryEnvironment->GetEnvironmentMap()->GetResource() : nullptr;
				renderProxy.PrimaryEnvironmentSourceType = pBackingPrimaryEnvironment->GetSourceType();
				renderProxy.PrimaryEnvironmentColor = pBackingPrimaryEnvironment->GetSolidColor();

				if (renderProxy.BlendFactor < 1.0f || !hasValidBlendEnvironment)
					renderProxy.Intensity *= pBackingPrimaryEnvironment->GetIntensity();
			}
			if (hasValidBlendEnvironment)
			{
				const Ref<Environment> pBackingBlendEnvironment = skyLightComponent.GetBlendEnvironment();
				renderProxy.BlendEnvironmentMap = pBackingBlendEnvironment->HasValidEnvironmentMap() ? pBackingBlendEnvironment->GetEnvironmentMap()->GetResource() : nullptr;
				renderProxy.BlendEnvironmentSourceType = pBackingBlendEnvironment->GetSourceType();
				renderProxy.BlendEnvironmentColor = pBackingBlendEnvironment->GetSolidColor();

				if (renderProxy.BlendFactor > 0.0f || !hasValidPrimaryEnvironment)
					renderProxy.Intensity *= pBackingBlendEnvironment->GetIntensity();
			}

			aSceneState.EntityManager.RemoveIfExists<SkyLightComponent::DirtyRenderState>(aEntity);
		}

		Renderer::Dispatch([renderProxies = std::move(skyLightRenderProxies), uid = aSceneState.Scene.GetUUID()](Renderer* aRenderer)
			{
				RenderScene* pRenderScene = aRenderer->GetRenderScene(uid);
				SkyLightRenderSubsystem* pSkyLightRenderSubsystem = pRenderScene->GetSubsystem<SkyLightRenderSubsystem>();
				pSkyLightRenderSubsystem->Patch(std::move(renderProxies));
			});
	}
}
