#include "SelectionObserverSystem.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/Scene/RenderScene.h"

#include "Scene/Scene.h"
#include "Subsystem/CoreTypes/SelectionRenderSubsystem.h"

namespace Relentless
{
	void SelectionObserverSystem::Register(Scene& aScene) noexcept
	{
		EntityManager& entityManager = aScene.GetEntityManager();

		entityManager.OnCreated<SelectedInEditorComponent>().Connect([this, sceneUUID = aScene.GetUUID()](MAYBE_UNUSED EntityManager& aEntityManager, entity aEntity) { OnEntitySelectedInEditor(sceneUUID, aEntity); });
		entityManager.OnRemove<SelectedInEditorComponent>().Connect([this, sceneUUID = aScene.GetUUID()](MAYBE_UNUSED EntityManager& aEntityManager, entity aEntity) { OnEntityDeselectedInEditor(sceneUUID, aEntity); });
	}

	void SelectionObserverSystem::OnEntitySelectedInEditor(const UUID& aSceneUUID, entity aEntity) noexcept
	{
		Renderer::Dispatch([aSceneUUID, aEntity](Renderer* aRenderer)
			{
				RenderScene* pRenderScene = aRenderer->GetRenderScene(aSceneUUID);
				pRenderScene->GetSubsystem<SelectionRenderSubsystem>()->Select({ aEntity });
			});
	}

	void SelectionObserverSystem::OnEntityDeselectedInEditor(const UUID& aSceneUUID, entity aEntity) noexcept
	{
		Renderer::Dispatch([aSceneUUID, aEntity](Renderer* aRenderer)
			{
				RenderScene* pRenderScene = aRenderer->GetRenderScene(aSceneUUID);
				pRenderScene->GetSubsystem<SelectionRenderSubsystem>()->Deselect({ aEntity });
			});
	}
}