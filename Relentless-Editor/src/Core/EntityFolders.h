#pragma once
#include <Relentless.h>

namespace Relentless
{
	class EntityFolder : public RefCounted<EntityFolder>
	{
	public:
		EntityFolder(const String& aPath, bool aExpanded = true) noexcept;

		void Fixup() noexcept;

		NO_DISCARD const String& GetLabel() const noexcept;
		NO_DISCARD EntityFolder* GetParent() const noexcept;
		NO_DISCARD String GetPath() const noexcept;
		NO_DISCARD const UUID& GetUUID() const noexcept;
		
		NO_DISCARD bool IsExpanded() const noexcept;
		NO_DISCARD bool IsMarkedAsDeleted() const noexcept;

		void MarkAsDeleted() noexcept;
		
		void SetExpandedState(bool aExpandedState) noexcept;
		void SetLabel(const String& aLabel) noexcept;
		void SetParent(EntityFolder* apParentFolder) noexcept;
	private:
		String m_Label;
		UUID m_UUID;
		EntityFolder* m_pParent = nullptr;
		bool m_IsExpanded = true;
		bool m_IsDeleted = false;
	};

	class Editor;
	
	class EntityFoldersManager
	{
	public:
		struct FolderContainer
		{
			std::vector<Ref<EntityFolder>> Folders;
		};

		explicit EntityFoldersManager(Editor* pAEditor) noexcept;

		NO_DISCARD bool AnyFolderContainsEntity(Scene& aScene, entity aEntity) const noexcept;
		void AttachEntityToFolder(Scene& aScene, entity aEntity, const Folder& aFolder) noexcept;

		EntityFolder* CreateFolder(Scene& aScene, const String& aPath) noexcept;
		EntityFolder* CreateFolder(Scene& aScene, const Folder& aFolder) noexcept;
		EntityFolder* CreateFolderContainingSelection(Scene& aScene) noexcept;
		NO_DISCARD bool ContainsFolder(const Scene& aScene, const String& aPath) const noexcept;
		NO_DISCARD bool ContainsFolder(const Scene& aScene, const Folder& aFolder) const noexcept;

		void DeleteFolder(Scene& aScene, const Folder& aFolder) noexcept;
		void DeleteFolder(Scene& aScene, const String& aPath) noexcept;

		NO_DISCARD bool FolderContainsEntity(Scene& aScene, const Folder& aFolder, entity aEntity) noexcept;
		static void ForEachEntityInFolders(Scene& aScene, const std::unordered_set<String>& somePaths, Callback<bool(entity)> aOperation) noexcept;
		void ForEachFolder(const Callback<bool(const EntityFolder&)>& aOperation) noexcept;
		void ForEachFolderWithRootObject(const FolderRoot& aRoot, const Callback<bool(const EntityFolder&)>& aOperation) noexcept;

		NO_DISCARD String GetDefaultFolderName(Scene& aScene, const String& aParentPath) const noexcept;
		NO_DISCARD Folder GetDefaultFolderName(Scene& aScene, const Folder& aParentFolder) const noexcept;
		NO_DISCARD Folder GetDefaultFolderForSelection(Scene& aScene, Span<Folder> someFolders) const noexcept;
		NO_DISCARD String GetFolderName(const Scene& aScene, const String& aParentPath, const String& aFolderName) const noexcept;

		NO_DISCARD bool IsFolderExpanded(Scene& aScene, const String& aPath) const noexcept;
		NO_DISCARD bool IsFolderExpanded(Scene& aScene, const Folder& aFolder) const noexcept;
		NO_DISCARD bool IsInitializedForScene(const Scene& aScene) const noexcept;
		NO_DISCARD bool IsInitializedForRoot(const FolderRoot& aRoot) const noexcept;

		void OnFolderRootObjectRemoved(const FolderRoot& aRootObject) noexcept;

		NO_DISCARD Ref<EntityFolder> GetFolder(const Scene& aScene, const String& aPath) const noexcept;
		NO_DISCARD Ref<EntityFolder> GetFolder(const Scene& aScene, const UUID& aFolderUUID) const noexcept;

		void RemoveEntityFromCurrentFolder(Scene& aScene, entity aEntity) noexcept;
		bool RenameFolder(Scene& aScene, const String& aOldPath, const String& aNewPath) noexcept;

		mutable Broadcaster<void(EntityFolder* apFolder)> OnEntityFolderCreate;
		mutable Broadcaster<void(EntityFolder* apFolder)> OnEntityFolderCreated;
		mutable Broadcaster<void(EntityFolder* apFolder)> OnEntityFolderDelete;
		mutable Broadcaster<void(EntityFolder* apFolder)> OnEntityFolderDeleted;
		mutable Broadcaster<void(EntityFolder* apFolder, EntityFolder* apOldParentFolder, const String& aOldPath, const String& aNewPath)> OnEntityFolderMoved;

		NO_DISCARD FolderContainer& GetFolderContainer(const Scene& aScene) noexcept;
		NO_DISCARD FolderContainer& GetFolderContainer(const FolderRoot& aRoot) noexcept;

		Broadcaster<void(entity, const Folder&)> OnEntityAttachedToFolder;
		Broadcaster<void(entity, const Folder&)> OnEntityRemovedFromFolder;
	private:
		void DeleteFolderContainer(const FolderRoot& aRoot) noexcept;


		NO_DISCARD const FolderContainer& GetFolderContainer(const Scene& aScene) const noexcept;
		NO_DISCARD const FolderContainer& GetFolderContainer(const FolderRoot& aRoot) const noexcept;
	
		void DeduplicateByPath(Scene& scene) noexcept;
		void MergeNodeInto(Scene& scene, Ref<EntityFolder> from, Ref<EntityFolder> into) noexcept;
	private:
		std::unordered_map<UUID, FolderContainer> m_SceneToFolders;
		Editor* m_pEditor = nullptr;
	};
}