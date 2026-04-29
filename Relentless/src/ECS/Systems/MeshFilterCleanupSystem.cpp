#include "MeshFilterCleanupSystem.h"

#include "ECS/Components/MeshFilterComponent.h"

#include "Scene/Scene.h"

namespace Relentless
{
	void MeshFilterCleanupSystem::Execute(SceneState& aSceneState) noexcept
	{
		aSceneState.EntityManager.Collect<MeshFilterComponent::DirtyRenderState>().Do([&aSceneState](entity aEntity)
			{
				aSceneState.EntityManager.Remove<MeshFilterComponent::DirtyRenderState>(aEntity);
			});
	}
}