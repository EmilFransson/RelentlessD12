#include "SkyBoxRenderDispatchSystem.h"

#include "Assets/CoreTypes/TextureCube.h"

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

			const bool hasValidPrimaryEnvironment = skyBoxComponent.HasAssignedPrimaryEnvironment() && skyBoxComponent.GetPrimaryEnvironment()->HasValidEnvironmentMap();
			const bool hasValidBlendEnvironment = skyBoxComponent.HasAssignedBlendEnvironment() && skyBoxComponent.GetBlendEnvironment()->HasValidEnvironmentMap();

			SkyBoxRenderProxy& renderProxy = skyBoxRenderProxies.emplace_back();
			renderProxy.EnvironmentMapA = hasValidPrimaryEnvironment ? skyBoxComponent.GetPrimaryEnvironment()->GetEnvironmentMap()->GetResource() : nullptr;
			renderProxy.EnvironmentMapB = hasValidBlendEnvironment ? skyBoxComponent.GetBlendEnvironment()->GetEnvironmentMap()->GetResource() : nullptr;
			renderProxy.WorldRotation = transformComponent.GetWorldRotation();
			renderProxy.TintColor = skyBoxComponent.GetTintColor();
			renderProxy.ID = aEntity;
			renderProxy.Intensity = skyBoxComponent.GetIntensity();
			renderProxy.LodBias = skyBoxComponent.GetLODBias();
			renderProxy.BlendFactor = skyBoxComponent.GetBlendFactor();
			renderProxy.IsActive = aSceneState.Scene.GetActiveSkyBox() == aEntity && (hasValidPrimaryEnvironment || hasValidBlendEnvironment) && aSceneState.Scene.IsEntityVisible(aEntity);

			if (aSceneState.EntityManager.Has<SkyBoxComponent::DirtyRenderState>(aEntity))
				aSceneState.EntityManager.Remove<SkyBoxComponent::DirtyRenderState>(aEntity);
		}

		Renderer::Dispatch([renderProxies = std::move(skyBoxRenderProxies)](Renderer* aRenderer)
			{
				SkyBoxRenderSubsystem* pSkyBoxRenderSubsystem = aRenderer->GetSubsystem<SkyBoxRenderSubsystem>();
				pSkyBoxRenderSubsystem->Patch(std::move(renderProxies));
			});
	}
}
