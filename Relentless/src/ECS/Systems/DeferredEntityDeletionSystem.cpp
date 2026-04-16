#include "DeferredEntityDeletionSystem.h"

#include "Scene/Scene.h"

namespace Relentless
{
	void DeferredEntityDeletionSystem::Execute(SceneState& aSceneState) noexcept
	{
		aSceneState.EntityManager.Collect<EntityDeleteRequestComponent, IDComponent>().Do([&aSceneState](entity aEntity, IDComponent& aIDComponent)
			{
				RLS_ASSERT(aSceneState.EntityManager.Exists(aEntity), "[DeferredEntityDeletionSystem::Execute]: Entity does not exist.");

				if (aSceneState.Scene.HasParent(aEntity))
					aSceneState.Scene.DetachEntity(aEntity);

				const std::vector<entity> children = aSceneState.Scene.GetEntityChildren(aEntity);
				for (entity child : children)
					aSceneState.Scene.DetachEntity(child);

				aSceneState.Scene.OnEntityDestroy(aEntity);
				aSceneState.Scene.m_EntityUUIDMap.erase(aIDComponent.UuId);
				aSceneState.EntityManager.DestroyEntity(aEntity);
				aSceneState.Scene.OnEntityDestroyed(aEntity);
			});
	}
}