#pragma once
#include <Relentless.h>
#include "IEditorSubsystem.h"

namespace Relentless
{
	class Editor;
	class EntityFolder;
	enum class ESelectionState : uint8;

	class EditorSceneBridgeSubsystem : public IEditorSubsystem
	{
	public:
		NO_DISCARD virtual bool OnLoad(ISystemManager* aSystemManager) noexcept override;
		
		void DeleteSelectedEntities() noexcept;

		void SelectAllEntities() noexcept;
		void SetVisibilityForSelectedEntities(bool aVisibilityState) noexcept;
		static bool ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept;
	protected:
		void OnEntityAttached(entity aChildEntity, MAYBE_UNUSED entity aParentEntity) noexcept;
		void OnEntityDestroy(entity aEntity) noexcept;
		void OnEntityFolderDelete(EntityFolder* aFolder) noexcept;
		void OnEntityFolderMoved(MAYBE_UNUSED EntityFolder* aMovedFolder, MAYBE_UNUSED EntityFolder* aMovedFolderParent, const String& aOldPath, const String& aNewPath) noexcept;
		void OnEntitySelectionChanged(entity aEntity, ESelectionState aSelectionState) noexcept;
		void OnSceneChanged(Scene* aNewScene) noexcept;
	private:
		Editor* m_pEditor = nullptr;
		Scene* m_pBridgedScene = nullptr;
	};
}