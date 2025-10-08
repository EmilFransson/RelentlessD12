#include "Selection.h"

namespace Relentless
{
	void Selection::SelectEntity(entity entityToSelect) noexcept
	{
		RLS_ASSERT(!IsEntitySelected(entityToSelect), "Entity is already selected.");
		
		m_SelectedEntities.push_back(entityToSelect);
		OnSelectionChanged(entityToSelect, ESelectionState::Selected);
	}

	void Selection::SelectEntities(std::span<entity> entitiesToSelect) noexcept
	{
		for (auto& entityToSelect : entitiesToSelect)
			SelectEntity(entityToSelect);
	}

	void Selection::DeselectEntity(entity entityToDeselect) noexcept
	{
		RLS_ASSERT(IsEntitySelected(entityToDeselect), "Entity is not selected.");
		m_SelectedEntities.erase(std::remove(m_SelectedEntities.begin(), m_SelectedEntities.end(), entityToDeselect), m_SelectedEntities.end());
		OnSelectionChanged(entityToDeselect, ESelectionState::Deselected);
	}

	void Selection::DeselectEntities(std::span<entity> entitiesToDeselect) noexcept
	{
		for (auto& entityToDeselect : entitiesToDeselect)
			DeselectEntity(entityToDeselect);
	}

	void Selection::DeselectAllEntities() noexcept
	{
		OnPreDeselectAll();

		for (int i = static_cast<int>(m_SelectedEntities.size()) - 1; i >= 0; --i)
			DeselectEntity(m_SelectedEntities[i]);
	}

	const std::vector<entity>& Selection::GetSelectedEntities() const noexcept
	{
		return m_SelectedEntities;
	}

	bool Selection::IsEntitySelected(entity entityToQuery) const noexcept
	{
		return std::find(m_SelectedEntities.begin(), m_SelectedEntities.end(), entityToQuery) != m_SelectedEntities.end();
	}

	uint32 Selection::GetSelectedEntityCount() const noexcept
	{
		return static_cast<uint32>(m_SelectedEntities.size());
	}

	entity Selection::GetFirstSelected() const noexcept
	{
		if (m_SelectedEntities.empty())
			return NULL_ENTITY;
		else
			return m_SelectedEntities.front();
	}

}