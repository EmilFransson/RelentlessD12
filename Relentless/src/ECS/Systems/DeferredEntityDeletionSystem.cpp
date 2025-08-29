#include "DeferredEntityDeletionSystem.h"

#include "Scene/Scene.h"

namespace Relentless
{
	void DeferredEntityDeletionSystem::Execute(SceneState& sceneState) noexcept
	{
		while (!sceneState.Scene.m_PendingEntityDeletionQueue.Empty())
		{
			entity toDestroy = NULL_ENTITY;
			if (!sceneState.Scene.m_PendingEntityDeletionQueue.TryPop(toDestroy))
				continue;

			if (!sceneState.EntityManager.Exists(toDestroy))
				continue;

			//Check if the entity itself is a child. If it is, remove its entry from parent child list:
			if (sceneState.Scene.HasParent(toDestroy))
				sceneState.Scene.DetachEntity(toDestroy);
		
			const std::vector<entity> children = sceneState.Scene.GetEntityChildren(toDestroy);
			for (entity child : children)
				sceneState.Scene.DetachEntity(child);

			sceneState.Scene.OnEntityPreDestroyed(toDestroy);
			sceneState.EntityManager.DestroyEntity(toDestroy);
			sceneState.Scene.OnEntityDestroyed(toDestroy);
		}
	}
}