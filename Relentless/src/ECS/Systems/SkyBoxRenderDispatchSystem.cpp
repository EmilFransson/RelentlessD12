#include "SkyBoxRenderDispatchSystem.h"

#include "Assets/CoreTypes/TextureCube.h"

#include "ECS/Components/TransformComponent.h"
#include "ECS/Components/SkyBoxComponent.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RenderProxy/SkyBoxRenderProxy.h"

#include "Scene/Scene.h"
#include "Subsystem/CoreTypes/SkyBoxRenderSubsystem.h"

namespace Relentless
{
	void SkyBoxRenderDispatchSystem::Execute(SceneState& aSceneState) noexcept
	{
		auto skyBoxDirtyCollection = aSceneState.EntityManager.Collect<SkyBoxComponent, SkyBoxComponent::DirtyRenderState, TransformComponent>();
		auto transformDirtyCollection = aSceneState.EntityManager.Collect<SkyBoxComponent, TransformComponent::DirtyRenderState, TransformComponent>();
		const uint32 skyBoxDirtyCollectionSize = skyBoxDirtyCollection.Size();
		const uint32 transformDirtyCollectionSize = transformDirtyCollection.Size();

		if (skyBoxDirtyCollectionSize == 0u && transformDirtyCollectionSize == 0u)
			return;

		std::unordered_set<entity> dirtyEntities;
		dirtyEntities.reserve(skyBoxDirtyCollectionSize + transformDirtyCollectionSize);

		skyBoxDirtyCollection.Do([&dirtyEntities](entity aEntity) { dirtyEntities.insert(aEntity); });
		transformDirtyCollection.Do([&dirtyEntities](entity aEntity) { dirtyEntities.insert(aEntity); });

		std::vector<SkyBoxRenderProxy> skyBoxRenderProxies;
		skyBoxRenderProxies.reserve(dirtyEntities.size());

		for (entity aEntity : dirtyEntities)
		{
			const auto& [skyBoxComponent, transformComponent] = aSceneState.EntityManager.Get<SkyBoxComponent, TransformComponent>(aEntity);

			const bool hasValidPrimaryEnvironment = skyBoxComponent.HasAssignedPrimaryEnvironment();
			const bool hasValidBlendEnvironment = skyBoxComponent.HasAssignedBlendEnvironment();

			SkyBoxRenderProxy& renderProxy = skyBoxRenderProxies.emplace_back();
			renderProxy.WorldRotation = transformComponent.GetWorldRotation();
			renderProxy.TintColor = skyBoxComponent.GetTintColor();
			renderProxy.ID = aEntity;
			renderProxy.Intensity = skyBoxComponent.GetIntensity();
			renderProxy.LodBias = skyBoxComponent.GetLODBias();
			renderProxy.BlendFactor = skyBoxComponent.GetBlendFactor();
			renderProxy.IsActive = aSceneState.Scene.GetActiveSkyBox() == aEntity && (hasValidPrimaryEnvironment || hasValidBlendEnvironment) && aSceneState.Scene.IsEntityVisible(aEntity);

			if (hasValidPrimaryEnvironment)
			{
				Ref<Environment> pPrimaryEnvironment = skyBoxComponent.GetPrimaryEnvironment();
				renderProxy.EnvironmentMapA = pPrimaryEnvironment->HasValidEnvironmentMap() ? pPrimaryEnvironment->GetEnvironmentMap()->GetResource() : nullptr;
				renderProxy.EnvironmentASolidColor = pPrimaryEnvironment->GetSolidColor();
				renderProxy.EnvironmentASourceType = pPrimaryEnvironment->GetSourceType();

				if (renderProxy.BlendFactor < 1.0f || !hasValidBlendEnvironment)
					renderProxy.Intensity *= pPrimaryEnvironment->GetIntensity();
			}
			if (hasValidBlendEnvironment)
			{
				Ref<Environment> pBlendEnvironment = skyBoxComponent.GetBlendEnvironment();
				renderProxy.EnvironmentMapB = pBlendEnvironment->HasValidEnvironmentMap() ? pBlendEnvironment->GetEnvironmentMap()->GetResource() : nullptr;
				renderProxy.EnvironmentBSolidColor = pBlendEnvironment->GetSolidColor();
				renderProxy.EnvironmentBSourceType = pBlendEnvironment->GetSourceType();

				if (renderProxy.BlendFactor > 0.0f || !hasValidPrimaryEnvironment)
					renderProxy.Intensity *= pBlendEnvironment->GetIntensity();
			}

			aSceneState.EntityManager.RemoveIfExists<SkyBoxComponent::DirtyRenderState>(aEntity);
		}

		Renderer::Dispatch([renderProxies = std::move(skyBoxRenderProxies), uid = aSceneState.Scene.GetUUID()](Renderer* aRenderer)
			{
				RenderScene* pRenderScene = aRenderer->GetRenderScene(uid);
				SkyBoxRenderSubsystem* pSkyBoxRenderSubsystem = pRenderScene->GetSubsystem<SkyBoxRenderSubsystem>();
				pSkyBoxRenderSubsystem->Patch(std::move(renderProxies));
			});
	}
}
