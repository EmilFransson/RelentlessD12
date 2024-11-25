#pragma once
#include <Relentless.h>

namespace Relentless
{
	enum class ESelectionState : uint8_t { Selected = 0, Deselected };

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

		[[nodiscard]] const std::vector<entity>& GetSelectedEntities() const noexcept;
		[[nodiscard]] bool IsEntitySelected(entity entityToQuery) const noexcept;
		[[nodiscard]] size_t GetSelectedEntityCount() const noexcept;
		[[nodiscard]] entity GetFirstSelected() const noexcept;

		Broadcaster<void(entity e, ESelectionState)> OnSelectionChanged;
	private:
		/*
			Element 0 - selected first. 
			Last element - selected last.
		*/
		std::vector<entity> m_SelectedEntities;
	};
}