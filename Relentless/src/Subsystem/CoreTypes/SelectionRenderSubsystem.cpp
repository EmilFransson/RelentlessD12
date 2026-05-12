#include "SelectionRenderSubsystem.h"

#include "Graphics/Scene/RenderScene.h"

namespace Relentless
{
	bool SelectionRenderSubsystem::ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept
	{
		return dynamic_cast<RenderScene*>(aSystemManager) != nullptr;
	}

	void SelectionRenderSubsystem::Deselect(std::vector<uint32> someDeselectedEntityIDs) noexcept
	{
		for (uint32 id : someDeselectedEntityIDs)
		{
			RLS_ASSERT(IsSelected(id), "[SelectionRenderSubsystem::Deselect]: Entity with ID {} no already selected", id);
			m_SelectedEntities.erase(id);
		}
	}

	uint32 SelectionRenderSubsystem::GetNumSelectedEntities() const noexcept
	{
		return static_cast<uint32>(m_SelectedEntities.size());
	}

	const std::unordered_set<uint32>& SelectionRenderSubsystem::GetSelectedEntities() const noexcept
	{
		return m_SelectedEntities;
	}

	bool SelectionRenderSubsystem::IsSelected(uint32 aID) const noexcept
	{
		return m_SelectedEntities.contains(aID);
	}

	void SelectionRenderSubsystem::Select(std::vector<uint32> someSelectedEntityIDs) noexcept
	{
		for (uint32 id : someSelectedEntityIDs)
		{
			RLS_ASSERT(!IsSelected(id), "[SelectionRenderSubsystem::Select]: Entity with ID {} is already selected", id);
			m_SelectedEntities.insert(id);
		}
	}

}