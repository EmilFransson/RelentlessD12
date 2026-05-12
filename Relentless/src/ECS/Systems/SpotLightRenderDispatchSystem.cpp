#include "SpotLightRenderDispatchSystem.h"

#include "ECS/Components/LightComponent.h"
#include "ECS/Components/TransformComponent.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RenderProxy/LightRenderProxy.h"

#include "Scene/Scene.h"
#include "Subsystem/CoreTypes/LightRenderSubsystem.h"

namespace Relentless
{
	void SpotLightRenderDispatchSystem::Execute(SceneState& aSceneState) noexcept
	{
		auto spotLightDirtyCollection = aSceneState.EntityManager.Collect<SpotLightComponent, LightBaseComponent<SpotLightComponent>::DirtyRenderState, TransformComponent>();
		auto transformDirtyCollection = aSceneState.EntityManager.Collect<SpotLightComponent, TransformComponent::DirtyRenderState, TransformComponent>();
		const uint32 spotLightDirtyCollectionSize = spotLightDirtyCollection.Size();
		const uint32 transformDirtyCollectionSize = transformDirtyCollection.Size();

		if (spotLightDirtyCollectionSize == 0u && transformDirtyCollectionSize == 0u)
			return;

		std::unordered_set<entity> dirtyEntities;
		dirtyEntities.reserve(spotLightDirtyCollectionSize + transformDirtyCollectionSize);

		spotLightDirtyCollection.Do([&dirtyEntities](entity aEntity) { dirtyEntities.insert(aEntity); });
		transformDirtyCollection.Do([&dirtyEntities](entity aEntity) { dirtyEntities.insert(aEntity); });

		std::vector<LightRenderProxy> lightRenderProxies;
		lightRenderProxies.reserve(dirtyEntities.size());

		for (entity aEntity : dirtyEntities)
		{
			const auto& [spotLightComponent, transformComponent] = aSceneState.EntityManager.Get<SpotLightComponent, TransformComponent>(aEntity);

			const Color& color = spotLightComponent.GetColor();

			LightRenderProxy& renderProxy = lightRenderProxies.emplace_back();
			renderProxy.Color = Vector3(color.R(), color.G(), color.B());
			renderProxy.Position = transformComponent.GetWorldLocation();
			renderProxy.Direction = transformComponent.GetWorldForward();
			renderProxy.AttenuationRadius = spotLightComponent.GetAttenuationRadius() > 0.0f ? (1.0f / (spotLightComponent.GetAttenuationRadius() * spotLightComponent.GetAttenuationRadius())) : 0.0f;
			renderProxy.Intensity = spotLightComponent.GetIntensity();
			renderProxy.InnerConeAngle = Math::Cos(spotLightComponent.GetInnerConeAngleRadians() * 0.5f);
			renderProxy.OuterConeAngle = Math::Cos(spotLightComponent.GetOuterConeAngleRadians() * 0.5f);
			renderProxy.IsEnabled = renderProxy.Intensity > 0.0f && aSceneState.Scene.IsEntityVisible(aEntity);
			renderProxy.LightType = ELightType::Spot;
			renderProxy.ID = aEntity;

			if (spotLightComponent.IsUsingTemperature())
			{
				const Color tempColor = Math::MakeFromColorTemperature(spotLightComponent.GetTemperature());
				const Vector3 vTempColor = Vector3(tempColor.x, tempColor.y, tempColor.z);
				renderProxy.Color *= vTempColor;
			}

			aSceneState.EntityManager.RemoveIfExists<LightBaseComponent<SpotLightComponent>::DirtyRenderState>(aEntity);
		}

		Renderer::Dispatch([renderProxies = std::move(lightRenderProxies), uid = aSceneState.Scene.GetUUID()](Renderer* aRenderer)
			{
				RenderScene* pRenderScene = aRenderer->GetRenderScene(uid);
				LightRenderSubsystem* pLightRenderSubsystem = pRenderScene->GetSubsystem<LightRenderSubsystem>();
				pLightRenderSubsystem->Patch(std::move(renderProxies));
			});
	}
}