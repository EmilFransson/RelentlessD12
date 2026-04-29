#include "PrimitiveRenderDispatchSystem.h"

#include "ECS/Components/MeshFilterComponent.h"
#include "ECS/Components/MeshRendererComponent.h"
#include "ECS/Components/TransformComponent.h"

#include "Graphics/Renderer/Renderer.h"

#include "Scene/Scene.h"
#include "Subsystem/CoreTypes/PrimitiveRenderSubsystem.h"

namespace Relentless
{
	void PrimitiveRenderDispatchSystem::Execute(SceneState& aSceneState) noexcept
	{
		PROFILE_FUNC;

		auto transformDirtyCollection = aSceneState.EntityManager.Collect<TransformComponent, MeshFilterComponent, MeshRendererComponent, TransformComponent::DirtyRenderState>();
		auto meshFilterDirtyCollection = aSceneState.EntityManager.Collect<TransformComponent, MeshFilterComponent, MeshRendererComponent, MeshFilterComponent::DirtyRenderState>();
		auto meshRenderDirtyCollection = aSceneState.EntityManager.Collect<TransformComponent, MeshFilterComponent, MeshRendererComponent, MeshRendererComponent::DirtyRenderState>();

		const uint32 totalSize = transformDirtyCollection.Size() + meshFilterDirtyCollection.Size() + meshRenderDirtyCollection.Size();
		if (totalSize == 0u)
			return;

		std::unordered_set<entity> dirtyEntities;
		dirtyEntities.reserve(totalSize);

		transformDirtyCollection.Do([&dirtyEntities](entity aEntity) { dirtyEntities.insert(aEntity); });
		meshFilterDirtyCollection.Do([&dirtyEntities](entity aEntity) { dirtyEntities.insert(aEntity); });
		meshRenderDirtyCollection.Do([&dirtyEntities](entity aEntity) { dirtyEntities.insert(aEntity); });

		std::vector<PrimitiveRenderProxy> primitiveRenderProxies;
		primitiveRenderProxies.reserve(dirtyEntities.size());

		for (const entity dirtyEntity : dirtyEntities)
		{
			const auto&[transformComponent, meshFilterComponent, meshRenderComponent] = aSceneState.EntityManager.Get<TransformComponent, MeshFilterComponent, MeshRendererComponent>(dirtyEntity);

			PrimitiveRenderProxy& renderProxy = primitiveRenderProxies.emplace_back();
			renderProxy.EntityID = dirtyEntity;
			renderProxy.LocalToWorld = transformComponent.GetWorldMatrix();
			renderProxy.MeshUUID = meshFilterComponent.HasAssignedMesh() ? meshFilterComponent.GetMeshHandle().Uuid : NULL_UUID;
			renderProxy.MaterialUUID = meshRenderComponent.HasAssignedMaterial() ? meshRenderComponent.GetMaterialHandle().Uuid : NULL_UUID;
			renderProxy.Visible = aSceneState.Scene.IsEntityVisible(dirtyEntity);
		}

		Renderer::Dispatch([renderProxies = std::move(primitiveRenderProxies), uid = aSceneState.Scene.GetUUID()](Renderer* aRenderer)
			{
				RenderScene* pRenderScene = aRenderer->GetRenderScene(uid);
				PrimitiveRenderSubsystem* pPrimitiveRenderSubsystem = pRenderScene->GetSubsystem<PrimitiveRenderSubsystem>();
				pPrimitiveRenderSubsystem->Patch(std::move(renderProxies));
			});
	}
}
