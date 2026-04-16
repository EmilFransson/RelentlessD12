#include "SkyLightObserverSystem.h"
#include "Scene/Scene.h"

#include "ECS/Components/SkyLightComponent.h"

#include "Graphics/Renderer/Renderer.h"

#include "Subsystem/CoreTypes/SkyLightRenderSubsystem.h"

namespace Relentless
{
	void SkyLightObserverSystem::Register(Scene& aScene) noexcept
	{
		EntityManager& entityManager = aScene.GetEntityManager();

		entityManager.OnRemove<SkyLightComponent>().Connect(this, &SkyLightObserverSystem::OnSkyLightComponentRemoved);
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

	void SkyLightObserverSystem::OnSkyLightComponentRemoved(MAYBE_UNUSED EntityManager& aEntityManager, entity aEntity) noexcept
	{
		Renderer::Dispatch([aEntity](Renderer* aRenderer)
			{
				SkyLightRenderSubsystem* pSkyLightRenderSubsystem = aRenderer->GetSubsystem<SkyLightRenderSubsystem>();
				pSkyLightRenderSubsystem->Remove({ aEntity });
			});
	}
}