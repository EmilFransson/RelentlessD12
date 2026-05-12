#include "PointLightRenderDispatchSystem.h"

#include "ECS/Components/LightComponent.h"
#include "ECS/Components/TransformComponent.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RenderProxy/LightRenderProxy.h"

#include "Scene/Scene.h"
#include "Subsystem/CoreTypes/LightRenderSubsystem.h"

namespace Relentless
{
	void PointLightRenderDispatchSystem::Execute(SceneState& aSceneState) noexcept
	{
		auto pointLightDirtyCollection = aSceneState.EntityManager.Collect<PointLightComponent, LightBaseComponent<PointLightComponent>::DirtyRenderState, TransformComponent>();
		auto transformDirtyCollection = aSceneState.EntityManager.Collect<PointLightComponent, TransformComponent::DirtyRenderState, TransformComponent>();
		const uint32 pointLightDirtyCollectionSize = pointLightDirtyCollection.Size();
		const uint32 transformDirtyCollectionSize = transformDirtyCollection.Size();

		if (pointLightDirtyCollectionSize == 0u && transformDirtyCollectionSize == 0u)
			return;

		std::unordered_set<entity> dirtyEntities;
		dirtyEntities.reserve(pointLightDirtyCollectionSize + transformDirtyCollectionSize);

		pointLightDirtyCollection.Do([&dirtyEntities](entity aEntity) { dirtyEntities.insert(aEntity); });
		transformDirtyCollection.Do([&dirtyEntities](entity aEntity) { dirtyEntities.insert(aEntity); });

		std::vector<LightRenderProxy> lightRenderProxies;
		lightRenderProxies.reserve(dirtyEntities.size());

		for (entity aEntity : dirtyEntities)
		{
			const auto& [pointLightComponent, transformComponent] = aSceneState.EntityManager.Get<PointLightComponent, TransformComponent>(aEntity);

			const Color& color = pointLightComponent.GetColor();

			LightRenderProxy& renderProxy = lightRenderProxies.emplace_back();
			renderProxy.Color = Vector3(color.R(), color.G(), color.B());
			renderProxy.Position = transformComponent.GetWorldLocation();
			renderProxy.AttenuationRadius = pointLightComponent.GetAttenuationRadius() > 0.0f ? (1.0f / (pointLightComponent.GetAttenuationRadius() * pointLightComponent.GetAttenuationRadius())) : 0.0f;
			renderProxy.Intensity = pointLightComponent.GetIntensity();
			renderProxy.IsEnabled = renderProxy.Intensity > 0.0f && aSceneState.Scene.IsEntityVisible(aEntity);
			renderProxy.LightType = ELightType::Point;
			renderProxy.ID = aEntity;

			if (pointLightComponent.IsUsingTemperature())
			{
				const Color tempColor = Math::MakeFromColorTemperature(pointLightComponent.GetTemperature());
				const Vector3 vTempColor = Vector3(tempColor.x, tempColor.y, tempColor.z);
				renderProxy.Color *= vTempColor;
			}

			aSceneState.EntityManager.RemoveIfExists<LightBaseComponent<PointLightComponent>::DirtyRenderState>(aEntity);
		}

		Renderer::Dispatch([renderProxies = std::move(lightRenderProxies), uid = aSceneState.Scene.GetUUID()](Renderer* aRenderer)
			{
				RenderScene* pRenderScene = aRenderer->GetRenderScene(uid);
				LightRenderSubsystem* pLightRenderSubsystem = pRenderScene->GetSubsystem<LightRenderSubsystem>();
				pLightRenderSubsystem->Patch(std::move(renderProxies));
			});
	}
}