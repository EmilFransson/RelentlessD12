#include "EntityFoldersSubsystem.h"
#include "../Core/Editor.h"

#include "../UI/Views/Outliner/EntityOutlinerView.h"

namespace Relentless
{
	static void CollectDirectChildren(Relentless::EntityFoldersSubsystem& M, Relentless::Scene& scene, Relentless::EntityFolder* parent, std::vector<Relentless::Ref<Relentless::EntityFolder>>& out)
	{
		out.clear();
		auto& vec = M.GetFolderContainer(scene).Folders;

		for (auto& f : vec)
		{
			if (f->GetParent() == parent)
				out.push_back(f);
		}
	}

	static Relentless::EntityFolder* FindChildByLabel(Relentless::EntityFoldersSubsystem& M, Relentless::Scene& scene, Relentless::EntityFolder* parent, const String& label)
	{
		auto& vec = M.GetFolderContainer(scene).Folders;
		for (auto& f : vec)
		{
			if (f->GetParent() == parent && f->GetLabel() == label)
				return f;
		}

		return nullptr;
	}

	void EntityFoldersSubsystem::MergeNodeInto(Scene& scene, Ref<EntityFolder> from, Ref<EntityFolder> into) noexcept
	{
		if (from == into)
			return;

		std::vector<std::pair<EntityFolder*, EntityFolder*>> stack;
		stack.emplace_back(from, into);

		std::vector<Ref<EntityFolder>> srcChildren;
		std::vector<Ref<EntityFolder>> toDelete;

		while (!stack.empty())
		{
			auto [srcParent, dstParent] = stack.back();
			stack.pop_back();

			CollectDirectChildren(*this, scene, srcParent, srcChildren);

			for (auto& srcChild : srcChildren)
			{
				EntityFolder* dstChild = FindChildByLabel(*this, scene, dstParent, srcChild->GetLabel());
				if (dstChild)
				{
					stack.emplace_back(srcChild, dstChild);
					toDelete.push_back(srcChild);
				}
				else
				{
					srcChild->SetParent(dstParent);
					srcChild->Fixup();
				}
			}
		}

		auto& vec = GetFolderContainer(scene).Folders;
		for (auto& lose : toDelete)
		{
			auto it = std::find(vec.begin(), vec.end(), lose);
			if (it != vec.end())
			{
				auto last = vec.end() - 1;
				if (it != last) std::iter_swap(it, last);
				vec.pop_back();
			}
		}

		auto& vec2 = GetFolderContainer(scene).Folders;
		auto it = std::find(vec2.begin(), vec2.end(), from);
		if (it != vec2.end())
		{
			auto last = vec2.end() - 1;
			if (it != last) std::iter_swap(it, last);
			vec2.pop_back();
		}
	}

	void EntityFoldersSubsystem::DeduplicateByPath(Scene& scene) noexcept
	{
		auto& vec = GetFolderContainer(scene).Folders;
		std::vector<Ref<EntityFolder>> snapshot = vec; // shallow copy of refs

		std::unordered_map<String, Ref<EntityFolder>> canonical;
		canonical.reserve(snapshot.size() * 2);

		for (auto& f : snapshot)
		{
			if (f->IsMarkedAsDeleted())
				continue;

			const String path = f->GetPath();
			if (!canonical.contains(path))
				canonical.emplace(path, f);
		}

		for (auto& f : snapshot)
		{
			if (f->IsMarkedAsDeleted())
				continue;

			const String path = f->GetPath();

			auto it = canonical.find(path);
			if (it == canonical.end())
				continue;

			Ref<EntityFolder> keep = it->second;
			if (keep == f)
				continue; // this one is canonical

			MergeNodeInto(scene, f, keep);
		}

#if defined(RLS_DEBUG)
		{
			std::unordered_set<String> seen;
			for (auto& f : GetFolderContainer(scene).Folders)
			{
				const String p = f->GetPath();
				RLS_ASSERT(!seen.contains(p), "Duplicate path remains after dedup");
				seen.insert(p);
			}
		}
#endif
	}

	bool EntityFoldersSubsystem::AnyFolderContainsEntity(Scene& aScene, entity aEntity) const noexcept
	{
		return aScene.GetEntityManager().Exists(aEntity) && aScene.GetEntityManager().Has<FolderComponent>(aEntity);
	}

	void EntityFoldersSubsystem::AttachEntityToFolder(Scene& aScene, entity aEntity, const Folder& aFolder) noexcept
	{
		if (aScene.EntityHasAncestors(aEntity))
			aScene.DetachEntity(aEntity);

		if (!ContainsFolder(aScene, aFolder))
			CreateFolder(aScene, aFolder);

		aScene.GetEntityManager().AddOrReplace<FolderComponent>(aEntity).Folder = aFolder;
		OnEntityAttachedToFolder(aEntity, aFolder);
	}

	EntityFolder* EntityFoldersSubsystem::CreateFolder(Scene& aScene, const String& aPath) noexcept
	{
		if (!IsInitializedForScene(aScene))
			m_SceneToFolders[aScene.GetUUID()] = FolderContainer{};

		if (Ref<EntityFolder> pFolder = GetFolder(aScene, aPath))
			return pFolder;

		const std::vector<String> components = StringUtils::Split(aPath, '/');
		if (components.empty())
			return nullptr;

		auto&& ConditionallyCreateFolder = [this, &aScene](const String& aInParentPath, const String& aLabel) -> Ref<EntityFolder>
			{
				Ref<EntityFolder> pFolder = GetFolder(aScene, aInParentPath + (aInParentPath.empty() ? "" : "/") + aLabel);
				EntityFolder* pParent = aInParentPath.empty() ? nullptr : GetFolder(aScene, aInParentPath);

				if (!pFolder)
				{
					pFolder = RLS_NEW EntityFolder(aLabel);

					if (!aInParentPath.empty())
						pFolder->SetParent(pParent);

					GetFolderContainer(aScene).Folders.push_back(pFolder);
					OnEntityFolderCreated(pFolder);
				}
				else if (pFolder->GetParent() != pParent)
					pFolder->SetParent(pParent);

				return pFolder;
			};

		String currentPath = components[0];
		EntityFolder* pFolderToReturn = ConditionallyCreateFolder("", currentPath);

		for (size_t i = 1u; i < components.size(); ++i)
		{
			pFolderToReturn = ConditionallyCreateFolder(currentPath, components[i]);
			currentPath += "/" + components[i];
		}

		return pFolderToReturn;
	}

	EntityFolder* EntityFoldersSubsystem::CreateFolder(Scene& aScene, const Folder& aFolder) noexcept
	{
		return CreateFolder(aScene, aFolder.GetPath());
	}

	EntityFolder* EntityFoldersSubsystem::CreateFolderContainingSelection(Scene& aScene) noexcept
	{
		Ref<EntityOutlinerView> pOutlinerView = m_pEditor->GetEntityOutlinerView();

		std::vector<Folder> folders;
		folders.resize(pOutlinerView->m_SelectedFolders.size());

		for (const UUID& uuid : pOutlinerView->m_SelectedFolders)
		{
			if (EntityFolder* pFolder = GetFolder(aScene, uuid))
				folders.push_back(Folder(FolderRoot::CreateFromScene(aScene), pFolder->GetPath()));
		}

		const Folder newFolder = GetDefaultFolderForSelection(aScene, folders);

		return CreateFolder(aScene, newFolder);
	}

	bool EntityFoldersSubsystem::ContainsFolder(const Scene& aScene, const Folder& aFolder) const noexcept
	{
		return ContainsFolder(aScene, aFolder.GetPath());
	}

	bool EntityFoldersSubsystem::ContainsFolder(const Scene& aScene, const String& aPath) const noexcept
	{
		if (!IsInitializedForScene(aScene))
			return false;

		const std::vector<Ref<EntityFolder>>& folders = GetFolderContainer(aScene).Folders;
		return std::ranges::any_of(folders, [&](auto const& f) { return f->GetPath() == aPath && !f->IsMarkedAsDeleted(); });
	}

	void EntityFoldersSubsystem::DeleteFolder(Scene& aScene, const Folder& aFolder) noexcept
	{
		DeleteFolder(aScene, aFolder.GetPath());
	}

	void EntityFoldersSubsystem::DeleteFolder(Scene& aScene, const String& aPath) noexcept
	{
		Ref<EntityFolder> pToDelete = GetFolder(aScene, aPath);
		if (!pToDelete)
			return;

		pToDelete->MarkAsDeleted();

		OnEntityFolderDelete(pToDelete);

		std::vector<Ref<EntityFolder>>& folders = GetFolderContainer(aScene).Folders;
		for (auto& pFolder : folders)
		{
			const String folderPathPreFixUp = pFolder->GetPath();
			EntityFolder* pParent = pFolder->GetParent();
			pFolder->Fixup();

			if (pParent != pFolder->GetParent())
				OnEntityFolderMoved(pFolder, pParent, folderPathPreFixUp, pFolder->GetPath());
		}

		const auto it = std::find_if(folders.begin(), folders.end(), [&](auto const& pFolder) { return pFolder == pToDelete; });

		if (it != folders.end()) {
			auto last = folders.end() - 1;
			if (it != last)
				std::iter_swap(it, last);

			folders.pop_back();
		}

		DeduplicateByPath(aScene);
	}

	bool EntityFoldersSubsystem::FolderContainsEntity(Scene& aScene, const Folder& aFolder, entity aEntity) noexcept
	{
		return aScene.GetEntityManager().Has<FolderComponent>(aEntity) && aScene.GetEntityManager().Get<FolderComponent>(aEntity).Folder == aFolder;
	}

	void EntityFoldersSubsystem::ForEachEntityInFolders(Scene& aScene, const std::unordered_set<String>& somePaths, Callback<bool(entity)> aOperation) noexcept
	{
		if (somePaths.empty())
			return;

		std::vector<entity> entities;

		aScene.GetEntityManager().Collect<FolderComponent>().Do([&](entity aEntity, const FolderComponent& fc)
			{
				if (somePaths.contains(fc.Folder.GetPath()))
					entities.push_back(aEntity);
			});

		for (entity currentEntity : entities)
		{
			if (!aOperation(currentEntity))
				return;
		}
	}

	String EntityFoldersSubsystem::GetDefaultFolderName(Scene& aScene, const String& aParentPath) const noexcept
	{
		return GetFolderName(aScene, aParentPath, "New Folder");
	}

	Folder EntityFoldersSubsystem::GetDefaultFolderName(Scene& aScene, const Folder& aParentFolder) const noexcept
	{
		return Folder(aParentFolder.GetRoot(), GetDefaultFolderName(aScene, aParentFolder.GetPath()));
	}

	Folder EntityFoldersSubsystem::GetDefaultFolderForSelection(Scene& aScene, Span<Folder> someFolders) const noexcept
	{
		if (!IsInitializedForScene(aScene))
			return Folder{};

		if (someFolders.GetSize() == 0)
			return Folder(FolderRoot::CreateFromScene(aScene), GetDefaultFolderName(aScene, ""));

		const String originalProposedPath = "New Folder";
		String proposedPath = originalProposedPath;

		std::unordered_set<String> folderLabels;
		for (uint32 i = 0; i < someFolders.GetSize(); ++i)
		{
			const String& fullPath = someFolders[i].GetPath();
			const String label = fullPath.contains('/') ? fullPath.substr(fullPath.rfind('/') + 1) : fullPath;
			folderLabels.insert(label);
		}

		uint32 counter = 1;
		while (ContainsFolder(aScene, proposedPath) || folderLabels.contains(proposedPath))
			proposedPath = std::format("{}{}", originalProposedPath, ++counter);

		return Folder(FolderRoot::CreateFromScene(aScene), proposedPath);
	}

	String EntityFoldersSubsystem::GetFolderName(const Scene& aScene, const String& aParentPath, const String& aFolderName) const noexcept
	{
		if (!IsInitializedForScene(aScene))
			return "";

		const String originalProposedPath = aParentPath + (aParentPath.empty() ? "" : "/") + aFolderName;
		String proposedPath = aParentPath + (aParentPath.empty() ? "" : "/") + aFolderName;

		uint32 counter = 1;
		while (ContainsFolder(aScene, proposedPath))
			proposedPath = std::format("{}{}", originalProposedPath, ++counter);

		if (!proposedPath.contains('/'))
			return proposedPath;
		else
			return proposedPath.substr(proposedPath.rfind('/') + 1);
	}

	bool EntityFoldersSubsystem::IsFolderExpanded(Scene& aScene, const String& aPath) const noexcept
	{
		return ContainsFolder(aScene, aPath) && GetFolder(aScene, aPath)->IsExpanded();
	}

	bool EntityFoldersSubsystem::IsFolderExpanded(Scene& aScene, const Folder& aFolder) const noexcept
	{
		return IsFolderExpanded(aScene, aFolder.GetPath());
	}

	bool EntityFoldersSubsystem::IsInitializedForScene(const Scene& aScene) const noexcept
	{
		return m_SceneToFolders.contains(aScene.GetUUID());
	}

	bool EntityFoldersSubsystem::IsInitializedForRoot(const FolderRoot& aRoot) const noexcept
	{
		return m_SceneToFolders.contains(aRoot.UUID);
	}

	bool EntityFoldersSubsystem::OnLoad(ISystemManager* aSystemManager) noexcept
	{
		m_pEditor = static_cast<Editor*>(aSystemManager);
		return true;
	}

	void EntityFoldersSubsystem::OnFolderRootObjectRemoved(const FolderRoot& aRootObject) noexcept
	{
		if (IsInitializedForRoot(aRootObject))
			DeleteFolderContainer(aRootObject);
	}

	void EntityFoldersSubsystem::ForEachFolder(const Callback<bool(const EntityFolder&)>& aOperation) noexcept
	{
		for (auto& [uuid, container] : m_SceneToFolders)
		{
			for (const Ref<EntityFolder>& pFolder : container.Folders)
			{
				if (!aOperation(*pFolder))
					return;
			}
		}
	}

	void EntityFoldersSubsystem::ForEachFolderWithRootObject(const FolderRoot& aRoot, const Callback<bool(const EntityFolder&)>& aOperation) noexcept
	{
		if (!IsInitializedForRoot(aRoot))
			return;

		const std::vector<Ref<EntityFolder>>& folders = GetFolderContainer(aRoot).Folders;

		for (const Ref<EntityFolder>& pFolder : folders)
		{
			if (!aOperation(*pFolder))
				break;
		}
	}

	Ref<EntityFolder> EntityFoldersSubsystem::GetFolder(const Scene& aScene, const String& aPath) const noexcept
	{
		if (!IsInitializedForScene(aScene))
			return nullptr;

		const std::vector<Ref<EntityFolder>>& folders = GetFolderContainer(aScene).Folders;
		auto it = std::ranges::find_if(folders, [&aPath](const Ref<EntityFolder>& pFolder) { return pFolder->GetPath() == aPath; });

		return it != folders.end() ? *it : nullptr;
	}

	Ref<EntityFolder> EntityFoldersSubsystem::GetFolder(const Scene& aScene, const UUID& aFolderUUID) const noexcept
	{
		if (!IsInitializedForScene(aScene))
			return nullptr;

		const std::vector<Ref<EntityFolder>>& folders = GetFolderContainer(aScene).Folders;
		for (const Ref<EntityFolder>& pFolder : folders)
		{
			if (pFolder->GetUUID() == aFolderUUID)
				return pFolder;
		}

		return nullptr;
	}

	void EntityFoldersSubsystem::DeleteFolderContainer(const FolderRoot& aRoot) noexcept
	{
		m_SceneToFolders.erase(aRoot.UUID);
	}

	EntityFoldersSubsystem::FolderContainer& EntityFoldersSubsystem::GetFolderContainer(const Scene& aScene) noexcept
	{
		return m_SceneToFolders.at(aScene.GetUUID());
	}

	const EntityFoldersSubsystem::FolderContainer& EntityFoldersSubsystem::GetFolderContainer(const Scene& aScene) const noexcept
	{
		return m_SceneToFolders.at(aScene.GetUUID());
	}

	EntityFoldersSubsystem::FolderContainer& EntityFoldersSubsystem::GetFolderContainer(const FolderRoot& aRoot) noexcept
	{
		return m_SceneToFolders.at(aRoot.UUID);
	}

	const EntityFoldersSubsystem::FolderContainer& EntityFoldersSubsystem::GetFolderContainer(const FolderRoot& aRoot) const noexcept
	{
		return m_SceneToFolders.at(aRoot.UUID);
	}

	void EntityFoldersSubsystem::RemoveEntityFromCurrentFolder(Scene& aScene, entity aEntity) noexcept
	{
		if (AnyFolderContainsEntity(aScene, aEntity))
		{
			const Folder folder = aScene.GetEntityManager().Get<FolderComponent>(aEntity).Folder;
			aScene.GetEntityManager().Remove<FolderComponent>(aEntity);
			OnEntityRemovedFromFolder(aEntity, folder);
		}
	}

	bool EntityFoldersSubsystem::RenameFolder(Scene& aScene, const String& aOldPath, const String& aNewPath) noexcept
	{
		if (aOldPath == aNewPath)
			return true;

		if (!ContainsFolder(aScene, aOldPath) || ContainsFolder(aScene, aNewPath))
			return false;

		// Prevent moving into own descendant: e.g. "A/B" -> "A/B/C"
		if (aNewPath.size() > aOldPath.size() && aNewPath.compare(0, aOldPath.size(), aOldPath) == 0 && aNewPath[aOldPath.size()] == '/')
			return false;

		Ref<EntityFolder> pRoot = GetFolder(aScene, aOldPath);
		if (!pRoot)
			return false;

		// Compute new parent + new label from newPath
		auto slash = aNewPath.rfind('/');
		const String newParentPath = (slash == String::npos) ? String{} : aNewPath.substr(0, slash);
		const String newLabel = (slash == String::npos) ? aNewPath : aNewPath.substr(slash + 1);

		// Ensure the target parent exists (or null for root-level)
		EntityFolder* newParent = nullptr;
		if (!newParentPath.empty())
			newParent = CreateFolder(aScene, newParentPath); // builds chain as needed

		EntityFolder* pOldParent = pRoot->GetParent();
		pRoot->SetParent(newParent);
		pRoot->SetLabel(newLabel);

		OnEntityFolderMoved(pRoot, pOldParent, aOldPath, aNewPath);

		return true;
	}

	bool EntityFoldersSubsystem::ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept
	{
		return dynamic_cast<Editor*>(aSystemManager) != nullptr;
	}

	EntityFolder::EntityFolder(const String& aLabel, bool aExpanded) noexcept
		:m_Label{ aLabel }
		, m_UUID{ CreateUUID() }
		, m_IsExpanded{ aExpanded }
	{
	}

	void EntityFolder::Fixup() noexcept
	{
		while (m_pParent && m_pParent->IsMarkedAsDeleted())
			m_pParent = m_pParent->GetParent();
	}

	const String& EntityFolder::GetLabel() const noexcept
	{
		return m_Label;
	}

	String EntityFolder::GetPath() const noexcept
	{
		String toReturn = "";

		std::vector<const EntityFolder*> hierarchy;
		hierarchy.push_back(this);

		EntityFolder* pCurrent = GetParent();

		while (pCurrent)
		{
			hierarchy.push_back(pCurrent);
			pCurrent = pCurrent->GetParent();
		}

		for (int i = static_cast<int>(hierarchy.size()) - 1; i >= 0; --i)
		{
			String label = hierarchy[i]->GetLabel();
			if (i != 0)
				label.append("/");

			toReturn.append(label);
		}

		return toReturn;
	}

	const UUID& EntityFolder::GetUUID() const noexcept
	{
		return m_UUID;
	}

	void EntityFolder::MarkAsDeleted() noexcept
	{
		m_IsDeleted = true;
	}

	void EntityFolder::SetExpandedState(bool aExpandedState) noexcept
	{
		m_IsExpanded = aExpandedState;
	}

	void EntityFolder::SetLabel(const String& aLabel) noexcept
	{
		m_Label = aLabel;
	}

	void EntityFolder::SetParent(EntityFolder* apParentFolder) noexcept
	{
		m_pParent = apParentFolder;
	}

	EntityFolder* EntityFolder::GetParent() const noexcept
	{
		return m_pParent;
	}

	bool EntityFolder::IsExpanded() const noexcept
	{
		return m_IsExpanded;
	}

	bool EntityFolder::IsMarkedAsDeleted() const noexcept
	{
		return m_IsDeleted;
	}

}