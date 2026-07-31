#include "ExponentialHeightFogObserverSystem.h"
#include "ECS/Components/ExponentialHeightFogComponent.h"

#include "Graphics/Renderer/Renderer.h"

#include "Scene/Scene.h"
#include "Subsystem/CoreTypes/FogRenderSubsystem.h"

namespace Relentless
{
	void ExponentialHeightFogObserverSystem::Register(Scene& aScene) noexcept
	{
		EntityManager& entityManager = aScene.GetEntityManager();

		entityManager.OnRemove<ExponentialHeightFogComponent>().Connect([this, uid = aScene.GetUUID()](MAYBE_UNUSED EntityManager& aEntityManager, entity aEntity) { OnExponentialHeightFogComponentRemoved(aEntity, uid); });
		entityManager.OnCreated<HiddenInGameComponent>().Connect(this, &ExponentialHeightFogObserverSystem::OnEntityVisibilityChanged);
		entityManager.OnRemove<HiddenInGameComponent>().Connect(this, &ExponentialHeightFogObserverSystem::OnEntityVisibilityChanged);
		aScene.OnExponentialHeightFogChange.Connect(this, &ExponentialHeightFogObserverSystem::OnActiveExponentialHeightFogChange);
	}

	void ExponentialHeightFogObserverSystem::OnActiveExponentialHeightFogChange(Scene& aScene, entity aCurrentExponentialHeightFog, entity aNewExponentialHeightFog) noexcept
	{
		if (aCurrentExponentialHeightFog != NULL_ENTITY)
			aScene.GetEntityManager().AddOrReplace<ExponentialHeightFogComponent::DirtyRenderState>(aCurrentExponentialHeightFog);
		if (aNewExponentialHeightFog != NULL_ENTITY)
			aScene.GetEntityManager().AddOrReplace<ExponentialHeightFogComponent::DirtyRenderState>(aNewExponentialHeightFog);
	}

	void ExponentialHeightFogObserverSystem::OnEntityVisibilityChanged(EntityManager& aEntityManager, entity aEntity) noexcept
	{
		if (aEntityManager.Has<ExponentialHeightFogComponent>(aEntity))
			aEntityManager.AddOrReplace<ExponentialHeightFogComponent::DirtyRenderState>(aEntity);
	}

	void ExponentialHeightFogObserverSystem::OnExponentialHeightFogComponentRemoved(entity aEntity, const UUID& aUUID) noexcept
	{
		Renderer::Dispatch([aEntity, aUUID](Renderer* aRenderer)
			{
				RenderScene* pRenderScene = aRenderer->GetRenderScene(aUUID);
				FogRenderSubsystem* pFogRenderSubsystem = pRenderScene->GetSubsystem<FogRenderSubsystem>();
				pFogRenderSubsystem->Remove({ aEntity });
			});
	}
}