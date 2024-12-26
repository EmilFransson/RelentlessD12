#include "EntityFilters.h"

namespace Relentless
{
	void EntityFiltersManager::CreateFilter(const std::string& path) noexcept
	{
		if (FilterExists(path))
			return;

		const std::vector<std::string> components = StringUtils::Split(path, '/');
		if (components.empty())
			return;

		std::shared_ptr<EntityFilter> pRootFilter = GetOrCreateRootFilter(components[0]);
			
		std::shared_ptr<EntityFilter> pCurrent = pRootFilter;
		for (size_t i = 1; i < components.size(); ++i)
			pCurrent = pCurrent->AddChild(components[i]);

		OnFilterCreated(path);
	}

	void EntityFiltersManager::DestroyFilter(const std::string& path) noexcept
	{
		DestroyHierarchy(path);

		//At this point it only remains to remove the actual filter, as the hierarchy depending on the filter will have been destroyed
		//at previous call.

		std::shared_ptr<EntityFilter> pToDestroy = GetFilter(path);
		EntityFilter* pParent = pToDestroy->GetParent();
		if (pParent)
			pParent->RemoveChild(pToDestroy->GetName());
		else
			m_RootFilters.erase(pToDestroy->GetName());

		OnFilterDestroyed(path);
	}

	void EntityFiltersManager::DestroyHierarchy(const std::string& path) noexcept
	{
		if (!FilterExists(path))
			return;

		std::shared_ptr<EntityFilter> pFilter = GetFilter(path);

		const std::unordered_set<entity>& entities = pFilter->GetEntities();

		for (auto entity : entities)
		{
			m_EntityToFilterMap.erase(entity);
			OnEntityRemovedFromFilter(entity, pFilter->GetPath());
		}

		const std::unordered_map<std::string, std::shared_ptr<EntityFilter>>& children = pFilter->GetChildren();

		std::vector<std::pair<std::string, std::string>> keys;
		keys.reserve(children.size());

		for (auto& [childName, filter] : children)
		{
			const std::string fullPath = filter->GetPath();
			DestroyHierarchy(fullPath);
			keys.push_back({ childName, fullPath });
		}

		for (auto& [name, fullPath] : keys)
		{
			pFilter->RemoveChild(name);
			OnFilterDestroyed(fullPath);
		}
	}

	std::shared_ptr<EntityFilter> EntityFiltersManager::GetFilter(const std::string& path) const noexcept
	{
		const std::vector<std::string> components = StringUtils::Split(path, '/');
		if (components.empty())
			return nullptr;

		auto it = m_RootFilters.find(components[0]);
		if (it == m_RootFilters.end())
			return nullptr;

		std::shared_ptr<EntityFilter> pCurrent = it->second;
		for (size_t i = 1; i < components.size(); ++i)
		{
			pCurrent = pCurrent->FindChild(components[i]);
			if (!pCurrent)
				return nullptr;
		}

		return pCurrent;
	}

	bool EntityFiltersManager::FilterExists(const std::string& path) const noexcept
	{
		const std::vector<std::string> components = StringUtils::Split(path, '/');
		if (components.empty()) 
			return false;

		auto it = m_RootFilters.find(components[0]);
		if (it == m_RootFilters.end())
			return false;

		std::shared_ptr<EntityFilter> pCurrent = it->second;
		for (size_t i = 1; i < components.size(); ++i)
		{
			pCurrent = pCurrent->FindChild(components[i]);
			if (!pCurrent) 
				return false;
		}
		return true;
	}

	bool EntityFiltersManager::IsEntityInAnyFilter(entity e) const noexcept
	{
		return m_EntityToFilterMap.contains(e);
	}

	void EntityFiltersManager::SetEntityToFilter(entity e, const std::string& path) noexcept
	{
		//If already exist in another filter, remove it.
		if (IsEntityInAnyFilter(e))
		{
			std::shared_ptr<EntityFilter> pEntityFilter = m_EntityToFilterMap[e].lock();

			//Entity is already in filter
			if (pEntityFilter->GetPath() == path)
				return;

			RemoveEntityFromCurrentFilter(e);
		}

		CreateFilter(path);

		std::shared_ptr<EntityFilter> pFilter = GetFilter(path);
		RLS_ASSERT(pFilter, "Filter is invalid.");
		
		pFilter->AddEntity(e);
		m_EntityToFilterMap[e] = pFilter;

		OnEntitySetToFilter(e, path);
	}

	void EntityFiltersManager::SetFilterToFilter(const std::string& filterPathChild, const std::string& filterPathParent) noexcept
	{
		std::shared_ptr<EntityFilter> pChildFilter = GetFilter(filterPathChild);
		if (!pChildFilter)
			return;

		std::shared_ptr<EntityFilter> pNewParentFilter = GetFilter(filterPathParent);
		if (!pNewParentFilter)
			return;

		EntityFilter* pCurrentParentFilter = pChildFilter->GetParent();
		if (pCurrentParentFilter == pNewParentFilter.get())
			return;

		if (pCurrentParentFilter)
			pCurrentParentFilter->RemoveChild(pChildFilter->GetName());
		
		if (pNewParentFilter->SetChild(pChildFilter))
			OnFilterReattached(filterPathChild, pChildFilter->GetPath(), filterPathParent);
	}

	void EntityFiltersManager::RemoveEntityFromCurrentFilter(entity e) noexcept
	{
		std::shared_ptr<EntityFilter> pEntityFilter = m_EntityToFilterMap[e].lock();

		pEntityFilter->RemoveEntity(e);

		m_EntityToFilterMap.erase(e);
		OnEntityRemovedFromFilter(e, pEntityFilter->GetPath());
	}

	std::shared_ptr<EntityFilter> EntityFiltersManager::GetOrCreateRootFilter(const std::string& path) noexcept
	{
		auto it = m_RootFilters.find(path);
		if (it == m_RootFilters.end())
			m_RootFilters[path] = std::make_shared<EntityFilter>(path, nullptr);

		return m_RootFilters[path];
	}

	EntityFilter::EntityFilter(const std::string& name, EntityFilter* pParent, bool expanded) noexcept
		: 
		m_Name{ name }, 
		m_pParent{ pParent }, 
		m_IsExpanded{ expanded }
	{
	}

	std::shared_ptr<EntityFilter> EntityFilter::AddChild(const std::string& childName) noexcept
	{
		if (!FindChild(childName))
			m_Children[childName] = std::make_shared<EntityFilter>(childName, this);

		return m_Children[childName];
	}

	bool EntityFilter::SetChild(const std::shared_ptr<EntityFilter>& pExistingFilter) noexcept
	{
		const std::string& name = pExistingFilter->GetName();

		if (!FindChild(name))
		{
			m_Children[name] = pExistingFilter;
			pExistingFilter->SetParent(this);
			return true;
		}

		return false;
	}

	void EntityFilter::RemoveChild(const std::string& childName) noexcept
	{
		m_Children.erase(childName);
	}

	std::shared_ptr<EntityFilter> EntityFilter::FindChild(const std::string& childName) const noexcept
	{
		auto It = m_Children.find(childName);
		return (It != m_Children.end()) ? It->second : nullptr;
	}

	const std::string& EntityFilter::GetName() const noexcept
	{
		return m_Name;
	}

	std::string EntityFilter::GetPath() const noexcept
	{
		std::string fullFilterPath = "";

		std::vector<const EntityFilter*> hierarchy;
		hierarchy.push_back(this);

		EntityFilter* pCurrent = GetParent();

		while (pCurrent)
		{
			hierarchy.push_back(pCurrent);
			pCurrent = pCurrent->GetParent();
		}

		for (int i = hierarchy.size() - 1; i >= 0; --i)
		{
			std::string name = hierarchy[i]->GetName();
			if (i != 0)
				name.append("/");

			fullFilterPath.append(name);
		}

		return fullFilterPath;
	}

	bool EntityFilter::Contains(entity e) const noexcept
	{
		return m_Entities.contains(e);
	}

	bool EntityFilter::Contains(const std::shared_ptr<EntityFilter>& pFilter) const noexcept
	{
		return m_Children.contains(pFilter->GetPath());
	}

	const std::unordered_set<entity>& EntityFilter::GetEntities() const noexcept
	{
		return m_Entities;
	}

	const std::unordered_map<std::string, std::shared_ptr<EntityFilter>>& EntityFilter::GetChildren() const noexcept
	{
		return m_Children;
	}

	void EntityFilter::AddEntity(entity e) noexcept
	{
		RLS_ASSERT(!Contains(e), "[EntityFilter]: Entity already exists in filter.");
		m_Entities.insert(e);
	}

	void EntityFilter::RemoveEntity(entity e) noexcept
	{
		RLS_ASSERT(Contains(e), "[EntityFilter]: Entity does not exist in filter.");
		m_Entities.erase(e);
	}

	void EntityFilter::SetExpandedState(bool expandedState) noexcept
	{
		m_IsExpanded = expandedState;
	}

	void EntityFilter::SetParent(EntityFilter* pParentFilter) noexcept
	{
		m_pParent = pParentFilter;
	}

	EntityFilter* EntityFilter::GetParent() const noexcept
	{
		return m_pParent;
	}

	bool EntityFilter::IsExpanded() const noexcept
	{
		return m_IsExpanded;
	}

}