#include "LightObserverSystem.h"

#include "ECS/Components/LightComponent.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/Scene/RenderScene.h"

#include "Scene/Scene.h"
#include "Subsystem/CoreTypes/LightRenderSubsystem.h"

namespace Relentless
{
	void LightObserverSystem::Register(Scene& aScene) noexcept
	{
		EntityManager& entityManager = aScene.GetEntityManager();

		entityManager.OnRemove<DirectionalLightComponent>().Connect([this, uid = aScene.GetUUID()](MAYBE_UNUSED EntityManager& aEntityManager, entity aEntity) { OnLightComponentRemoved(aEntity, uid); });
		entityManager.OnRemove<PointLightComponent>().Connect([this, uid = aScene.GetUUID()](MAYBE_UNUSED EntityManager& aEntityManager, entity aEntity) { OnLightComponentRemoved(aEntity, uid); });
		entityManager.OnRemove<SpotLightComponent>().Connect([this, uid = aScene.GetUUID()](MAYBE_UNUSED EntityManager& aEntityManager, entity aEntity) { OnLightComponentRemoved(aEntity, uid); });
		
		entityManager.OnCreated<HiddenInGameComponent>().Connect(this, &LightObserverSystem::OnEntityVisibilityChanged);
		entityManager.OnRemove<HiddenInGameComponent>().Connect(this, &LightObserverSystem::OnEntityVisibilityChanged);
	}

	void LightObserverSystem::OnEntityVisibilityChanged(EntityManager& aEntityManager, entity aEntity) noexcept
	{
		if (aEntityManager.Has<DirectionalLightComponent>(aEntity))
			aEntityManager.AddOrReplace<LightBaseComponent<DirectionalLightComponent>::DirtyRenderState>(aEntity);
		else if (aEntityManager.Has<PointLightComponent>(aEntity))
			aEntityManager.AddOrReplace<LightBaseComponent<PointLightComponent>::DirtyRenderState>(aEntity);
		else if (aEntityManager.Has<SpotLightComponent>(aEntity))
			aEntityManager.AddOrReplace<LightBaseComponent<SpotLightComponent>::DirtyRenderState>(aEntity);
	}

	void LightObserverSystem::OnLightComponentRemoved(entity aEntity, const UUID& aUUID) noexcept
	{
		Renderer::Dispatch([aEntity, aUUID](Renderer* aRenderer)
			{
				RenderScene* pRenderScene = aRenderer->GetRenderScene(aUUID);
				LightRenderSubsystem* pLightRenderSubsystem = pRenderScene->GetSubsystem<LightRenderSubsystem>();
				pLightRenderSubsystem->Remove({ aEntity });
			});
	}
}