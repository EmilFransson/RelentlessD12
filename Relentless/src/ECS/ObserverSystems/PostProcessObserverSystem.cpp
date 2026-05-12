#include "PostProcessObserverSystem.h"

#include "ECS/Components/PostProcessVolumeComponent.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/Scene/RenderScene.h"

#include "Scene/Scene.h"
#include "Subsystem/CoreTypes/PostProcessRenderSubsystem.h"

namespace Relentless
{
	void PostProcessObserverSystem::Register(Scene& aScene) noexcept
	{
		EntityManager& entityManager = aScene.GetEntityManager();

		entityManager.OnRemove<PostProcessVolumeComponent>().Connect([this, uid = aScene.GetUUID()](MAYBE_UNUSED EntityManager& aEntityManager, entity aEntity) { OnPostProcessVolumeComponentRemoved(aEntity, uid); });

		entityManager.OnCreated<HiddenInGameComponent>().Connect(this, &PostProcessObserverSystem::OnEntityVisibilityChanged);
		entityManager.OnRemove<HiddenInGameComponent>().Connect(this, &PostProcessObserverSystem::OnEntityVisibilityChanged);
	}

	void PostProcessObserverSystem::OnEntityVisibilityChanged(EntityManager& aEntityManager, entity aEntity) noexcept
	{
		if (aEntityManager.Has<PostProcessVolumeComponent>(aEntity))
			aEntityManager.AddOrReplace<PostProcessVolumeComponent::DirtyRenderState>(aEntity);
	}

	void PostProcessObserverSystem::OnPostProcessVolumeComponentRemoved(entity aEntity, const UUID& aUUID) noexcept
	{
		Renderer::Dispatch([aEntity, aUUID](Renderer* aRenderer)
			{
				RenderScene* pRenderScene = aRenderer->GetRenderScene(aUUID);
				PostProcessRenderSubsystem* pPostProcessRenderSubsystem = pRenderScene->GetSubsystem<PostProcessRenderSubsystem>();
				pPostProcessRenderSubsystem->Remove({ aEntity });
			});
	}
}