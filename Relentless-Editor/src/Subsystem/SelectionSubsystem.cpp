#include "SelectionSubsystem.h"
#include "../Core/Editor.h"

namespace Relentless
{
	void SelectionSubsystem::DeselectEntity(entity aEntityToDeselect) noexcept
	{
		RLS_ASSERT(IsEntitySelected(aEntityToDeselect), "Entity is not selected.");
		
		m_SelectedEntities.erase(std::remove(m_SelectedEntities.begin(), m_SelectedEntities.end(), aEntityToDeselect), m_SelectedEntities.end());
		OnSelectionChanged(aEntityToDeselect, ESelectionState::Deselected);
	}

	void SelectionSubsystem::DeselectEntities(std::span<entity> someEntitiesToDeselect) noexcept
	{
		for (auto& entityToDeselect : someEntitiesToDeselect)
			DeselectEntity(entityToDeselect);
	}

	void SelectionSubsystem::DeselectAllEntities() noexcept
	{
		OnPreDeselectAll();

		for (int i = static_cast<int>(m_SelectedEntities.size()) - 1; i >= 0; --i)
			DeselectEntity(m_SelectedEntities[i]);
	}

	const std::vector<entity>& SelectionSubsystem::GetSelectedEntities() const noexcept
	{
		return m_SelectedEntities;
	}

	uint32 SelectionSubsystem::GetSelectedEntityCount() const noexcept
	{
		return static_cast<uint32>(m_SelectedEntities.size());
	}

	entity SelectionSubsystem::GetFirstSelected() const noexcept
	{
		if (m_SelectedEntities.empty())
			return NULL_ENTITY;
		else
			return m_SelectedEntities.front();
	}

	bool SelectionSubsystem::IsEntitySelected(entity aEntityToQuery) const noexcept
	{
		return std::find(m_SelectedEntities.begin(), m_SelectedEntities.end(), aEntityToQuery) != m_SelectedEntities.end();
	}

	bool SelectionSubsystem::OnLoad(ISystemManager* aSystemManager) noexcept
	{
		Editor* pEditor = static_cast<Editor*>(aSystemManager);
		pEditor->OnSceneChange.Connect(this, &SelectionSubsystem::OnSceneChange);

		return true;
	}

	void SelectionSubsystem::OnSceneChange(MAYBE_UNUSED Scene* aScene) noexcept
	{
		DeselectAllEntities();
	}

	void SelectionSubsystem::SelectEntity(entity aEntityToSelect) noexcept
	{
		RLS_ASSERT(!IsEntitySelected(aEntityToSelect), "Entity is already selected.");

		m_SelectedEntities.push_back(aEntityToSelect);
		OnSelectionChanged(aEntityToSelect, ESelectionState::Selected);
	}

	void SelectionSubsystem::SelectEntities(std::span<entity> someEntitiesToSelect) noexcept
	{
		for (auto& entityToSelect : someEntitiesToSelect)
			SelectEntity(entityToSelect);
	}
	
	bool SelectionSubsystem::ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept
	{
		return dynamic_cast<Editor*>(aSystemManager) != nullptr;
	}
}