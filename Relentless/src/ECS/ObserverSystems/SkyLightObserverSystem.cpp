#include "SkyLightObserverSystem.h"
#include "Scene/Scene.h"

#include "ECS/Components/SkyLightComponent.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/Scene/RenderScene.h"

#include "Subsystem/CoreTypes/SkyLightRenderSubsystem.h"

namespace Relentless
{
	void SkyLightObserverSystem::Register(Scene& aScene) noexcept
	{
		EntityManager& entityManager = aScene.GetEntityManager();

		entityManager.OnRemove<SkyLightComponent>().Connect([this, uid = aScene.GetUUID()](MAYBE_UNUSED EntityManager& aEntityManager, entity aEntity) { OnSkyLightComponentRemoved(aEntity, uid); });
		entityManager.OnCreated<HiddenInGameComponent>().Connect(this, &SkyLightObserverSystem::OnEntityVisibilityChanged);
		entityManager.OnRemove<HiddenInGameComponent>().Connect(this, &SkyLightObserverSystem::OnEntityVisibilityChanged);
		aScene.OnSkyLightChange.Connect(this, &SkyLightObserverSystem::OnActiveSkyLightChange);
	}

	void SkyLightObserverSystem::OnActiveSkyLightChange(Scene& aScene, entity aCurrentSkyLight, entity aNewSkyLight) noexcept
	{
		if (aCurrentSkyLight != NULL_ENTITY)
			aScene.GetEntityManager().AddOrReplace<SkyLightComponent::DirtyRenderState>(aCurrentSkyLight);
		if (aNewSkyLight != NULL_ENTITY)
			aScene.GetEntityManager().AddOrReplace<SkyLightComponent::DirtyRenderState>(aNewSkyLight);
	}

	void SkyLightObserverSystem::OnEntityVisibilityChanged(EntityManager& aEntityManager, entity aEntity) noexcept
	{
		if (aEntityManager.Has<SkyLightComponent>(aEntity))
			aEntityManager.AddOrReplace<SkyLightComponent::DirtyRenderState>(aEntity);
	}

	void SkyLightObserverSystem::OnSkyLightComponentRemoved(entity aEntity, const UUID& aUUID) noexcept
	{
		Renderer::Dispatch([aEntity, aUUID](Renderer* aRenderer)
			{
				RenderScene* pRenderScene = aRenderer->GetRenderScene(aUUID);
				SkyLightRenderSubsystem* pSkyLightRenderSubsystem = pRenderScene->GetSubsystem<SkyLightRenderSubsystem>();
				pSkyLightRenderSubsystem->Remove({ aEntity });
			});
	}
}