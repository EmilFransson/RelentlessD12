#include "PrimitiveObserverSystem.h"

#include "Graphics/Renderer/Renderer.h"

#include "ECS/Components/MeshFilterComponent.h"
#include "ECS/Components/MeshRendererComponent.h"
#include "ECS/Components/TransformComponent.h"

#include "Scene/Scene.h"
#include "Subsystem/CoreTypes/PrimitiveRenderSubsystem.h"

namespace Relentless
{
	void PrimitiveObserverSystem::Register(Scene& aScene) noexcept
	{
		EntityManager& entityManager = aScene.GetEntityManager();

		entityManager.OnRemove<TransformComponent>().Connect([this, uid = aScene.GetUUID()](EntityManager& aEntityManager, entity aEntity) { OnComponentRemoved(aEntityManager, aEntity, uid); });
		entityManager.OnRemove<MeshRendererComponent>().Connect([this, uid = aScene.GetUUID()](EntityManager& aEntityManager, entity aEntity) { OnComponentRemoved(aEntityManager, aEntity, uid); });
		entityManager.OnRemove<MeshFilterComponent>().Connect([this, uid = aScene.GetUUID()](EntityManager& aEntityManager, entity aEntity) { OnComponentRemoved(aEntityManager, aEntity, uid); });

		entityManager.OnCreated<HiddenInGameComponent>().Connect(this, &PrimitiveObserverSystem::OnEntityVisibilityChanged);
		entityManager.OnRemove<HiddenInGameComponent>().Connect(this, &PrimitiveObserverSystem::OnEntityVisibilityChanged);
	}

	void PrimitiveObserverSystem::OnEntityVisibilityChanged(EntityManager& aEntityManager, entity aEntity) noexcept
	{
		//Transform component guaranteed:
		aEntityManager.AddOrReplace<TransformComponent::DirtyRenderState>(aEntity);

		if (aEntityManager.Has<MeshRendererComponent>(aEntity))
			aEntityManager.AddOrReplace<MeshRendererComponent::DirtyRenderState>(aEntity);
		if (aEntityManager.Has<MeshFilterComponent>(aEntity))
			aEntityManager.AddOrReplace<MeshFilterComponent::DirtyRenderState>(aEntity);
	}

	void PrimitiveObserverSystem::OnComponentRemoved(EntityManager& aEntityManager, entity aEntity, const UUID& aUUID) noexcept
	{
		if (aEntityManager.HasAllOf<TransformComponent, MeshRendererComponent, MeshFilterComponent>(aEntity))
		{
			Renderer::Dispatch([aEntity, aUUID](Renderer* aRenderer)
				{
					RenderScene* pRenderScene = aRenderer->GetRenderScene(aUUID);
					PrimitiveRenderSubsystem* pPrimitiveRenderSubsystem = pRenderScene->GetSubsystem<PrimitiveRenderSubsystem>();
					pPrimitiveRenderSubsystem->Remove({ aEntity });
				});
		}
	}
}