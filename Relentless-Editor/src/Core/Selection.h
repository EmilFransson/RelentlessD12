#pragma once
#include <Relentless.h>

namespace Relentless
{
	enum class ESelectionState : uint8 { Selected = 0, Deselected };

	class Selection
	{
	public:
		Selection() noexcept = default;
		~Selection() noexcept = default;

		void SelectEntity(entity entityToSelect) noexcept;
		void SelectEntities(std::span<entity> entitiesToSelect) noexcept;

		void DeselectEntity(entity entityToDeselect) noexcept;
		void DeselectEntities(std::span<entity> entitiesToDeselect) noexcept;
		void DeselectAllEntities() noexcept;

		NO_DISCARD const std::vector<entity>& GetSelectedEntities() const noexcept;
		NO_DISCARD bool IsEntitySelected(entity entityToQuery) const noexcept;
		NO_DISCARD uint32 GetSelectedEntityCount() const noexcept;
		NO_DISCARD entity GetFirstSelected() const noexcept;

		Broadcaster<void()> OnPreDeselectAll;
		Broadcaster<void(entity e, ESelectionState)> OnSelectionChanged;
	private:
		/*
			Element 0 - selected first. 
			Last element - selected last.
		*/
		std::vector<entity> m_SelectedEntities;
	};
}