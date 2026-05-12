#pragma once
#include "Subsystem/ISubsystem.h"

namespace Relentless
{
	class RLS_API SelectionRenderSubsystem : public ISubsystem
	{
	public:
		void Deselect(std::vector<uint32> someDeselectedEntityIDs) noexcept;

		NO_DISCARD uint32 GetNumSelectedEntities() const noexcept;
		NO_DISCARD const std::unordered_set<uint32>& GetSelectedEntities() const noexcept;

		NO_DISCARD bool IsSelected(uint32 aID) const noexcept;

		void Select(std::vector<uint32> someSelectedEntityIDs) noexcept;
		static bool ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept;
	private:
		std::unordered_set<uint32> m_SelectedEntities;
	};
}
