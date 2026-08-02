#include "ExponentialHeightFogRenderDispatchSystem.h"

#include "Assets/CoreTypes/TextureCube.h"

#include "ECS/Components/ExponentialHeightFogComponent.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RenderProxy/ExponentialHeightFogRenderProxy.h"

#include "Scene/Scene.h"
#include "Subsystem/CoreTypes/FogRenderSubsystem.h"
namespace Relentless
{
	void ExponentialHeightFogRenderDispatchSystem::Execute(SceneState& aSceneState) noexcept
	{
		auto dirtyFogCollection = aSceneState.EntityManager.Collect<ExponentialHeightFogComponent::DirtyRenderState>();
		const uint32 numDirtyComponents = dirtyFogCollection.Size();
		if (numDirtyComponents == 0u)
			return;

		std::vector<ExponentialHeightFogRenderProxy> renderProxies;
		renderProxies.reserve(numDirtyComponents);

		const entity currentFogEntity = aSceneState.Scene.GetActiveExponentialHeightFog();
		dirtyFogCollection.Do([&aSceneState, &renderProxies, currentFogEntity](entity aEntity)
			{
				const ExponentialHeightFogComponent& exponentialHeightFogComponent = aSceneState.EntityManager.Get<ExponentialHeightFogComponent>(aEntity);
				const ExponentialHeightFogComponent::FogLayer& fogLayer0 = exponentialHeightFogComponent.GetFogLayer(0u);
				const ExponentialHeightFogComponent::FogLayer& fogLayer1 = exponentialHeightFogComponent.GetFogLayer(1u);

				ExponentialHeightFogRenderProxy& renderProxy = renderProxies.emplace_back();
				renderProxy.ID = aEntity;
				renderProxy.IsActive = currentFogEntity == aEntity && aSceneState.Scene.IsEntityVisible(aEntity);;
				renderProxy.InScatteringColor = exponentialHeightFogComponent.GetInscatteringColor().ToVector3();
				renderProxy.InScatteringTextureColorTint = exponentialHeightFogComponent.GetInscatterTextureTintColor().ToVector3();
				renderProxy.MaxOpacity = exponentialHeightFogComponent.GetMaxOpacity();
				renderProxy.UseUniformInscatter = exponentialHeightFogComponent.GetInscatterMode() == EFogInscatterMode::Uniform;
				renderProxy.DensityLayer0 = fogLayer0.Density;
				renderProxy.DensityLayer1 = fogLayer1.Density;
				renderProxy.HeightFallOffLayer0 = fogLayer0.HeightFalloff;
				renderProxy.HeightFallOffLayer1 = fogLayer1.HeightFalloff;
				renderProxy.StartDistanceLayer0 = fogLayer0.StartDistance;
				renderProxy.StartDistanceLayer1 = fogLayer1.StartDistance;
				renderProxy.EndDistanceLayer0 = fogLayer0.EndDistance;
				renderProxy.EndDistanceLayer1 = fogLayer1.EndDistance;
				renderProxy.HeightOffsetLayer0 = fogLayer0.HeightOffset;
				renderProxy.HeightOffsetLayer1 = fogLayer1.HeightOffset;
				renderProxy.InScatterTexture = exponentialHeightFogComponent.HasAssignedInScatterTexture() ? exponentialHeightFogComponent.GetInscatterTexture()->GetResource() : nullptr;
				renderProxy.FullyDirectionalDistance = exponentialHeightFogComponent.GetFullyDirectionalInScatteringColorDistance();
				renderProxy.NonDirectionalDistance = exponentialHeightFogComponent.GetNonDirectionalInScatteringColorDistance();
				renderProxy.InscatteringColorIntensity = exponentialHeightFogComponent.GetInscatteringColorIntensity();

				aSceneState.EntityManager.Remove<ExponentialHeightFogComponent::DirtyRenderState>(aEntity);
			});

		Renderer::Dispatch([uuid = aSceneState.Scene.GetUUID(), proxies = std::move(renderProxies)](Renderer* aRenderer)
			{
				RenderScene* pRenderScene = aRenderer->GetRenderScene(uuid);
				FogRenderSubsystem* pFogRenderSubsystem = pRenderScene->GetSubsystem<FogRenderSubsystem>();
				pFogRenderSubsystem->Patch(std::move(proxies));
			});
	}
}