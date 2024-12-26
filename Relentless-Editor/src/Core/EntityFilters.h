#pragma once
#include <Relentless.h>

namespace Relentless
{
	class EntityFilter
	{
	public:
		explicit EntityFilter(const std::string& path, EntityFilter* pParent, bool expanded = true) noexcept;
		[[nodiscard]] std::shared_ptr<EntityFilter> AddChild(const std::string& childName) noexcept;
		[[nodiscard]] bool SetChild(const std::shared_ptr<EntityFilter>& pExistingFilter) noexcept;
		void RemoveChild(const std::string& childName) noexcept;
		[[nodiscard]] std::shared_ptr<EntityFilter> FindChild(const std::string& childName) const noexcept;
		[[nodiscard]] const std::string& GetName() const noexcept;
		[[nodiscard]] std::string GetPath() const noexcept;
		[[nodiscard]] bool Contains(entity e) const noexcept;
		[[nodiscard]] bool Contains(const std::shared_ptr<EntityFilter>& pFilter) const noexcept;
		[[nodiscard]] const std::unordered_set<entity>& GetEntities() const noexcept;
		[[nodiscard]] const std::unordered_map<std::string, std::shared_ptr<EntityFilter>>& GetChildren() const noexcept;

		void AddEntity(entity e) noexcept;
		void RemoveEntity(entity e) noexcept;

		void SetExpandedState(bool expandedState) noexcept;

		void SetParent(EntityFilter* pParentFilter) noexcept;
		[[nodiscard]] EntityFilter* GetParent() const noexcept;
		[[nodiscard]] bool IsExpanded() const noexcept;
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
		void CreateFilter(const std::string& path) noexcept;
		void DestroyFilter(const std::string& path) noexcept;

		[[nodiscard]] std::shared_ptr<EntityFilter> GetFilter(const std::string& path) const noexcept;
		[[nodiscard]] bool FilterExists(const std::string& path) const noexcept;
		[[nodiscard]] bool IsEntityInAnyFilter(entity e) const noexcept;
		void SetEntityToFilter(entity e, const std::string& path) noexcept;
		void SetFilterToFilter(const std::string& filterPathChild, const std::string& filterPathParent) noexcept;
		void RemoveEntityFromCurrentFilter(entity e) noexcept;

		Broadcaster<void(entity e, const std::string& path)> OnEntityRemovedFromFilter;
		Broadcaster<void(entity e, const std::string& path)> OnEntitySetToFilter;
		Broadcaster<void(const std::string& path)> OnFilterCreated;
		Broadcaster<void(const std::string& path)> OnFilterDestroyed;

		Broadcaster<void(const std::string& originalChildPath, const std::string& newChildPath, const std::string& parentPath)> OnFilterReattached;
	private:
		void DestroyHierarchy(const std::string& path) noexcept;
		[[nodiscard]] std::shared_ptr<EntityFilter> GetOrCreateRootFilter(const std::string& path) noexcept;
	private:
		std::unordered_map<std::string, std::shared_ptr<EntityFilter>> m_RootFilters;
		std::unordered_map<entity, std::weak_ptr<EntityFilter>> m_EntityToFilterMap;
	};
}