#include "DirectionalLightRenderDispatchSystem.h"

#include "ECS/Components/LightComponent.h"
#include "ECS/Components/TransformComponent.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RenderProxy/LightRenderProxy.h"

#include "Scene/Scene.h"
#include "Subsystem/CoreTypes/LightRenderSubsystem.h"

namespace Relentless
{
	void DirectionalLightRenderDispatchSystem::Execute(SceneState& aSceneState) noexcept
	{
		auto directionalLightDirtyCollection = aSceneState.EntityManager.Collect<DirectionalLightComponent, LightBaseComponent<DirectionalLightComponent>::DirtyRenderState, TransformComponent>();
		auto transformDirtyCollection = aSceneState.EntityManager.Collect<DirectionalLightComponent, TransformComponent::DirtyRenderState, TransformComponent>();
		const uint32 directionalLightDirtyCollectionSize = directionalLightDirtyCollection.Size();
		const uint32 transformDirtyCollectionSize = transformDirtyCollection.Size();
		
		if (directionalLightDirtyCollectionSize == 0u && transformDirtyCollectionSize == 0u)
			return;

		std::unordered_set<entity> dirtyEntities;
		dirtyEntities.reserve(directionalLightDirtyCollectionSize + transformDirtyCollectionSize);

		directionalLightDirtyCollection.Do([&dirtyEntities](entity aEntity) { dirtyEntities.insert(aEntity); });
		transformDirtyCollection.Do([&dirtyEntities](entity aEntity) { dirtyEntities.insert(aEntity); });

		std::vector<LightRenderProxy> lightRenderProxies;
		lightRenderProxies.reserve(dirtyEntities.size());

		for (entity aEntity : dirtyEntities)
		{
			const auto& [directionalLightComponent, transformComponent] = aSceneState.EntityManager.Get<DirectionalLightComponent, TransformComponent>(aEntity);

			const Color& color = directionalLightComponent.GetColor();

			LightRenderProxy& renderProxy = lightRenderProxies.emplace_back();
			renderProxy.Color = Vector3(color.R(), color.G(), color.B());
			renderProxy.Direction = transformComponent.GetWorldForward();
			renderProxy.Intensity = directionalLightComponent.GetIntensity();
			renderProxy.IsEnabled = renderProxy.Intensity > 0.0f && aSceneState.Scene.IsEntityVisible(aEntity);
			renderProxy.LightType = ELightType::Directional;
			renderProxy.ID = aEntity;

			if (directionalLightComponent.IsUsingTemperature())
			{
				const Color tempColor = Math::MakeFromColorTemperature(directionalLightComponent.GetTemperature());
				const Vector3 vTempColor = Vector3(tempColor.x, tempColor.y, tempColor.z);
				renderProxy.Color *= vTempColor;
			}

			if (aSceneState.EntityManager.Has<LightBaseComponent<DirectionalLightComponent>::DirtyRenderState>(aEntity))
				aSceneState.EntityManager.Remove<LightBaseComponent<DirectionalLightComponent>::DirtyRenderState>(aEntity);
		}

		Renderer::Dispatch([renderProxies = std::move(lightRenderProxies), uid = aSceneState.Scene.GetUUID()](Renderer* aRenderer)
			{
				RenderScene* pRenderScene = aRenderer->GetRenderScene(uid);
				LightRenderSubsystem* pLightRenderSubsystem = pRenderScene->GetSubsystem<LightRenderSubsystem>();
				pLightRenderSubsystem->Patch(std::move(renderProxies));
			});
	}
}