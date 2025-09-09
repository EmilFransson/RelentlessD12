#pragma once
#include <Relentless.h>

namespace Relentless
{
	class EntityFilter
	{
	public:
		explicit EntityFilter(const std::string& path, EntityFilter* pParent, bool expanded = true) noexcept;
		NO_DISCARD std::shared_ptr<EntityFilter> AddChild(const std::string& childName) noexcept;
		NO_DISCARD bool SetChild(const std::shared_ptr<EntityFilter>& pExistingFilter) noexcept;
		void RemoveChild(const std::string& childName) noexcept;
		NO_DISCARD std::shared_ptr<EntityFilter> FindChild(const std::string& childName) const noexcept;
		NO_DISCARD const std::string& GetName() const noexcept;
		NO_DISCARD std::string GetPath() const noexcept;
		NO_DISCARD bool Contains(entity e) const noexcept;
		NO_DISCARD bool Contains(const std::shared_ptr<EntityFilter>& pFilter) const noexcept;
		NO_DISCARD bool Contains(const EntityFilter* pFilter) const noexcept;
		NO_DISCARD const std::unordered_set<entity>& GetEntities() const noexcept;
		NO_DISCARD const std::unordered_map<std::string, std::shared_ptr<EntityFilter>>& GetChildren() const noexcept;

		void AddEntity(entity e) noexcept;
		void RemoveEntity(entity e) noexcept;

		void SetExpandedState(bool expandedState) noexcept;

		void SetName(const String& name) noexcept;
		void SetParent(EntityFilter* pParentFilter) noexcept;
		NO_DISCARD EntityFilter* GetParent() const noexcept;
		NO_DISCARD bool IsExpanded() const noexcept;
	private:
		std::string m_Name;
		std::unordered_set<entity> m_Entities;
		std::unordered_map<std::string, std::shared_ptr<EntityFilter>> m_Children;

		EntityFilter* m_pParent = nullptr;
		bool m_IsExpanded = true;
	};

	class EntityFiltersManager
	{
	public:
		EntityFilter* CreateFilter(const std::string& path) noexcept;
		void DestroyFilter(const std::string& path) noexcept;

		void ForEachFilterWithDescendantObject(EntityFilter* pFilter, bool includeDescendantObject, const Callback<bool(EntityFilter*)>& operation) noexcept;
		void ForEachFilterWithRootObject(EntityFilter* pFilter, bool includeRootObject, const Callback<bool(EntityFilter*)>& operation) noexcept;
		void ForEachFilterWithParentObject(EntityFilter* pFilter, const Callback<bool(EntityFilter*)>& operation) noexcept;
		void ForEachRootFilters(const Callback<bool(EntityFilter*)>& operation) noexcept;

		NO_DISCARD std::shared_ptr<EntityFilter> GetFilter(const std::string& path) const noexcept;
		NO_DISCARD bool FilterExists(const std::string& path) const noexcept;
		NO_DISCARD EntityFilter* GetFilterContainingEntity(entity e) const noexcept;
		NO_DISCARD bool IsEntityInAnyFilter(entity e) const noexcept;
		NO_DISCARD bool IsRootFilter(const std::string& path) const noexcept;
		void SetEntityToFilter(entity e, const std::string& path) noexcept;
		void SetFilterToFilter(const std::string& filterPathChild, const std::string& filterPathParent) noexcept;
		void RemoveEntityFromCurrentFilter(entity e) noexcept;

		mutable Broadcaster<void(entity e, EntityFilter* pParentFilter, bool filterToBeDestroyed)> OnEntityRemovedFromFilter;
		mutable Broadcaster<void(entity e, EntityFilter* pFilter)> OnEntitySetToFilter;
		mutable Broadcaster<void(EntityFilter* pFilter)> OnFilterCreated;
		mutable Broadcaster<void(EntityFilter* pFilter)> OnFilterDestroyed;
		mutable Broadcaster<void(EntityFilter* pFilter)> OnPreFilterDestroyed;

		mutable Broadcaster<void(EntityFilter* pReattachedFilter, EntityFilter* pPreviousParent, EntityFilter* pNewParent)> OnFilterReattached;
	private:
		void DestroyHierarchy(const std::string& path) noexcept;
		void RemoveEntityFromCurrentFilterInternal(entity e, bool partOfFilterDestroyAction) noexcept;
		NO_DISCARD std::shared_ptr<EntityFilter> GetOrCreateRootFilter(const std::string& path) noexcept;
	private:
		std::unordered_map<std::string, std::shared_ptr<EntityFilter>> m_RootFilters;
		std::unordered_map<entity, std::weak_ptr<EntityFilter>> m_EntityToFilterMap;
	};
}