#include "TransformRenderDispatchSystem.h"

#include "Scene/Scene.h"
#include "Subsystem/CoreTypes/SkyLightRenderSubsystem.h"

namespace Relentless
{
	void TransformRenderDispatchSystem::Execute(SceneState& aSceneState) noexcept
	{
		auto collection = aSceneState.EntityManager.Collect<TransformComponent, TransformComponent::DirtyRenderState>();
		const uint32 collectionSize = collection.Size();
		if (collectionSize == 0u)
			return;

		collection.Do([&aSceneState](entity aEntity)
			{
				aSceneState.EntityManager.Remove<TransformComponent::DirtyRenderState>(aEntity);
			});
	}
}
