#include "EntityFolders.h"
#include "Editor.h"

namespace Relentless
{
	// Collect direct children by parent pointer
	static void CollectDirectChildren(Relentless::EntityFoldersManager& M, Relentless::Scene& scene, Relentless::EntityFolder* parent, std::vector<Relentless::Ref<Relentless::EntityFolder>>& out)
	{
		out.clear();
		auto& vec = M.GetFolderContainer(scene).Folders;

		for (auto& f : vec)
		{
			if (f->GetParent() == parent)
				out.push_back(f);
		}
	}

	// Find child of `parent` with given label (linear scan; fine for editor sizes)
	static Relentless::EntityFolder* FindChildByLabel(Relentless::EntityFoldersManager& M, Relentless::Scene& scene, Relentless::EntityFolder* parent, const String& label)
	{
		auto& vec = M.GetFolderContainer(scene).Folders;
		for (auto& f : vec)
		{
			if (f->GetParent() == parent && f->GetLabel() == label)
				return f;
		}

		return nullptr;
	}

	void EntityFoldersManager::MergeNodeInto(Scene& scene, Ref<EntityFolder> from, Ref<EntityFolder> into) noexcept
	{
		if (from == into) 
			return;

		// Worklist of pairs (srcParent, dstParent)
		std::vector<std::pair<EntityFolder*, EntityFolder*>> stack;
		stack.emplace_back(from, into);

		std::vector<Ref<EntityFolder>> srcChildren;
		std::vector<Ref<EntityFolder>> toDelete; // losing nodes we’ll remove

		while (!stack.empty())
		{
			auto [srcParent, dstParent] = stack.back();
			stack.pop_back();

			// Enumerate direct children of srcParent
			CollectDirectChildren(*this, scene, srcParent, srcChildren);

			for (auto& srcChild : srcChildren)
			{
				EntityFolder* dstChild = FindChildByLabel(*this, scene, dstParent, srcChild->GetLabel());
				if (dstChild)
				{
					// Same label exists under destination → recurse pair
					stack.emplace_back(srcChild, dstChild);
					toDelete.push_back(srcChild); // this node is a duplicate after merge
				}
				else
				{
					// Unique branch → just reparent under dstParent
					EntityFolder* oldParent = srcChild->GetParent();
					srcChild->SetParent(dstParent);
					srcChild->Fixup();

					// Optional: notify (paths unchanged, so keep this lightweight)
					// OnEntityFolderMoved(srcChild, oldParent, srcChild->GetPath(), srcChild->GetPath());
				}
			}
		}

		// Delete all losing nodes (post-order enough for our flat storage)
		auto& vec = GetFolderContainer(scene).Folders;
		for (auto& lose : toDelete)
		{
			// Remove 'lose' from storage
			auto it = std::find(vec.begin(), vec.end(), lose);
			if (it != vec.end())
			{
				auto last = vec.end() - 1;
				if (it != last) std::iter_swap(it, last);
				vec.pop_back();
			}
		}

		// Finally delete the root 'from' itself
		auto& vec2 = GetFolderContainer(scene).Folders;
		auto it = std::find(vec2.begin(), vec2.end(), from);
		if (it != vec2.end())
		{
			auto last = vec2.end() - 1;
			if (it != last) std::iter_swap(it, last);
			vec2.pop_back();
		}
	}

	void EntityFoldersManager::DeduplicateByPath(Scene& scene) noexcept
	{
		// 1) Snapshot current folders
		auto& vec = GetFolderContainer(scene).Folders;
		std::vector<Ref<EntityFolder>> snapshot = vec; // shallow copy of refs

		// 2) Build path -> canonical map
		std::unordered_map<String, Ref<EntityFolder>> canonical;
		canonical.reserve(snapshot.size() * 2);

		// To ensure determinism, pick earliest (in storage order) as canonical
		for (auto& f : snapshot)
		{
			if (f->IsMarkedAsDeleted()) 
				continue;

			const String path = f->GetPath();
			if (!canonical.contains(path))
				canonical.emplace(path, f);
		}

		// 3) For each folder that maps to an existing canonical with same path but different node → merge
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

			// Merge f into keep (paths identical)
			MergeNodeInto(scene, f, keep);
		}

		// Optional: you can assert no duplicates remain
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

	EntityFoldersManager::EntityFoldersManager(Editor* pAEditor) noexcept
		:m_pEditor{ pAEditor }
	{
	}

	EntityFolder* EntityFoldersManager::CreateFolder(Scene& aScene, const String& aPath) noexcept
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

	EntityFolder* EntityFoldersManager::CreateFolder(Scene& aScene, const Folder& aFolder) noexcept
	{
		return CreateFolder(aScene, aFolder.GetPath());
	}

	EntityFolder* EntityFoldersManager::CreateFolderContainingSelection(Scene& aScene, const String& aPath) noexcept
	{
		//TODO!
		return nullptr;
	}

	bool EntityFoldersManager::ContainsFolder(const Scene& aScene, const Folder& aFolder) const noexcept
	{
		return ContainsFolder(aScene, aFolder.GetPath());
	}

	bool EntityFoldersManager::ContainsFolder(const Scene& aScene, const String& aPath) const noexcept
	{
		if (!IsInitializedForScene(aScene))
			return false;

		const std::vector<Ref<EntityFolder>>& folders = GetFolderContainer(aScene).Folders;
		return std::ranges::any_of(folders, [&](auto const& f) { return f->GetPath() == aPath && !f->IsMarkedAsDeleted(); });
	}

	void EntityFoldersManager::DeleteFolder(Scene& aScene, const Folder& aFolder) noexcept
	{
		DeleteFolder(aScene, aFolder.GetPath());
	}

	void EntityFoldersManager::DeleteFolder(Scene& aScene, const String& aPath) noexcept
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

	void EntityFoldersManager::ForEachEntityInFolders(Scene& aScene, const std::unordered_set<String>& somePaths, Callback<bool(entity)> aOperation) noexcept
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

	String EntityFoldersManager::GetDefaultFolderName(Scene& aScene, const String& aParentPath) const noexcept
	{
		return GetFolderName(aScene, aParentPath, "New Folder");
	}

	Folder EntityFoldersManager::GetDefaultFolderName(Scene& aScene, const Folder& aParentFolder) const noexcept
	{
		return Folder(aParentFolder.GetRoot(), GetDefaultFolderName(aScene, aParentFolder.GetPath()));
	}

	String EntityFoldersManager::GetFolderName(const Scene& aScene, const String& aParentPath, const String& aFolderName) const noexcept
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

	bool EntityFoldersManager::IsFolderExpanded(Scene& aScene, const String& aPath) const noexcept
	{
		return ContainsFolder(aScene, aPath) && GetFolder(aScene, aPath)->IsExpanded();
	}

	bool EntityFoldersManager::IsFolderExpanded(Scene& aScene, const Folder& aFolder) const noexcept
	{
		return IsFolderExpanded(aScene, aFolder.GetPath());
	}

	bool EntityFoldersManager::IsInitializedForScene(const Scene& aScene) const noexcept
	{
		return m_SceneToFolders.contains(aScene.GetUUID());
	}

	bool EntityFoldersManager::IsInitializedForRoot(const FolderRoot& aRoot) const noexcept
	{
		return m_SceneToFolders.contains(aRoot.UUID);
	}

	void EntityFoldersManager::OnFolderRootObjectRemoved(const FolderRoot& aRootObject) noexcept
	{
		if (IsInitializedForRoot(aRootObject))
			DeleteFolderContainer(aRootObject);
	}

	void EntityFoldersManager::ForEachFolder(const Callback<bool(const EntityFolder&)>& aOperation) noexcept
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

	void EntityFoldersManager::ForEachFolderWithRootObject(const FolderRoot& aRoot, const Callback<bool(const EntityFolder&)>& aOperation) noexcept
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

	Ref<EntityFolder> EntityFoldersManager::GetFolder(const Scene& aScene, const String& aPath) const noexcept
	{
		if (!IsInitializedForScene(aScene))
			return nullptr;

		const std::vector<Ref<EntityFolder>>& folders = GetFolderContainer(aScene).Folders;
		auto it = std::ranges::find_if(folders, [&aPath](const Ref<EntityFolder>& pFolder) { return pFolder->GetPath() == aPath; });

		return it != folders.end() ? *it : nullptr;
	}

	void EntityFoldersManager::DeleteFolderContainer(const FolderRoot& aRoot) noexcept
	{
		m_SceneToFolders.erase(aRoot.UUID);
	}

	EntityFoldersManager::FolderContainer& EntityFoldersManager::GetFolderContainer(const Scene& aScene) noexcept
	{
		return m_SceneToFolders.at(aScene.GetUUID());
	}

	const EntityFoldersManager::FolderContainer& EntityFoldersManager::GetFolderContainer(const Scene& aScene) const noexcept
	{
		return m_SceneToFolders.at(aScene.GetUUID());
	}

	EntityFoldersManager::FolderContainer& EntityFoldersManager::GetFolderContainer(const FolderRoot& aRoot) noexcept
	{
		return m_SceneToFolders.at(aRoot.UUID);
	}

	const EntityFoldersManager::FolderContainer& EntityFoldersManager::GetFolderContainer(const FolderRoot& aRoot) const noexcept
	{
		return m_SceneToFolders.at(aRoot.UUID);
	}

	bool EntityFoldersManager::RenameFolder(Scene& aScene, const String& aOldPath, const String& aNewPath) noexcept
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

	EntityFolder::EntityFolder(const String& aLabel, bool aExpanded) noexcept
		:m_Label{ aLabel }
		,m_UUID{ CreateUUID()}
		,m_IsExpanded{ aExpanded }
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