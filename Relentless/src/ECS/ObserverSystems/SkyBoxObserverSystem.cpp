#include "SkyBoxObserverSystem.h"
#include "Scene/Scene.h"

#include "ECS/Components/SkyBoxComponent.h"

#include "Graphics/Renderer/Renderer.h"

#include "Subsystem/CoreTypes/SkyBoxRenderSubsystem.h"

namespace Relentless
{
	void SkyBoxObserverSystem::Register(Scene& aScene) noexcept
	{
		EntityManager& entityManager = aScene.GetEntityManager();

		entityManager.OnRemove<SkyBoxComponent>().Connect(this, &SkyBoxObserverSystem::OnSkyBoxComponentRemoved);
		entityManager.OnCreated<HiddenInGameComponent>().Connect(this, &SkyBoxObserverSystem::OnEntityVisibilityChanged);
		entityManager.OnRemove<HiddenInGameComponent>().Connect(this, &SkyBoxObserverSystem::OnEntityVisibilityChanged);
		aScene.OnSkyBoxChange.Connect(this, &SkyBoxObserverSystem::OnActiveSkyBoxChange);
	}

	void SkyBoxObserverSystem::OnActiveSkyBoxChange(Scene& aScene, entity aCurrentSkybox, entity aNewSkyBox) noexcept
	{
		if (aCurrentSkybox != NULL_ENTITY)
			aScene.GetEntityManager().AddOrReplace<SkyBoxComponent::DirtyRenderState>(aCurrentSkybox);
		if (aNewSkyBox != NULL_ENTITY)
			aScene.GetEntityManager().AddOrReplace<SkyBoxComponent::DirtyRenderState>(aNewSkyBox);
	}

	void SkyBoxObserverSystem::OnEntityVisibilityChanged(EntityManager& aEntityManager, entity aEntity) noexcept
	{
		if (aEntityManager.Has<SkyBoxComponent>(aEntity))
			aEntityManager.AddOrReplace<SkyBoxComponent::DirtyRenderState>(aEntity);
	}

	void SkyBoxObserverSystem::OnSkyBoxComponentRemoved(MAYBE_UNUSED EntityManager& aEntityManager, entity aEntity) noexcept
	{
		Renderer::Dispatch([aEntity](Renderer* aRenderer)
			{
				SkyBoxRenderSubsystem* pSkyBoxRenderSubsystem = aRenderer->GetSubsystem<SkyBoxRenderSubsystem>();
				pSkyBoxRenderSubsystem->Remove({ aEntity });
			});
	}
}