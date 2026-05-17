#include "EditorSceneBridgeSubsystem.h"

#include "../Core/Editor.h"
#include "EntityFoldersSubsystem.h"
#include "SelectionSubsystem.h"

namespace Relentless
{
	bool EditorSceneBridgeSubsystem::ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept
	{
		return dynamic_cast<Editor*>(aSystemManager) != nullptr;
	}

	bool EditorSceneBridgeSubsystem::OnLoad(ISystemManager* aSystemManager) noexcept
	{
		Editor* pEditor = static_cast<Editor*>(aSystemManager);
		pEditor->OnSceneChange.Connect(this, &EditorSceneBridgeSubsystem::OnSceneChange);
		pEditor->OnSceneChanged.Connect(this, &EditorSceneBridgeSubsystem::OnSceneChanged);
		pEditor->GetSubsystem<SelectionSubsystem>()->OnSelectionChanged.Connect(this, &EditorSceneBridgeSubsystem::OnEntitySelectionChanged);

		EntityFoldersSubsystem* pFoldersSubsystem = pEditor->GetSubsystem<EntityFoldersSubsystem>();
		pFoldersSubsystem->OnEntityFolderMoved.Connect(this, &EditorSceneBridgeSubsystem::OnEntityFolderMoved);
		pFoldersSubsystem->OnEntityFolderDelete.Connect(this, &EditorSceneBridgeSubsystem::OnEntityFolderDelete);

		m_pEditor = pEditor;
		return true;
	}

	void EditorSceneBridgeSubsystem::DeleteSelectedEntities() noexcept
	{
		SelectionSubsystem* pSelection = m_pEditor->GetSubsystem<SelectionSubsystem>();
		const std::vector<entity>& selectedEntities = pSelection->GetSelectedEntities();

		for (int i = static_cast<int>(selectedEntities.size()) - 1; i >= 0; --i)
			m_pBridgedScene->DestroyEntity(selectedEntities[i]);
	}

	void EditorSceneBridgeSubsystem::SelectAllEntities() noexcept
	{
		SelectionSubsystem* pSelection = m_pEditor->GetSubsystem<SelectionSubsystem>();
		m_pBridgedScene->GetEntityManager().Collect<IDComponent>().Do([pSelection](entity aEntity)
			{
				if (!pSelection->IsEntitySelected(aEntity))
					pSelection->SelectEntity(aEntity);
			});
	}

	void EditorSceneBridgeSubsystem::SetVisibilityForSelectedEntities(bool aVisibilityState) noexcept
	{
		if (aVisibilityState)
		{
			m_pBridgedScene->GetEntityManager().Collect<HiddenInGameComponent, RootComponent>().Do([this, aVisibilityState](entity aEntity)
				{
					m_pBridgedScene->SetEntityVisibleInGame(aEntity, aVisibilityState);
				});

			m_pBridgedScene->GetEntityManager().Collect<HiddenInGameComponent>().Do([this, aVisibilityState](entity aEntity)
				{
					m_pBridgedScene->SetEntityVisibleInGame(aEntity, aVisibilityState);
				});
		}
		else
		{
			SelectionSubsystem* pSelection = m_pEditor->GetSubsystem<SelectionSubsystem>();

			const std::vector<entity>& selectedEntities = pSelection->GetSelectedEntities();

			for (int i = (int)selectedEntities.size() - 1; i >= 0; --i)
			{
				const entity currentEntity = selectedEntities[i];

				m_pBridgedScene->SetEntityVisibleInGame(currentEntity, aVisibilityState);
				pSelection->DeselectEntity(currentEntity);
			}
		}
	}

	void EditorSceneBridgeSubsystem::OnEntityAttached(entity aChildEntity, MAYBE_UNUSED entity aParentEntity) noexcept
	{
		if (!m_pBridgedScene)
			return;

		EntityManager& entityManager = m_pBridgedScene->GetEntityManager();

		if (entityManager.Has<FolderComponent>(aChildEntity))
			entityManager.Remove<FolderComponent>(aChildEntity);
	}

	void EditorSceneBridgeSubsystem::OnEntityDestroy(entity aEntity) noexcept
	{
		SelectionSubsystem* pSelection = m_pEditor->GetSubsystem<SelectionSubsystem>();

		if (pSelection->IsEntitySelected(aEntity))
			pSelection->DeselectEntity(aEntity);
	}

	void EditorSceneBridgeSubsystem::OnEntityFolderDelete(EntityFolder* aFolder) noexcept
	{
		if (!m_pBridgedScene)
			return;

		m_pBridgedScene->GetEntityManager().Collect<FolderComponent>().Do([&](entity aEntity, FolderComponent& fc)
			{
				if (fc.Folder.GetPath() == aFolder->GetPath())
					m_pBridgedScene->GetEntityManager().Remove<FolderComponent>(aEntity);
			});
	}

	void EditorSceneBridgeSubsystem::OnEntityFolderMoved(MAYBE_UNUSED EntityFolder* aMovedFolder, MAYBE_UNUSED EntityFolder* aMovedFolderParent, const String& aOldPath, const String& aNewPath) noexcept
	{
		if (!m_pBridgedScene)
			return;

		m_pBridgedScene->GetEntityManager().Collect<FolderComponent>().Do([&](FolderComponent& fc)
			{
				const String& path = fc.Folder.GetPath();

				if (path == aOldPath)
					fc.Folder = Folder(fc.Folder.GetRoot(), aNewPath);
				else if (path.size() > aOldPath.size() && path.compare(0, aOldPath.size(), aOldPath) == 0 && path[aOldPath.size()] == '/')
				{
					const String suffix = path.substr(aOldPath.size());
					fc.Folder = Folder(fc.Folder.GetRoot(), aNewPath + suffix);
				}
			});
	}

	void EditorSceneBridgeSubsystem::OnEntitySelectionChanged(entity aEntity, ESelectionState aSelectionState) noexcept
	{
		if (!m_pBridgedScene)
			return;

		if (aSelectionState == ESelectionState::Selected)
			m_pBridgedScene->GetEntityManager().AddOrReplace<SelectedInEditorComponent>(aEntity);
		else
			m_pBridgedScene->GetEntityManager().Remove<SelectedInEditorComponent>(aEntity);
	}

	void EditorSceneBridgeSubsystem::OnSceneChange(Scene* aCurrentScene) noexcept
	{
		if (aCurrentScene && aCurrentScene == m_pBridgedScene)
		{
			m_pBridgedScene->OnEntityDestroy.Detach(this);
			m_pBridgedScene->OnEntityAttached.Detach(this);
		}
	}

	void EditorSceneBridgeSubsystem::OnSceneChanged(Scene* aNewScene) noexcept
	{
		m_pBridgedScene = aNewScene;
		if (!m_pBridgedScene)
			return;

		m_pBridgedScene->OnEntityDestroy.Connect(this, &EditorSceneBridgeSubsystem::OnEntityDestroy);
		m_pBridgedScene->OnEntityAttached.Connect(this, &EditorSceneBridgeSubsystem::OnEntityAttached);
	}
}
