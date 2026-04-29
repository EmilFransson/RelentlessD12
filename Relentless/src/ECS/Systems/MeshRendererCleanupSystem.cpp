#include "MeshRendererCleanupSystem.h"

#include "ECS/Components/MeshRendererComponent.h"

#include "Scene/Scene.h"

namespace Relentless
{
	void MeshRendererCleanupSystem::Execute(SceneState& aSceneState) noexcept
	{
		aSceneState.EntityManager.Collect<MeshRendererComponent::DirtyRenderState>().Do([&aSceneState](entity aEntity)
			{
				aSceneState.EntityManager.Remove<MeshRendererComponent::DirtyRenderState>(aEntity);
			});
	}
}