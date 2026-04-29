#include "TransformCleanupSystem.h"

#include "ECS/Components/TransformComponent.h"

#include "Scene/Scene.h"

namespace Relentless
{
	void TransformCleanupSystem::Execute(SceneState& aSceneState) noexcept
	{
		aSceneState.EntityManager.Collect<TransformComponent::DirtyRenderState>().Do([&aSceneState](entity aEntity)
			{
				aSceneState.EntityManager.Remove<TransformComponent::DirtyRenderState>(aEntity);
			});
	}
}